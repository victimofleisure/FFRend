// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25dec07	initial version

        metaparameter group dialog
 
*/

#if !defined(AFX_METAPARMGROUPDLG_H__50E0EE6D_ED82_47B1_8BDC_B53BBA1F9948__INCLUDED_)
#define AFX_METAPARMGROUPDLG_H__50E0EE6D_ED82_47B1_8BDC_B53BBA1F9948__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetaparmGroupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetaparmGroupDlg dialog

#include "CtrlResize.h"
#include "MetaparmArray.h"

class CMetaparmGroupDlg : public CDialog
{
	DECLARE_DYNCREATE(CMetaparmGroupDlg)
// Construction
public:
	CMetaparmGroupDlg(CMetaparmArray& Metaparm, int ParmIdx, CWnd* pParent = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaparmGroupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMetaparmGroupDlg)
	enum { IDD = IDD_METAPARM_GROUP };
	CStatic	m_MasterName;
	CListCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMetaparmGroupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CCtrlResize	m_Resize;		// control resizing object
	CRect	m_InitRect;			// initial size of dialog
	CMetaparmArray&	m_Metaparm;	// reference to metaparameter array
	int		m_MasterIdx;		// index of master of group being edited
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METAPARMGROUPDLG_H__50E0EE6D_ED82_47B1_8BDC_B53BBA1F9948__INCLUDED_)
