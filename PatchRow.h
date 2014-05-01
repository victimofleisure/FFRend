// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23dec06	initial version
		01		26dec06	visually group inputs by plugin
		02		28dec06	handle dialog left-click
		03		03jan07	add plugin enable checkbox
		04		21apr10	port to refactored RowView
		05		04jun10	replace OnPaint with OnEraseBkgnd

        patch bar row dialog
 
*/

#if !defined(AFX_PATCHROW_H__C40E1361_957F_4F96_B3B5_A2353EA4584E__INCLUDED_)
#define AFX_PATCHROW_H__C40E1361_957F_4F96_B3B5_A2353EA4584E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatchRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPatchRow dialog

#include "RowDlg.h"

class CPatchRow : public CRowDlg
{
	DECLARE_DYNAMIC(CPatchRow);
// Construction
public:
	CPatchRow(CWnd* pParent = NULL);   // standard constructor

// Attributes

// Operations

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatchRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPatchRow)
	enum { IDD = IDD_PATCH_ROW };
	CButton	m_Enable;
	CComboBox	m_Source;
	CStatic	m_Input;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPatchRow)
	afx_msg void OnSelchangeSource();
	afx_msg void OnInput();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEnable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants

// Member data
	int		m_SlotIdx;		// index of associated plugin slot
	int		m_NumInputs;	// number of inputs our plugin has
	int		m_InpIdx;		// index of this particular input

// Overrides

// Helpers
	friend class CPatchBar;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATCHROW_H__C40E1361_957F_4F96_B3B5_A2353EA4584E__INCLUDED_)
