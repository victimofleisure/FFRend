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

// MetaparmGroupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MetaparmGroupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef ListView_SetCheckState
	#define ListView_SetCheckState(hwndLV, i, fCheck) \
		ListView_SetItemState(hwndLV, i, \
		INDEXTOSTATEIMAGEMASK(((fCheck) + 1)), LVIS_STATEIMAGEMASK)
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetaparmGroupDlg dialog

IMPLEMENT_DYNAMIC(CMetaparmGroupDlg, CDialog);

CMetaparmGroupDlg::CMetaparmGroupDlg(CMetaparmArray& Metaparm, int ParmIdx, CWnd* pParent)
	: m_Metaparm(Metaparm), CDialog(CMetaparmGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMetaparmGroupDlg)
	//}}AFX_DATA_INIT
	m_MasterIdx = Metaparm.GetMaster(ParmIdx);	// get group master
	if (m_MasterIdx < 0)	// if not grouped
		m_MasterIdx = ParmIdx;	// ParmIdx is group master by default
	m_InitRect.SetRectEmpty();
}


void CMetaparmGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetaparmGroupDlg)
	DDX_Control(pDX, IDC_MPG_MASTER, m_MasterName);
	DDX_Control(pDX, IDC_MPG_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMetaparmGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CMetaparmGroupDlg)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetaparmGroupDlg message handlers

const CCtrlResize::CTRL_LIST	m_CtrlInfo[] = {
	{IDC_MPG_MASTER,	BIND_LEFT | BIND_TOP}, 
	{IDC_MPG_LIST,		BIND_ALL}, 
	{IDOK,				BIND_LEFT | BIND_BOTTOM}, 
	{IDCANCEL,			BIND_LEFT | BIND_BOTTOM},
	{0}
};

BOOL CMetaparmGroupDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetWindowRect(m_InitRect);
	m_Resize.AddControlList(this, m_CtrlInfo);
	m_List.InsertColumn(0, NULL, LVCFMT_LEFT, 100);
	m_List.SetExtendedStyle(LVS_EX_CHECKBOXES);
	int	parms = m_Metaparm.GetSize();
	const CMetaparm&	master = m_Metaparm[m_MasterIdx];
	m_MasterName.SetWindowText(master.m_Name);
	for (int i = 0; i < parms; i++) {
		const CMetaparm&	mp = m_Metaparm[i];
		if (i != m_MasterIdx && mp.IsAssigned() && !mp.IsMaster()
		&& (mp.m_Master < 0 || mp.m_Master == m_MasterIdx)) {
			int	pos = m_List.InsertItem(i, mp.m_Name);
			m_List.SetItemData(pos, i);
			if (mp.m_Master == m_MasterIdx)
				ListView_SetCheckState(m_List.m_hWnd, pos, 1);
		}
	}
	m_List.SetColumnWidth(0, LVSCW_AUTOSIZE);	// autosize column to fit data
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMetaparmGroupDlg::OnOK()
{
	int	items = m_List.GetItemCount();
	m_Metaparm.Unlink(m_MasterIdx);
	for (int i = 0; i < items; i++) {
		if (ListView_GetCheckState(m_List.m_hWnd, i)) {
			int	SlaveIdx = m_List.GetItemData(i);
			m_Metaparm.Group(m_MasterIdx, SlaveIdx);
		}
	}
	CDialog::OnOK();
}

void CMetaparmGroupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CMetaparmGroupDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI->ptMinTrackSize.x = m_InitRect.Width();
	lpMMI->ptMinTrackSize.y = m_InitRect.Height();
}
