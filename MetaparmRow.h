// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
		01		15jan08	replace OnNotify with individual handlers
		02		01may10	port to refactored RowView

        metaplugin parameter row dialog
 
*/

#if !defined(AFX_METAPARMROW_H__C40E1361_957F_4F96_B3B5_A2353EA4584E__INCLUDED_)
#define AFX_METAPARMROW_H__C40E1361_957F_4F96_B3B5_A2353EA4584E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetaparmRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetaparmRow dialog

#include "RowDlg.h"
#include "EditSliderCtrl.h"
#include "NumEdit.h"
#include "NumSpin.h"

class CMetaparm;

class CMetaparmRow : public CRowDlg
{
	DECLARE_DYNAMIC(CMetaparmRow);
// Construction
public:
	CMetaparmRow(CWnd* pParent = NULL);   // standard constructor

// Attributes
	void	SetParm(const CMetaparm& Parm);
	void	GetParm(CMetaparm& Parm) const;
	void	SetCaption(const CString& Caption);
	void	SetVal(float Val);
	void	SetGroupName(const CString& Name);

// Operations
	void	EnableCtrls(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaparmRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMetaparmRow)
	enum { IDD = IDD_METAPARM_ROW };
	CStatic	m_ParmGroupName;
	CStatic	m_ParmName;
	CEditSliderCtrl	m_ParmSlider;
	CNumSpin	m_ParmSpin;
	CNumEdit	m_ParmEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMetaparmRow)
	virtual BOOL OnInitDialog();
	afx_msg void OnName();
	afx_msg void OnChangedMetaparmEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Overrides

// Constants
	static const CEditSliderCtrl::INFO m_SliderInfo;
};

inline void CMetaparmRow::SetVal(float Val)
{
	m_ParmSlider.SetVal(Val);
}

inline void CMetaparmRow::SetGroupName(const CString& Name)
{
	m_ParmGroupName.SetWindowText(Name);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METAPARMROW_H__C40E1361_957F_4F96_B3B5_A2353EA4584E__INCLUDED_)
