#pragma once
#include <xstring>
#include <wtypes.h>
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <msctf.h>

#include <Commctrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")

#include "resource.h"
#include <vector>
#include <map>

#include <atlstr.h>
#include <strsafe.h>
class TextBox
{
public:
	HWND m_hWnd;
	std::wstring m_Text;
	std::wstring m_CompText;
	int m_maxLength = 5;
	RECT m_rectComp;

	TextBox(HWND hWnd);

	VOID Draw(HWND hwnd, HDC hdc, PAINTSTRUCT* ps);
	VOID onKeyDown(WPARAM wParam, LPARAM lParam);
	VOID onKeyUp(WPARAM wParam, LPARAM lParam);
	VOID GetCompExt(RECT* rect);
};
