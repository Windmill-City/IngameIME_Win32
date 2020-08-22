#pragma once
#include "tf_common.h"
#include <xstring>
namespace libtf {
	enum TFAPI CompositionState
	{
		StartComposition,
		Composing,
		Commit,
		EndComposition
	};

	struct TFAPI CompositionEventArgs
	{
	public:
		CompositionState                m_state;
		std::wstring                    m_strComp = L"";
		LONG                            m_caretPos = 0;
		std::wstring                    m_strCommit = L"";

		CompositionEventArgs(std::wstring compStr, int caretPos);

		CompositionEventArgs(std::wstring commitStr);

		CompositionEventArgs(CompositionState state);
	};
}