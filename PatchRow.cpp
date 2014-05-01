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
		03		03jan07	add bypass checkbox
		04		21jan07	replace AfxGetMainWnd with GetThis
		05		31jan07	remove dynamic downcast
		06		21apr10	port to refactored RowView
		07		04jun10	replace OnPaint with OnEraseBkgnd
		08		24nov11	in OnInput, set focus to row

        patch bar row dialog
 
*/

// PatchRow.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "PatchRow.h"
#include "MainFrm.h"
#include "FFPluginEx.h"
#include "FFRendDoc.h"
#include "FFRendView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPatchRow dialog

IMPLEMENT_DYNAMIC(CPatchRow, CRowDlg);

CPatchRow::CPatchRow(CWnd* pParent /*=NULL*/)
	: CRowDlg(CPatchRow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPatchRow)
	//}}AFX_DATA_INIT
	m_SlotIdx = 0;
	m_NumInputs = 0;
	m_InpIdx = 0;
}

void CPatchRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatchRow)
	DDX_Control(pDX, IDC_PB_ENABLE, m_Enable);
	DDX_Control(pDX, IDC_PB_SOURCE, m_Source);
	DDX_Control(pDX, IDC_PB_INPUT, m_Input);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPatchRow, CRowDlg)
	//{{AFX_MSG_MAP(CPatchRow)
	ON_CBN_SELCHANGE(IDC_PB_SOURCE, OnSelchangeSource)
	ON_BN_CLICKED(IDC_PB_INPUT, OnInput)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_PB_ENABLE, OnEnable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatchRow message handlers

void CPatchRow::OnSelchangeSource() 
{
	int	sel = m_Source.GetCurSel();	// sel is a plugin index plus one for default
	if (sel >= 0 && theApp.GetEngine().IsLoaded(m_SlotIdx)) {
		theApp.GetView()->Connect(sel - 1, m_SlotIdx, m_InpIdx); // view is undoable
	}
}

void CPatchRow::OnInput() 
{
	// must set focus to ensure that a kill focus message is sent, otherwise
	// if a parameter row numeric edit is in progress, edit may not be saved
	SetFocus();
	theApp.GetEngine().SetCurSel(m_SlotIdx);
	CDragRowView	*pView = (CDragRowView *)GetView();
	pView->BeginDrag(m_RowIdx);
}

BOOL CPatchRow::OnEraseBkgnd(CDC* pDC)
{
	CRect	r;
	GetClientRect(r);
	int	rt;	// rectangle type
	if (m_NumInputs > 1) {	// if multi-input plugin
		if (!m_InpIdx)	// if first input
			rt = BF_RECT & ~BF_BOTTOM;	// don't draw bottom
		else if (m_InpIdx == m_NumInputs - 1)	// else if last input
			rt = BF_RECT & ~BF_TOP;	// don't draw top
		else	// middle input
			rt = BF_LEFT | BF_RIGHT;	// draw sides only
	} else	// single input plugin
		rt = BF_RECT;	// draw entire rectangle
	BOOL	retc = CDialog::OnEraseBkgnd(pDC);	// call base class
	pDC->DrawEdge(r, EDGE_RAISED, rt);	// draw edge over background
	return(retc);
}

void CPatchRow::OnLButtonDown(UINT nFlags, CPoint point) 
{
	OnInput();
	CRowDlg::OnLButtonDown(nFlags, point);
}

void CPatchRow::OnEnable() 
{
	bool	bypass = m_Enable.GetCheck() == 0;	// invert sense
	theApp.GetView()->Bypass(m_SlotIdx, bypass);	// view is undoable
}
