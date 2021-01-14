#pragma once
#include "Common.hpp"
namespace libtf {
	enum class UIElementState
	{
		Begin,
		Update,
		End
	};

	class UIElementEventArgs
	{
	public:
		UIElementState				m_state;
		DWORD						m_dwUIElementId;
		PBOOL						m_pfShow = FALSE;

		UIElementEventArgs(UIElementState state, DWORD dwUIElementId) {
			m_state = state;
			m_dwUIElementId = dwUIElementId;
		}

		UIElementEventArgs(DWORD dwUIElementId, PBOOL pfShow) {
			m_state = UIElementState::Begin;
			m_dwUIElementId = dwUIElementId;
			m_pfShow = pfShow;
		}
	};
}
