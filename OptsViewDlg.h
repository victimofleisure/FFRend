// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04may11	initial version

        view options dialog
 
*/

#if !defined(AFX_OPTSVIEWDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTSVIEWDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsViewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsViewDlg dialog

#include "OptionsInfo.h"

class COptsViewDlg : public CPropertyPage
{
// Construction
public:
	COptsViewDlg(COptionsInfo& Info);
	~COptsViewDlg();

// Attributes

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsViewDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptsViewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnUndoUnlimited();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog data
	//{{AFX_DATA(COptsViewDlg)
	enum { IDD = IDD_OPTS_VIEW };
	CComboBox	m_ThumbSizeCombo;
	CComboBox	m_MonitorQualityCombo;
	CNumEdit	m_UndoLevelsEdit;
	//}}AFX_DATA

// Constants
	enum {
		THUMB_SIZE_PRESETS = 7,
	};
	static const SIZE	m_ThumbSizePreset[THUMB_SIZE_PRESETS];

// Member data
	COptionsInfo&	m_oi;		// reference to parent's options info 

// Helpers
	void	InitThumbSizeCombo();
	void	UpdateUI();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSVIEWDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
