// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27apr10	initial version
		01		29aug10	remove dirty view flag
		02		18nov11	inline UpdateView

        MIDI setup bar
 
*/

#if !defined(AFX_MIDISETUPBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_MIDISETUPBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiSetupBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupBar window

#include "MySizingControlBar.h"
#include "MidiSetupDlg.h"

class CMidiSetupBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CMidiSetupBar);
// Construction
public:
	CMidiSetupBar();

// Attributes
public:
	CMidiSetupDlg&	GetDlg();

// Operations
public:
	void	UpdateView();

// Handlers

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiSetupBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMidiSetupBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMidiSetupBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	
// Constants

// Member data
	CMidiSetupDlg	m_Dlg;		// child dialog
};

inline void CMidiSetupBar::UpdateView()
{
	if (FastIsVisible())
		m_Dlg.UpdateView();
}

inline CMidiSetupDlg& CMidiSetupBar::GetDlg()
{
	return(m_Dlg);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDISETUPBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
