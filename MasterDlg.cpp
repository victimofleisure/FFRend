// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15aug06	initial version
		01		23apr10	convert from dialog bar
		02		13jan12	use NotifyEdit wrapper

        master dialog
 
*/

// MasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MasterDlg.h"
#include "UndoCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog

IMPLEMENT_DYNAMIC(CMasterDlg, CDialog);

const CEditSliderCtrl::INFO CMasterDlg::m_SliderInfo = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	-100,	100,	10,		100,	0,		0,		.01f,	2
};

const CCtrlResize::CTRL_LIST	CMasterDlg::m_CtrlList[] = {
	{IDC_MA_SPEED_CAP,		BIND_LEFT},
	{IDC_MA_SPEED_SLIDER,	BIND_LEFT | BIND_RIGHT},
	{IDC_MA_SPEED_EDIT,		BIND_RIGHT},
	{0, 0}	// list terminator
};

CMasterDlg::CMasterDlg(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CMasterDlg)
	//}}AFX_DATA_INIT
}

void CMasterDlg::SetSpeed(double Speed)
{
	m_SpeedSlider.SetVal(Speed);
}

double CMasterDlg::GetSpeedNorm() const
{
	return((m_SpeedSlider.GetValNorm() + 1) / 2);
}

void CMasterDlg::SetSpeedNorm(double Speed)
{
	m_SpeedSlider.SetValNorm(Speed * 2 - 1);
}

void CMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMasterDlg)
	DDX_Control(pDX, IDC_MA_SPEED_CAP, m_SpeedCap);
	DDX_Control(pDX, IDC_MA_SPEED_SLIDER, m_SpeedSlider);
	DDX_Control(pDX, IDC_MA_SPEED_EDIT, m_SpeedEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMasterDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NEN_CHANGED, IDC_MA_SPEED_EDIT, OnChangedSpeedEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg message handlers

BOOL CMasterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_SpeedSlider.SetInfo(m_SliderInfo, &m_SpeedEdit);
	m_SpeedEdit.SetVal(1);
	m_Resize.AddControlList(this, m_CtrlList);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMasterDlg::OnOK()
{
}

void CMasterDlg::OnCancel()
{
}

void CMasterDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CMasterDlg::OnChangedSpeedEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	theApp.GetMain()->NotifyEdit(0, UCODE_MASTER_SPEED, CUndoable::UE_COALESCE);
	theApp.GetEngine().SetSpeed(float(m_SpeedEdit.GetVal()));
	*pResult = 0;
}

BOOL CMasterDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		// if non-system key down and main accelerators, give main a try
		if (pMsg->message == WM_KEYDOWN
		&& AfxGetMainWnd()->SendMessage(UWM_HANDLEDLGKEY, (WPARAM)pMsg))
			return(TRUE);
	}
	return CDialog::PreTranslateMessage(pMsg);
}
