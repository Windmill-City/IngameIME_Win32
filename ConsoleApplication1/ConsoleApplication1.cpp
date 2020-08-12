// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

class item {
public:
	typedef boost::signals2::signal<void(char*, int)> signal_Func;
	item::signal_Func sigFunc;
};
class item2 {
public:
	item2() {
		item* item_ = new item();
		item_->sigFunc.connect(boost::bind(&item2::func, this, _1, _2));
		item_->sigFunc(new char[] {"Hello World!\n"}, 1);
		item_->sigFunc(new char[] {"Hello\n"}, 2);
	}

	void func(char* text, int val) {
		std::cout << text;
	};
};

int main()
{
	new item2();
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