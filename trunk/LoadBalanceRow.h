// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01may11	initial version

		load balance dialog row
 
*/

#if !defined(AFX_LOADBALANCEROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_)
#define AFX_LOADBALANCEROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadBalanceRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadBalanceRow dialog

#include "RowDlg.h"
#include "NumEdit.h"
#include "NumSpin.h"
#include "PercentProgressCtrl.h"

class CLoadBalanceRow : public CRowDlg
{
	DECLARE_DYNAMIC(CLoadBalanceRow);
// Construction
public:
	CLoadBalanceRow(CWnd* pParent = NULL);   // standard constructor

// Types

// Constants

// Attributes
	void	SetPluginName(LPCTSTR Name);
	int		GetThreadCount() const;
	void	SetThreadCount(int Count);
	void	SetCPUPercent(int Pct);
	void	SetCPUTime(UINT Time);

// Operations
	void	BlankThreadCount();
	void	EnableThreadCountEdit(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadBalanceRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CLoadBalanceRow)
	enum { IDD = IDD_LOAD_BALANCE_ROW };
	CStatic	m_CPUTime;
	CPercentProgressCtrl	m_CPUPercentBar;
	CNumSpin	m_ThreadCountSpin;
	CNumEdit	m_ThreadCountEdit;
	CStatic	m_PluginName;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CLoadBalanceRow)
	virtual BOOL OnInitDialog();
	afx_msg	void OnChangedThreadCount(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants

// Data members

// Helpers
};

inline void CLoadBalanceRow::SetPluginName(LPCTSTR Name)
{
	m_PluginName.SetWindowText(Name);
}

inline int CLoadBalanceRow::GetThreadCount() const
{
	return(m_ThreadCountEdit.GetIntVal());
}

inline void CLoadBalanceRow::SetThreadCount(int Count)
{
	m_ThreadCountEdit.SetVal(Count);
}

inline void CLoadBalanceRow::BlankThreadCount()
{
	m_ThreadCountEdit.SetWindowText(_T(""));
}

inline void CLoadBalanceRow::SetCPUPercent(int Pct)
{
	m_CPUPercentBar.SetPos(Pct);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADBALANCEROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_)
