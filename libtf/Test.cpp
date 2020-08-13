#include "pch.h"

#include <iostream>
#include <boost/bind.hpp>
#include "TextStore.h"

void func(const TextStore* ts, const std::wstring str)
{
	std::cout << str.c_str();
}
int main()
{
	TextStore* ts = new TextStore(HWND_DESKTOP);
	ts->m_sigCommitStr.connect(&func);
	ts->m_sigCommitStr(ts, L"Test");
}