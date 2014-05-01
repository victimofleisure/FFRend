// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04may11	initial version
		01		01dec11	add advanced options dialog

        engine options dialog
 
*/

#if !defined(AFX_OPTSENGINEDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTSENGINEDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsEngineDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsEngineDlg dialog

#include "FrameSizeCtrl.h"
#include "OptionsInfo.h"

class COptsEngineDlg : public CPropertyPage
{
// Construction
public:
	COptsEngineDlg(COptionsInfo& Info);
	~COptsEngineDlg();

// Attributes

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsEngineDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptsEngineDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFrameSize();
	afx_msg void OnRandUseTime();
	afx_msg void OnAdvanced();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog data
	//{{AFX_DATA(COptsEngineDlg)
	enum { IDD = IDD_OPTS_ENGINE };
	CComboBox	m_ColorDepthCombo;
	CNumEdit	m_FrameWidth;
	CNumEdit	m_FrameHeight;
	CComboBox	m_FrameSizeCombo;
	//}}AFX_DATA

// Constants
	enum {
		COLOR_DEPTHS = 3,
	};
	static const double	MIN_FRAME_RATE;
	static const double	MAX_FRAME_RATE;

// Member data
	COptionsInfo&	m_oi;		// reference to parent's options info 
	CFrameSizeCtrl	m_FrameSizeCtrl;	// frame size control

// Helpers
	void	UpdateUI();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSENGINEDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
