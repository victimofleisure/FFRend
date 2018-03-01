// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01dec11	initial version

        advanced engine options dialog
 
*/

#if !defined(AFX_ADVANCEDENGINEOPTSDLG_H__B36D632C_B1B1_4A72_9144_1E8E91665226__INCLUDED_)
#define AFX_ADVANCEDENGINEOPTSDLG_H__B36D632C_B1B1_4A72_9144_1E8E91665226__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvancedEngineOptsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvancedEngineOptsDlg dialog

class CAdvancedEngineOptsDlg : public CDialog
{
// Construction
public:
	CAdvancedEngineOptsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAdvancedEngineOptsDlg)
	enum { IDD = IDD_ADV_ENGINE_OPTS };
	BOOL	m_RunWhileLoading;
	UINT	m_FrameMemoryLimit;
	UINT	m_FrameMemoryUsed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvancedEngineOptsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvancedEngineOptsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCEDENGINEOPTSDLG_H__B36D632C_B1B1_4A72_9144_1E8E91665226__INCLUDED_)
