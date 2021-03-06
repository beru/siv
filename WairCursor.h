#pragma once

#include "noncopyable.h"

class WaitCursor : private noncopyable
{
public:
	WaitCursor(LPCTSTR lpstrCursor = IDC_WAIT)
	{
		m_hWaitCursor = ::LoadCursor(NULL, lpstrCursor);
		m_hOldCursor = ::SetCursor(m_hWaitCursor);
	}
	
	~WaitCursor()
	{
		::SetCursor(m_hOldCursor);
	}

private:
	HCURSOR m_hWaitCursor;
	HCURSOR m_hOldCursor;
};

