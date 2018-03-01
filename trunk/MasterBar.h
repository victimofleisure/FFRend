// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		23apr10	initial version

        master bar
 
*/

#if !defined(AFX_MASTERBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_MASTERBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MasterBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMasterBar window

#include "MySizingControlBar.h"
#include "MasterDlg.h"

class CMasterBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CMasterBar);
// Construction
public:
	CMasterBar();

// Attributes
public:
	CMasterDlg&	GetDlg();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMasterBar)
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CMasterBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Overrides
	BOOL HasGripper() const;

// Member data
	CMasterDlg	m_Dlg;			// child dialog
};

inline CMasterDlg& CMasterBar::GetDlg()
{
	return(m_Dlg);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASTERBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
