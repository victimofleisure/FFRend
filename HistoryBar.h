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

        history bar
 
*/

#if !defined(AFX_HISTORYBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_HISTORYBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar window

#include "MySizingControlBar.h"
#include "HistoryView.h"

class CHistoryBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CHistoryBar);
// Construction
public:
	CHistoryBar();

// Attributes
public:

// Operations
public:
	void	UpdateView();
	void	Run(bool Enable);
	void	TimerHook();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHistoryBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHistoryBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CHistoryView	*m_View;	// pointer to child view
};

inline void CHistoryBar::Run(bool Enable)
{
	m_View->Run(Enable);
}

inline void CHistoryBar::TimerHook()
{
	m_View->TimerHook();
	UpdateView();
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
