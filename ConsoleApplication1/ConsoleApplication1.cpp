// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include "../libtf/TextStore.h"

class item {
public:
	typedef boost::signals2::signal<VOID(TextStore*, std::wstring)> signal_CommitStr;
	signal_CommitStr m_sigCommitStr;
};
class item2 {
public:
	TextStore* m_TextStore;
	item* i1;
	item2() {
		m_TextStore = new TextStore((HWND)NULL);
		auto tobind = boost::bind(&item2::func, this, _1, _2);
		m_TextStore->m_sigCommitStr.connect(tobind);
		i1 = new item();
		i1->m_sigCommitStr.connect(tobind);
	}

	VOID func(TextStore* text, std::wstring val) {
		std::cout << val.c_str();
	};
};

int main()
{
	item2* i2 = new item2();
	i2->m_TextStore->m_sigCommitStr(i2->m_TextStore, L"123");
	i2->i1->m_sigCommitStr(i2->m_TextStore, L"123");
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件