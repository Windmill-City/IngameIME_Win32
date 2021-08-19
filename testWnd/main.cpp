#include "libtf.h"
#include <iostream>
#include <string>

#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <msctf.h>

using namespace std;
libtf_pInputContext ctx;

void error_callback(int error, const char* description)
{
    cout << "Error:" << error << description << endl;
}

void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, GLFW_TRUE); }

#pragma region Toggle IMState
    static bool toggleDown = false;
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !toggleDown) {
        toggleDown = true;
        bool imState;
        libtf_get_im_state(ctx, &imState);
        if (FAILED(libtf_set_im_state(ctx, !imState))) {
            cout << "Failed to set im state!" << endl;
            return;
        }
        cout << "IM State:" << !imState << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE) { toggleDown = false; }
#pragma endregion
#pragma region FullScreen
    static bool fullScreenDown = false;
    static bool isFullScreen   = false;
    if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS && !fullScreenDown) {
        fullScreenDown = true;
        isFullScreen   = !isFullScreen;
        if (FAILED(libtf_set_full_screen(ctx, isFullScreen))) {
            cout << "Failed to set full screen!" << endl;
            return;
        }
        cout << "Full Screen:" << isFullScreen << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_RELEASE) { fullScreenDown = false; }
#pragma endregion
}

void window_focus_callback(GLFWwindow* window, int focused)
{
    libtf_on_focus_msg(ctx, glfwGetWin32Window(window), focused ? WM_SETFOCUS : WM_KILLFOCUS);
}

void memory_dump(void* ptr, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if (i % 8 == 0 && i != 0) printf(" ");
        if (i % 16 == 0 && i != 0) printf("\n");
        printf("%02x ", *((uint8_t*)ptr + i));
    }
    printf("\n");
}

#pragma region libtf callbacks

void onBoundingBox(libtf_BoundingBox_t* box, void* userData)
{
    box->left   = 0;
    box->right  = 0;
    box->top    = 0;
    box->bottom = 20;// Font height
    printf("Fetch Bounding Box:%d, %d,%d, %d\n", box->left, box->right, box->top, box->bottom);
}
void onConversionMode(libtf_ConversionMode mode, void* userData)
{
    printf("Conversion Mode:%d\n", mode);
}
void onSentenceMode(libtf_SentenceMode mode, void* userData)
{
    printf("Sentence Mode:%d\n", mode);
}
void onComposition(libtf_Composition_t composition, void* userData)
{
    setlocale(LC_ALL, "");
    switch (composition.state) {
        case libtf_CompositionBegin: printf("Composition Begin\n"); break;
        case libtf_CompositionUpdate:
            printf("Composition Update\n");
            printf("PreEdit:%ls\n", composition.preEdit);
            memory_dump(composition.preEdit, (SysStringLen(composition.preEdit) + 1) * sizeof(wchar_t));
            printf("Selection:%d, %d\n", composition.selection[0], composition.selection[1]);
            break;
        case libtf_CompositionEnd: printf("Composition End\n"); break;
        default: break;
    }
}
void onCommit(libtf_Commit commit, void* userData)
{
    setlocale(LC_ALL, "");
    printf("Commit: %ls\n", commit);
    /**
     *       |<----Poiter start from here, not from the len
     * |len |---Str---|NULL|
     * |----|---Str---|0000|
     *
     * len is uint32_t
     * Str is in Unicode
     */
    memory_dump(commit - sizeof(int32_t) / sizeof(wchar_t),
                (SysStringLen(commit) + 1) * sizeof(wchar_t) + sizeof(int32_t));
}
void onCandidateList(libtf_CandidateList_t list, void* userData)
{
    setlocale(LC_ALL, "");
    switch (list.state) {
        case libtf_CandidateListBegin: printf("Candidate List Begin\n"); break;
        case libtf_CandidateListUpdate:
            printf("Composition Update\n");
            printf(
                "Total:%d; Page:%d, %d; CurSel:%d\n", list.totalCount, list.pageStart, list.pageEnd, list.curSelection);

            for (size_t i = list.pageStart; i <= list.pageEnd; i++) {
                printf("[%zd]%ls\n", i - list.pageStart + 1, list.candidates[i]);
            }
            break;
        case libtf_CandidateListEnd: printf("Candidate List End\n"); break;
        default: break;
    }
}
void onInputProcessor(libtf_InputProcessorProfile_t profile, void* userData)
{
    printf("%s\n", (char*)userData);
    printf("[%s] [%x-%x-%x-%llx] State:%d\n",
           profile.profileType & TF_PROFILETYPE_INPUTPROCESSOR ? "TIP" : "HKL",
           profile.clsid.Data1,
           profile.clsid.Data2,
           profile.clsid.Data3,
           *(unsigned long long*)profile.clsid.Data4,
           profile.activated);
}
#pragma endregion

void testGetCurInputProcessor()
{
    libtf_InputProcessorProfile_t curActiveInputProcessor;
    libtf_get_active_input_processor(&curActiveInputProcessor);
    printf("Cur Active:[%s][%x-%x-%x-%llx]Lang Id:%d, Active:%d\n",
           curActiveInputProcessor.profileType & TF_PROFILETYPE_INPUTPROCESSOR ? "TIP" : "HKL",
           curActiveInputProcessor.clsid.Data1,
           curActiveInputProcessor.clsid.Data2,
           curActiveInputProcessor.clsid.Data3,
           *(unsigned long long*)curActiveInputProcessor.clsid.Data4,
           curActiveInputProcessor.langId,
           curActiveInputProcessor.activated);
}

void testSetInputProcessor()
{
    uint32_t fetched;
    libtf_get_input_processors(NULL, 0, &fetched);
    libtf_pInputProcessorProfile profiles = new libtf_InputProcessorProfile_t[fetched];
    libtf_get_input_processors(profiles, fetched, &fetched);

    printf("Found %d InputProcessors\n", (int)fetched);
    for (size_t i = 0; i < fetched; i++) {
        printf("[%d][%s][%x-%x-%x-%llx]Lang Id:%d, Active:%d\n",
               (int)i,
               profiles[i].profileType & TF_PROFILETYPE_INPUTPROCESSOR ? "TIP" : "HKL",
               profiles[i].clsid.Data1,
               profiles[i].clsid.Data2,
               profiles[i].clsid.Data3,
               *(unsigned long long*)profiles[i].clsid.Data4,
               profiles[i].langId,
               profiles[i].activated);

        setlocale(LC_ALL, "");
        BSTR locale;
        libtf_get_input_processor_locale(profiles[i], &locale);
        BSTR name;
        libtf_get_locale_name(locale, &name);
        BSTR desc;
        libtf_get_input_processor_desc(profiles[i], &desc);
        printf("InputProcessor:[%ls][%ls][%ls]\n", locale, name, desc);
        SysFreeString(locale);
        SysFreeString(name);
        SysFreeString(desc);

        // If the input processor is not active, try to active it
        if (!profiles[i].activated) {
            libtf_set_active_input_processor(profiles[i]);
        }
    }

    delete[] profiles;
}

int main()
{
    glfwSetErrorCallback(error_callback);
    if (glfwInit()) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(800, 600, "libtf", NULL, NULL);
        if (!window) {
            cout << "Failed to create window!" << endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        // libtf support both ApartMentThreaded and MultiThreaded
        // CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        CoInitializeEx(NULL, COINIT_MULTITHREADED);

        testSetInputProcessor();

#pragma region libtf init
        HRESULT hr;
        if (FAILED(hr = libtf_create_ctx(&ctx))) {
            cout << "Failed to create input context!" << hr << endl;
            glfwTerminate();
            return -1;
        }
        // Once the window get focus, you need to call libtf_set_focus_wnd
        glfwSetWindowFocusCallback(window, window_focus_callback);

        libtf_set_conversion_mode_callback(ctx, onConversionMode, NULL);
        libtf_set_sentence_mode_callback(ctx, onSentenceMode, NULL);
        libtf_set_composition_callback(ctx, onComposition, NULL);
        libtf_set_commit_callback(ctx, onCommit, NULL);
        libtf_set_candidate_list_callback(ctx, onCandidateList, NULL);
        libtf_set_bounding_box_callback(ctx, onBoundingBox, NULL);
        libtf_set_input_processor_callback(ctx, onInputProcessor, "UserData Test");
#pragma endregion

        while (!glfwWindowShouldClose(window)) {
            process_input(window);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

#pragma region libtf dispose
        if (FAILED(hr = libtf_dispose_ctx(ctx))) { cout << "Failed to dispose input context!" << hr << endl; }
#pragma endregion

        glfwTerminate();
        return 0;
    }
    return -1;
}