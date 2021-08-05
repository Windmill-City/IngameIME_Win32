#pragma once
#include <xstring>
#include "Common.hpp"
namespace libtf
{
	enum class TF_API CompositionState
	{
		StartComposition,
		Composing,
		Commit,
		EndComposition
	};

	struct TF_API CompositionEventArgs
	{
	public:
		CompositionState m_state;
		LONG m_lCaretPos = 0;
		std::wstring m_strComposition;
		std::wstring m_strCommit;

		CompositionEventArgs(std::wstring strComposition, int caretPos)
		{
			m_state = CompositionState::Composing;
			m_strComposition = strComposition;
			m_lCaretPos = caretPos;
		}

		CompositionEventArgs(std::wstring commitStr)
		{
			m_state = CompositionState::Commit;
			m_strCommit = commitStr;
		}

		CompositionEventArgs(CompositionState state)
		{
			m_state = state;
		}
	};
}