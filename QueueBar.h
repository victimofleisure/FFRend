// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		26mar10	initial version
		01		29aug10	remove dirty view flag
		02		18nov11	inline UpdateView

        queue bar
 
*/

#if !defined(AFX_QUEUEBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_QUEUEBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueueBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQueueBar window

#include "MySizingControlBar.h"
#include "QueueView.h"

class CQueueBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CQueueBar);
// Construction
public:
	CQueueBar();

// Attributes
public:

// Operations
public:
	void	UpdateView();
	void	Run(bool Enable);
	void	TimerHook();
	void	SetFrameStyle(UINT Style);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueueBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CQueueBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CQueueBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CQueueView	*m_View;		// pointer to child view
};

inline void CQueueBar::UpdateView()
{
	if (FastIsVisible())
		m_View->UpdateView();
}

inline void CQueueBar::Run(bool Enable)
{
	UpdateView();
}

inline void CQueueBar::TimerHook()
{
	UpdateView();
}

inline void CQueueBar::SetFrameStyle(UINT Style)
{
	m_View->SetFrameStyle(Style);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUEUEBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
