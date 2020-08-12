#pragma once
#include <xstring>
#include <boost/signals2.hpp>
#include "../libtf/TextStore.h"
class TestItem2
{
public:
	typedef boost::signals2::signal<void(TextStore* p, std::wstring val)> signal_CommitStr;
	signal_CommitStr m_sigCommitStr;

	TestItem2();
};
