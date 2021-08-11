#include "libtf.h"
#include <iostream>
#include <string>

#include <glfw/glfw3.h>

using namespace std;
libtf_pInputContext ctx;

void error_callback(int error, const char *description)
{
    cout << "Error:" << error << description << endl;
}

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

#pragma region Toggle IMState
    static bool toggleDown = false;
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !toggleDown)
    {
        toggleDown = true;
        bool imState;
        libtf_get_im_state(ctx, &imState);
        if (FAILED(libtf_set_im_state(ctx, !imState)))
        {
            cout << "Failed to set im state!" << endl;
            return;
        }
        cout << "IM State:" << !imState << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE)
    {
        toggleDown = false;
    }
#pragma endregion
#pragma region FullScreen
    static bool fullScreenDown = false;
    static bool isFullScreen = false;
    if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS && !fullScreenDown)
    {
        fullScreenDown = true;
        isFullScreen = !isFullScreen;
        if (FAILED(libtf_set_full_screen(ctx, isFullScreen)))
        {
            cout << "Failed to set full screen!" << endl;
            return;
        }
        cout << "Full Screen:" << isFullScreen << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_RELEASE)
    {
        fullScreenDown = false;
    }
#pragma endregion
}

void window_focus_callback(GLFWwindow *window, int focused)
{
    if (focused)
    {
#pragma region libtf setFocus
        if (FAILED(libtf_set_focus_wnd(ctx, GetForegroundWindow())))
        {
            cout << "Failed to focus window!" << endl;
        }
#pragma endregion
    }
}

void memory_dump(void *ptr, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        if (i % 8 == 0 && i != 0)
            printf(" ");
        if (i % 16 == 0 && i != 0)
            printf("\n");
        printf("%02x ", *((uint8_t *)ptr + i));
    }
    printf("\n");
}

#pragma region libtf callbacks
void onBoundingBox(libtf_BoundingBox_t *box)
{
    box->left = 0;
    box->right = 0;
    box->top = 0;
    box->bottom = 20; //Font height
    printf("Fetch Bounding Box:%d, %d,%d, %d\n", box->left, box->right, box->top, box->bottom);
}
void onConversionMode(libtf_ConversionMode mode)
{
    printf("Conversion Mode:%d\n", mode);
}
void onSentenceMode(libtf_SentenceMode mode)
{
    printf("Sentence Mode:%d\n", mode);
}
void onComposition(libtf_Composition_t composition)
{
    setlocale(LC_ALL, "");
    switch (composition.state)
    {
    case libtf_CompositionBegin:
        printf("Composition Begin\n");
        break;
    case libtf_CompositionUpdate:
        printf("Composition Update\n");
        printf("PreEdit:%ls\n", composition.preEdit);
        memory_dump(composition.preEdit, 32);
        printf("Selection:%d, %d\n", composition.selection[0], composition.selection[1]);
        break;
    case libtf_CompositionEnd:
        printf("Composition End\n");
        break;
    default:
        break;
    }
}
void onCommit(libtf_Commit commit)
{
    setlocale(LC_ALL, "");
    printf("Commit: %ls\n", commit);
}
void onCandidateList(libtf_CandidateList_t list)
{
    setlocale(LC_ALL, "");
    switch (list.state)
    {
    case libtf_CandidateListBegin:
        printf("Candidate List Begin\n");
        break;
    case libtf_CandidateListUpdate:
        printf("Composition Update\n");
        printf("Total:%d; Page:%d, %d; CurSel:%d\n", list.totalCount, list.pageStart, list.pageEnd, list.curSelection);

        for (size_t i = list.pageStart; i <= list.pageEnd; i++)
        {
            printf("[%zd]%ls\n", i - list.pageStart + 1, list.candidates[i]);
        }
        break;
    case libtf_CandidateListEnd:
        printf("Candidate List End\n");
        break;
    default:
        break;
    }
}
#pragma endregion

int main()
{
    glfwSetErrorCallback(error_callback);
    if (glfwInit())
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *window = glfwCreateWindow(800, 600, "libtf", NULL, NULL);
        if (!window)
        {
            cout << "Failed to create window!" << endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

#pragma region libtf init
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        HRESULT hr;
        if (FAILED(hr = libtf_create_ctx(&ctx)))
        {
            cout << "Failed to create input context!" << hr << endl;
            glfwTerminate();
            return -1;
        }
        //Once the window get focus, you need to call libtf_set_focus_wnd
        glfwSetWindowFocusCallback(window, window_focus_callback);

        libtf_set_conversion_mode_callback(ctx, onConversionMode);
        libtf_set_sentence_mode_callback(ctx, onSentenceMode);
        libtf_set_composition_callback(ctx, onComposition);
        libtf_set_commit_callback(ctx, onCommit);
        libtf_set_candidate_list_callback(ctx, onCandidateList);
        libtf_set_bounding_box_callback(ctx, onBoundingBox);
#pragma endregion

        while (!glfwWindowShouldClose(window))
        {
            process_input(window);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

#pragma region libtf dispose
        if (FAILED(hr = libtf_dispose_ctx(ctx)))
        {
            cout << "Failed to dispose input context!" << hr << endl;
        }
#pragma endregion

        glfwTerminate();
        return 0;
    }
    return -1;
}