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
#include <boost\smart_ptr\shared_ptr.hpp>
class TextBox
{
public:
	HWND m_hWnd;
	std::wstring m_Text;
	std::wstring m_CompText;
	RECT m_rectComp;
	std::shared_ptr<std::wstring[]> Candidates;
	LONG Count;

	LONG m_CaretPos;

	TextBox(HWND hWnd);

	VOID Draw(HWND hwnd, HDC hdc, PAINTSTRUCT* ps);
	//For IME CandidateWnd
	VOID GetCompExt(RECT* rect);

	VOID onChar(WPARAM wParam, LPARAM lParam);
	VOID onKeyDown(WPARAM wParam, LPARAM lParam);
	VOID onKeyUp(WPARAM wParam, LPARAM lParam);
};
