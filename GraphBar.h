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

        graph bar
 
*/

#if !defined(AFX_GRAPHBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_GRAPHBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GraphBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGraphBar window

#include "MySizingControlBar.h"
#include "GraphView.h"

class CGraphBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CGraphBar);
// Construction
public:
	CGraphBar();

// Attributes
public:

// Operations
public:
	void	UpdateView();
	void	OnGraphDone(LPTSTR pPicPath);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGraphBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGraphBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CGraphView	*m_View;		// pointer to child view
};

inline void CGraphBar::OnGraphDone(LPTSTR pPicPath)
{
	m_View->OnGraphDone(pPicPath);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
