// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01may11	initial version
		01		30may11	remove undo
		02		18nov11	convert from dialog to bar
		03		30mar12	use CArrayEx to avoid bounds checking in Release

		load balance bar
 
*/

#if !defined(AFX_LOADBALANCEBAR_H__0AC35BFF_87FD_4FA5_B639_E8A8D39DD0DB__INCLUDED_)
#define AFX_LOADBALANCEBAR_H__0AC35BFF_87FD_4FA5_B639_E8A8D39DD0DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadBalanceBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadBalanceBar dialog

#include "MySizingControlBar.h"
#include "LoadBalanceRow.h"
#include "WndTimer.h"
#include "FFEngine.h"

class CLoadBalanceView;

class CLoadBalanceBar : public CMySizingControlBar
{
	DECLARE_DYNCREATE(CLoadBalanceBar)
// Construction
public:
	CLoadBalanceBar();

// Constants
	enum {	// columns
		COL_PLUGIN_NAME,
		COL_THREAD_COUNT,
		COL_LOAD_BAR,
		COL_LOAD_VALUE,
		COLS
	};

// Types

// Attributes

// Operations
	void	UpdateView();
	void	TimerHook();
	void	OnChangedThreadCount(int RowIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadBalanceBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CLoadBalanceBar)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CLoadBalanceBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		TIMER_ID = 1,
		TIMER_PERIOD = 1000,
	};
	static const CRowView::COLINFO	m_ColInfo[COLS];

// Types
	typedef CArrayEx<ULONGLONG, ULONGLONG> CThreadTimeArray;

// Member data
	CLoadBalanceView	*m_View;	// pointer to row view
	CFFEngine	*m_Engine;	// pointer to engine
	CRect	m_InitRect;		// initial rectangle
	CArrayEx<int, int>	m_ThreadPlugin;	// map thread indices to plugin indices
	CArrayEx<DWORD, DWORD>	m_RunTime;	// array of execution times, one per thread
	DWORD	m_PrevTicks;	// previous tick count

// Helpers
	CLoadBalanceRow	*GetRow(int RowIdx);
	void	PopulateView();
};

class CLoadBalanceView : public CRowView {
public:
	DECLARE_DYNCREATE(CLoadBalanceView);
	CRowDlg	*CreateRow(int Idx);
	CLoadBalanceBar	*m_Parent;
};

inline CLoadBalanceRow *CLoadBalanceBar::GetRow(int RowIdx)
{
	return((CLoadBalanceRow *)m_View->GetRow(RowIdx));
}

inline void CLoadBalanceBar::UpdateView()
{
	if (FastIsVisible())
		PopulateView();
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADBALANCEBAR_H__0AC35BFF_87FD_4FA5_B639_E8A8D39DD0DB__INCLUDED_)
