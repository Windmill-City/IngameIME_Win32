#pragma once
#include <xstring>
#include "Common.hpp"
namespace libtf {
	enum class TFAPI CompositionState
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
		LONG                            m_caretPos = 0;
		std::wstring					m_strComposition = L"";
		std::wstring					m_strCommit = L"";

		CompositionEventArgs(std::wstring strComposition, int caretPos) {
			m_state = CompositionState::Composing;
			m_strComposition = strComposition;
			m_caretPos = caretPos;
		}

		CompositionEventArgs(std::wstring commitStr) {
			m_state = CompositionState::Commit;
			m_strCommit = commitStr;
		}

		CompositionEventArgs(CompositionState state) {
			m_state = state;
		}
	};
}