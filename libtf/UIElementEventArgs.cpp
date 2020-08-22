#include "pch.h"
#include "UIElementEventArgs.h"
namespace libtf {
	UIElementEventArgs::UIElementEventArgs(UIElementState state, DWORD dwUIElementId)
	{
		m_state = state;
		m_dwUIElementId = dwUIElementId;
	}

	UIElementEventArgs::UIElementEventArgs(DWORD dwUIElementId, PBOOL pfShow)
	{
		m_state = UIElementState::Begin;
		m_dwUIElementId = dwUIElementId;
		m_pfShow = pfShow;
	}
}