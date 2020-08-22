#include "pch.h"
#include "CompositionEventArgs.h"
namespace libtf {
	CompositionEventArgs::CompositionEventArgs(std::wstring compStr, int caretPos)
	{
		m_state = CompositionState::Composing;
		this->m_strComp = compStr;
		this->m_caretPos = caretPos;
	}

	CompositionEventArgs::CompositionEventArgs(std::wstring commitStr)
	{
		m_state = CompositionState::Commit;
		this->m_strCommit = commitStr;
	}

	CompositionEventArgs::CompositionEventArgs(CompositionState state)
	{
		m_state = state;
	}
}