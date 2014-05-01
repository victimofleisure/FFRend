// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04may11	initial version
		01		23jan12	add check for updates

        view options dialog
 
*/

// OptsViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "OptsViewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsViewDlg dialog

#define RK_SAVE_CHGS_WARN	_T("SaveChgsWarn")
#define RK_UNDO_UNLIMITED	_T("UndoUnlimited")
#define RK_UNDO_LEVELS		_T("UndoLevels")
#define RK_HISTORY_SIZE		_T("HistorySize")
#define RK_VIEW_FREQ		_T("ViewFreq")
#define RK_CACHE_THUMBS		_T("CacheThumbs")
#define RK_THUMB_SIZE		_T("ThumbSize")
#define	RK_MONITOR_QUALITY	_T("MonitorQuality")
#define	RK_CHECK_FOR_UPDATES	_T("CheckForUpdates")

const SIZE COptsViewDlg::m_ThumbSizePreset[THUMB_SIZE_PRESETS] = {
	{64, 48},
	{80, 60},
	{96, 72},
	{128, 96},
	{144, 108},
	{160, 120},
	{192, 144},
};

COptsViewDlg::COptsViewDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsViewDlg)
	//}}AFX_DATA_INIT
	theApp.RdReg2Bool(RK_SAVE_CHGS_WARN, m_oi.m_SaveChgsWarn);
	theApp.RdReg2Bool(RK_UNDO_UNLIMITED, m_oi.m_UndoUnlimited);
	theApp.RdReg2Int(RK_UNDO_LEVELS, m_oi.m_UndoLevels);
	theApp.RdReg2Int(RK_HISTORY_SIZE, m_oi.m_HistorySize);
	theApp.RdReg2Float(RK_VIEW_FREQ, m_oi.m_ViewFreq);
	theApp.RdReg2Bool(RK_CACHE_THUMBS, m_oi.m_CacheThumbs);
	theApp.RdRegStruct(RK_THUMB_SIZE, m_oi.m_ThumbSize, m_oi.m_ThumbSize);
	theApp.RdReg2Bool(RK_MONITOR_QUALITY, m_oi.m_MonitorQuality);
	theApp.RdReg2Bool(RK_CHECK_FOR_UPDATES, m_oi.m_CheckForUpdates);
}

COptsViewDlg::~COptsViewDlg()
{
	theApp.WrRegBool(RK_SAVE_CHGS_WARN, m_oi.m_SaveChgsWarn);
	theApp.WrRegBool(RK_UNDO_UNLIMITED, m_oi.m_UndoUnlimited);
	theApp.WrRegInt(RK_UNDO_LEVELS, m_oi.m_UndoLevels);
	theApp.WrRegInt(RK_HISTORY_SIZE, m_oi.m_HistorySize);
	theApp.WrRegFloat(RK_VIEW_FREQ, m_oi.m_ViewFreq);
	theApp.WrRegBool(RK_CACHE_THUMBS, m_oi.m_CacheThumbs);
	theApp.WrRegStruct(RK_THUMB_SIZE, m_oi.m_ThumbSize);
	theApp.WrRegBool(RK_MONITOR_QUALITY, m_oi.m_MonitorQuality);
	theApp.WrRegBool(RK_CHECK_FOR_UPDATES, m_oi.m_CheckForUpdates);
}

void COptsViewDlg::InitThumbSizeCombo()
{
	CString	s;
	int	sel = -1;
	for (int i = 0; i < THUMB_SIZE_PRESETS; i++) {
		CSize	sz(m_ThumbSizePreset[i]);
		s.Format(_T("%d x %d"), sz.cx, sz.cy);
		m_ThumbSizeCombo.AddString(s);
		if (sz == m_oi.m_ThumbSize)
			sel = i;
	}
	m_ThumbSizeCombo.SetCurSel(sel);
}

void COptsViewDlg::UpdateUI()
{
	BOOL	unlim = IsDlgButtonChecked(IDC_OPTS_UNDO_UNLIMITED);
	if (unlim)
		m_UndoLevelsEdit.SetWindowText(_T(""));
	m_UndoLevelsEdit.EnableWindow(!unlim);
}

void COptsViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsViewDlg)
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_OPTS_SAVE_CHGS_WARN, m_oi.m_SaveChgsWarn);
	DDX_Check(pDX, IDC_OPTS_UNDO_UNLIMITED, m_oi.m_UndoUnlimited);
	DDX_Control(pDX, IDC_OPTS_UNDO_LEVELS, m_UndoLevelsEdit);
	DDX_Text(pDX, IDC_OPTS_HISTORY_SIZE, m_oi.m_HistorySize);
	DDV_MinMaxInt(pDX, m_oi.m_HistorySize, 0, 10000000);
	DDX_Text(pDX, IDC_OPTS_VIEW_FREQ, m_oi.m_ViewFreq);
	DDV_MinMaxDouble(pDX, m_oi.m_ViewFreq, 1, 100);
	DDX_Check(pDX, IDC_OPTS_CACHE_THUMBS, m_oi.m_CacheThumbs);
	DDX_Control(pDX, IDC_OPTS_THUMB_SIZE, m_ThumbSizeCombo);
	DDX_Control(pDX, IDC_OPTS_MONITOR_QUALITY, m_MonitorQualityCombo);
	DDX_Check(pDX, IDC_OPTS_CHECK_FOR_UPDATES, m_oi.m_CheckForUpdates);
}

BEGIN_MESSAGE_MAP(COptsViewDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsViewDlg)
	ON_BN_CLICKED(IDC_OPTS_UNDO_UNLIMITED, OnUndoUnlimited)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsViewDlg message handlers

BOOL COptsViewDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	InitThumbSizeCombo();
	m_MonitorQualityCombo.SetCurSel(m_oi.m_MonitorQuality);
	m_UndoLevelsEdit.SetVal(m_oi.m_UndoLevels);
	m_UndoLevelsEdit.SetRange(0, INT_MAX);
	m_UndoLevelsEdit.SetFormat(CNumEdit::DF_INT);
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsViewDlg::OnOK() 
{
	CPropertyPage::OnOK();
	int	sel = m_ThumbSizeCombo.GetCurSel();
	if (sel >= 0 && sel < THUMB_SIZE_PRESETS)
		m_oi.m_ThumbSize = m_ThumbSizePreset[sel];
	m_oi.m_MonitorQuality = m_MonitorQualityCombo.GetCurSel() != 0;
	m_oi.m_UndoLevels = m_UndoLevelsEdit.GetIntVal();
}

void COptsViewDlg::OnUndoUnlimited() 
{
	if (!IsDlgButtonChecked(IDC_OPTS_UNDO_UNLIMITED)) {
		const COptionsInfo& defs = theApp.GetMain()->GetOptions().GetDefaults();
		m_UndoLevelsEdit.SetVal(defs.m_UndoLevels);
	}
	UpdateUI();
}
