// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04may11	initial version
		01		01dec11	add advanced options dialog

        engine options dialog
 
*/

// OptsEngineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "OptsEngineDlg.h"
#include "AdvancedEngineOptsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsEngineDlg dialog

#define RK_FRAME_RATE		_T("FrameRate")
#define RK_COLOR_DEPTH		_T("ColorDepth")
#define	RK_RAND_SEED		_T("RandSeed")
#define	RK_RAND_USE_TIME	_T("RandUseTime")
#define	RK_FRAME_TIMEOUT	_T("FrameTimeout")
#define	RK_LOCK_FRAME_RATE	_T("LockFrameRate")
#define	RK_USE_MM_TIMER		_T("UseMMTimer")
#define	RK_RUN_WHILE_LOADING	_T("RunWhileLoading")
#define	RK_FRAME_MEMORY_LIMIT	_T("FrameMemoryLimit")

const double COptsEngineDlg::MIN_FRAME_RATE = 1000.0 / CEngine::FRAME_TIMEOUT * 2;
const double COptsEngineDlg::MAX_FRAME_RATE = 1000;

COptsEngineDlg::COptsEngineDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info),
	m_FrameSizeCtrl(m_FrameSizeCombo, m_FrameWidth, m_FrameHeight, _T(""))
{
	//{{AFX_DATA_INIT(COptsEngineDlg)
	//}}AFX_DATA_INIT
	m_FrameSizeCtrl.Read(m_oi.m_FrameSize);
	m_oi.m_FrameSize = m_FrameSizeCtrl.m_Size;
	theApp.RdReg2Float(RK_FRAME_RATE, m_oi.m_FrameRate);
	theApp.RdReg2UInt(RK_COLOR_DEPTH, m_oi.m_ColorDepth);
	theApp.RdReg2Int(RK_RAND_SEED, m_oi.m_RandSeed);
	theApp.RdReg2Bool(RK_RAND_USE_TIME, m_oi.m_RandUseTime);
	theApp.RdReg2UInt(RK_FRAME_TIMEOUT, m_oi.m_FrameTimeout);
	theApp.RdReg2Bool(RK_LOCK_FRAME_RATE, m_oi.m_LockFrameRate);
	theApp.RdReg2Bool(RK_USE_MM_TIMER, m_oi.m_UseMMTimer);
	theApp.RdReg2Bool(RK_RUN_WHILE_LOADING, m_oi.m_RunWhileLoading);
	theApp.RdReg2UInt(RK_FRAME_MEMORY_LIMIT, m_oi.m_FrameMemoryLimit);
}

COptsEngineDlg::~COptsEngineDlg()
{
	m_FrameSizeCtrl.Write();
	theApp.WrRegFloat(RK_FRAME_RATE, m_oi.m_FrameRate);
	theApp.WrRegInt(RK_COLOR_DEPTH, m_oi.m_ColorDepth);
	theApp.WrRegInt(RK_RAND_SEED, m_oi.m_RandSeed);
	theApp.WrRegBool(RK_RAND_USE_TIME, m_oi.m_RandUseTime);
	theApp.WrRegInt(RK_FRAME_TIMEOUT, m_oi.m_FrameTimeout);
	theApp.WrRegBool(RK_LOCK_FRAME_RATE, m_oi.m_LockFrameRate);
	theApp.WrRegBool(RK_USE_MM_TIMER, m_oi.m_UseMMTimer);
	theApp.WrRegBool(RK_RUN_WHILE_LOADING, m_oi.m_RunWhileLoading);
	theApp.WrRegInt(RK_FRAME_MEMORY_LIMIT, m_oi.m_FrameMemoryLimit);
}

void COptsEngineDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsEngineDlg)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_OPTS_FRAME_SIZE, m_FrameSizeCombo);
	DDX_Control(pDX, IDC_OPTS_FRAME_WIDTH, m_FrameWidth);
	DDX_Control(pDX, IDC_OPTS_FRAME_HEIGHT, m_FrameHeight);
	DDX_Text(pDX, IDC_OPTS_FRAME_RATE, m_oi.m_FrameRate);
	DDV_MinMaxDouble(pDX, m_oi.m_FrameRate, MIN_FRAME_RATE, MAX_FRAME_RATE);
	DDX_Control(pDX, IDC_OPTS_COLOR_DEPTH, m_ColorDepthCombo);
	DDX_Check(pDX, IDC_OPTS_RAND_USE_TIME, m_oi.m_RandUseTime);
	DDX_Text(pDX, IDC_OPTS_RAND_SEED, m_oi.m_RandSeed);
	DDX_Text(pDX, IDC_OPTS_FRAME_TIMEOUT, m_oi.m_FrameTimeout);
	DDV_MinMaxInt(pDX, m_oi.m_FrameTimeout, 1000, 3600000);
	DDX_Check(pDX, IDC_OPTS_LOCK_FRAME_RATE, m_oi.m_LockFrameRate);
	DDX_Check(pDX, IDC_OPTS_USE_MM_TIMER, m_oi.m_UseMMTimer);
}

void COptsEngineDlg::UpdateUI()
{
	BOOL	UseTime = IsDlgButtonChecked(IDC_OPTS_RAND_USE_TIME);
	GetDlgItem(IDC_OPTS_RAND_SEED)->EnableWindow(!UseTime);
}

BEGIN_MESSAGE_MAP(COptsEngineDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsEngineDlg)
	ON_CBN_SELCHANGE(IDC_OPTS_FRAME_SIZE, OnSelchangeFrameSize)
	ON_BN_CLICKED(IDC_OPTS_RAND_USE_TIME, OnRandUseTime)
	ON_BN_CLICKED(IDC_OPTS_ADVANCED, OnAdvanced)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsEngineDlg message handlers

BOOL COptsEngineDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_FrameSizeCtrl.m_Size = m_oi.m_FrameSize;
	m_FrameSizeCtrl.InitCtrls();
	m_ColorDepthCombo.SetCurSel((m_oi.m_ColorDepth >> 3) - 2);
	if (theApp.GetMain()->IsRecording()) {	// if recording, disable engine options
		m_FrameSizeCombo.EnableWindow(FALSE);
		m_FrameWidth.EnableWindow(FALSE);
		m_FrameHeight.EnableWindow(FALSE);
		m_ColorDepthCombo.EnableWindow(FALSE);
		GetDlgItem(IDC_OPTS_FRAME_RATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPTS_FRAME_TIMEOUT)->EnableWindow(FALSE);
	}
	UpdateUI();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsEngineDlg::OnOK() 
{
	CPropertyPage::OnOK();
	m_FrameSizeCtrl.OnOK();
	m_oi.m_FrameSize = m_FrameSizeCtrl.m_Size;
	int	sel = m_ColorDepthCombo.GetCurSel();
	if (sel >= 0 && sel < COLOR_DEPTHS)
		m_oi.m_ColorDepth = (sel + 2) << 3;
}

void COptsEngineDlg::OnSelchangeFrameSize() 
{
	m_FrameSizeCtrl.OnSelChange();
}

void COptsEngineDlg::OnRandUseTime() 
{
	UpdateUI();
}

void COptsEngineDlg::OnAdvanced() 
{
	CAdvancedEngineOptsDlg	dlg;
	dlg.m_RunWhileLoading = m_oi.m_RunWhileLoading;
	dlg.m_FrameMemoryLimit = m_oi.m_FrameMemoryLimit;
	if (dlg.DoModal() == IDOK) {
		m_oi.m_RunWhileLoading = dlg.m_RunWhileLoading != 0;
		m_oi.m_FrameMemoryLimit = dlg.m_FrameMemoryLimit;
	}
}
