#pragma once
#include "tf_common.h"
namespace libtf {
	enum UIElementState
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
		PBOOL						m_pfShow;

		UIElementEventArgs(UIElementState state, DWORD dwUIElementId);
		UIElementEventArgs(DWORD dwUIElementId, PBOOL pfShow);
	};
}
