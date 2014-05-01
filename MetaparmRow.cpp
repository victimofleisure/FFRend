// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
		01		25dec07	in OnNotify, call OnRowValChange instead of SetValue
		02		15jan08	replace OnNotify with individual handlers
		03		01may10	port to refactored RowView

        metaplugin parameter row dialog
 
*/

// MetaparmRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MetaparmRow.h"
#include "MetaparmBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetaparmRow dialog

IMPLEMENT_DYNAMIC(CMetaparmRow, CRowDlg);

const CEditSliderCtrl::INFO CMetaparmRow::m_SliderInfo = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	0,		100,	0,		100,	50,		0,		1,		-2
};

CMetaparmRow::CMetaparmRow(CWnd* pParent /*=NULL*/)
	: CRowDlg(CMetaparmRow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMetaparmRow)
	//}}AFX_DATA_INIT
}

void CMetaparmRow::SetParm(const CMetaparm& Parm)
{
	SetCaption(Parm.m_Name);
	SetVal(Parm.m_Value);
	EnableCtrls(Parm.IsAssigned());
}

void CMetaparmRow::GetParm(CMetaparm& Parm) const
{
	m_ParmName.GetWindowText(Parm.m_Name);
	Parm.m_Value = float(m_ParmSlider.GetVal());
}

void CMetaparmRow::SetCaption(const CString& Caption)
{
	CString	s(Caption);
	if (s.GetLength())
		s += _T(":");
	m_ParmName.SetWindowText(s);
}

void CMetaparmRow::EnableCtrls(bool Enable)
{
	m_ParmSlider.EnableWindow(Enable);
	m_ParmEdit.EnableWindow(Enable);
}

void CMetaparmRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetaparmRow)
	DDX_Control(pDX, IDC_METAPARM_GROUP, m_ParmGroupName);
	DDX_Control(pDX, IDC_METAPARM_NAME, m_ParmName);
	DDX_Control(pDX, IDC_METAPARM_SLIDER, m_ParmSlider);
	DDX_Control(pDX, IDC_METAPARM_SPIN, m_ParmSpin);
	DDX_Control(pDX, IDC_METAPARM_EDIT, m_ParmEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMetaparmRow, CRowDlg)
	//{{AFX_MSG_MAP(CMetaparmRow)
	ON_BN_CLICKED(IDC_METAPARM_NAME, OnName)
	ON_NOTIFY(NEN_CHANGED, IDC_METAPARM_EDIT, OnChangedMetaparmEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetaparmRow message handlers

BOOL CMetaparmRow::OnInitDialog() 
{
	CRowDlg::OnInitDialog();
	m_ParmSlider.SetInfo(m_SliderInfo, &m_ParmEdit);
	m_ParmEdit.SetRange(0, 1);
	m_ParmSpin.SetDelta(.01);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMetaparmRow::OnName() 
{
	CDragRowView	*pView = (CDragRowView *)GetView();
	pView->BeginDrag(m_RowIdx);
}

void CMetaparmRow::OnChangedMetaparmEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMetaparmBar	*pb = (CMetaparmBar *)GetNotifyWnd();
	pb->OnRowValChange(m_RowIdx, float(m_ParmSlider.GetVal()));
	*pResult = 0;
}
