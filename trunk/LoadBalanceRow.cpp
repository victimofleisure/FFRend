// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01may11	initial version
        01      18nov11	convert from dialog to bar

		load balance dialog row
 
*/

// LoadBalanceRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "LoadBalanceRow.h"
#include "LoadBalanceBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadBalanceRow dialog

IMPLEMENT_DYNAMIC(CLoadBalanceRow, CRowDlg);

CLoadBalanceRow::CLoadBalanceRow(CWnd* pParent /*=NULL*/)
	: CRowDlg(CLoadBalanceRow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadBalanceRow)
	//}}AFX_DATA_INIT
}

void CLoadBalanceRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadBalanceRow)
	DDX_Control(pDX, IDC_LBR_CPU_TIME, m_CPUTime);
	DDX_Control(pDX, IDC_LBR_CPU_PERCENT_BAR, m_CPUPercentBar);
	DDX_Control(pDX, IDC_LBR_THREAD_COUNT_SPIN, m_ThreadCountSpin);
	DDX_Control(pDX, IDC_LBR_THREAD_COUNT_EDIT, m_ThreadCountEdit);
	DDX_Control(pDX, IDC_LBR_PLUGIN_NAME, m_PluginName);
	//}}AFX_DATA_MAP
}

void CLoadBalanceRow::EnableThreadCountEdit(bool Enable)
{
	m_ThreadCountEdit.EnableWindow(Enable);
	m_ThreadCountSpin.EnableWindow(Enable);
}

void CLoadBalanceRow::SetCPUTime(UINT Time)
{
	CString	s;
	s.Format(_T("%d "), Time);	// trailing space provides margin
	m_CPUTime.SetWindowText(s);
}

BEGIN_MESSAGE_MAP(CLoadBalanceRow, CRowDlg)
	//{{AFX_MSG_MAP(CLoadBalanceRow)
	ON_NOTIFY(NEN_CHANGED, IDC_LBR_THREAD_COUNT_EDIT, OnChangedThreadCount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadBalanceRow message handlers

BOOL CLoadBalanceRow::OnInitDialog() 
{
	CRowDlg::OnInitDialog();
	
	m_ThreadCountEdit.SetRange(1, 256);
	m_ThreadCountEdit.SetFormat(CNumEdit::DF_INT);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoadBalanceRow::OnChangedThreadCount(NMHDR* pNMHDR, LRESULT* pResult)
{
	CWnd	*pForm = GetParent();
	CWnd	*pView = pForm->GetParent();
	CLoadBalanceBar	*pBar = DYNAMIC_DOWNCAST(CLoadBalanceBar, pView->GetParent());
	pBar->OnChangedThreadCount(m_RowIdx);
	*pResult = 0;
}
