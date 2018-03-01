// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      07aug06	initial version
		01		27oct06	add option to get duration from AVI file
		02		05nov06	add duration unit
		03		21jan07	replace AfxGetMainWnd with GetThis
		04		02aug07	add queue job checkbox
		05		06aug07	update duration on output frame rate change
		06		07aug07	add get/set info
		07		23nov07	support Unicode
		08		20may10	remove job queue, AVI file duration
		09		28aug10	in OnInitDialog, fix m_CurOutFrameRate init

        record dialog
 
*/

// RecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "RecordDlg.h"
#include "RecordInfo.h"
#include "Persist.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

IMPLEMENT_DYNAMIC(CRecordDlg, CDialog);

#define RK_REC_TAG				_T("Rec")
#define	RK_BIT_COUNT			_T("RecBitCount")
#define	RK_DURATION				_T("RecDuration")
#define	RK_FRAME_COUNT			_T("RecFrameCount")
#define	RK_DURATION_UNIT		_T("RecDurationUnit")
#define	RK_UNLIMITED			_T("RecUnlimited")
#define	RK_USE_AVI_LENGTH		_T("RecUseAviLength")
#define	RK_FRAME_RATE			_T("RecFrameRate")
#define	RK_USE_INP_FRAME_SIZE	_T("RecUseInpFrameSize")
#define	RK_USE_INP_FRAME_RATE	_T("RecUseInpFrameRate")
#define	RK_QUEUE_JOB			_T("RecQueueJob")

const int CRecordDlg::m_PresetBitCount[PBC_PRESETS] = {16, 24, 32};

const SIZE CRecordDlg::DEF_FRAME_SIZE = {320, 240};

CRecordDlg::CRecordDlg(CWnd* pParent /*=NULL*/) :
	CDialog(CRecordDlg::IDD, pParent),
	m_OutFrameSize(m_OutFrameSizeCombo, m_OutFrameWidth, m_OutFrameHeight, RK_REC_TAG)
{
	//{{AFX_DATA_INIT(CRecordDlg)
	m_DurationType = 0;
	m_DurationUnit = 0;
	//}}AFX_DATA_INIT
	m_InpFrameSize = CSize(0, 0);
	m_InpFrameRate = 0;
	m_OutFrameSize.Read(DEF_FRAME_SIZE);
	m_OutFrameRate = CPersist::GetFloat(REG_SETTINGS, RK_FRAME_RATE, DEF_FRAME_RATE);
	m_BitCount = CPersist::GetInt(REG_SETTINGS, RK_BIT_COUNT, DEF_BIT_COUNT);
	m_Duration = CPersist::GetInt(REG_SETTINGS, RK_DURATION, DEF_DURATION);
	m_FrameCount = CPersist::GetInt(REG_SETTINGS, RK_FRAME_COUNT, DEF_FRAME_COUNT);
	m_CurDuration = 0;
	m_CurFrameCount = 0;
	m_CurOutFrameRate = 0;
	m_DurationUnit = CPersist::GetInt(REG_SETTINGS, RK_DURATION_UNIT, 0) != 0;
	m_Unlimited = CPersist::GetInt(REG_SETTINGS, RK_UNLIMITED, FALSE) != 0;
	m_UseAviLength = CPersist::GetInt(REG_SETTINGS, RK_USE_AVI_LENGTH, FALSE) != 0;
	m_UseInpFrameSize = CPersist::GetInt(REG_SETTINGS, RK_USE_INP_FRAME_SIZE, TRUE) != 0;
	m_UseInpFrameRate = CPersist::GetInt(REG_SETTINGS, RK_USE_INP_FRAME_RATE, TRUE) != 0;
	m_QueueJob = CPersist::GetInt(REG_SETTINGS, RK_QUEUE_JOB, FALSE) != 0;
}

CRecordDlg::~CRecordDlg()
{
	m_OutFrameSize.Write();
	CPersist::WriteFloat(REG_SETTINGS, RK_FRAME_RATE, m_OutFrameRate);
	CPersist::WriteInt(REG_SETTINGS, RK_BIT_COUNT, m_BitCount);
	CPersist::WriteInt(REG_SETTINGS, RK_DURATION, m_Duration);
	CPersist::WriteInt(REG_SETTINGS, RK_FRAME_COUNT, m_FrameCount);
	CPersist::WriteInt(REG_SETTINGS, RK_DURATION_UNIT, m_DurationUnit);
	CPersist::WriteInt(REG_SETTINGS, RK_UNLIMITED, m_Unlimited);
	CPersist::WriteInt(REG_SETTINGS, RK_USE_AVI_LENGTH, m_UseAviLength);
	CPersist::WriteInt(REG_SETTINGS, RK_USE_INP_FRAME_SIZE, m_UseInpFrameSize);
	CPersist::WriteInt(REG_SETTINGS, RK_USE_INP_FRAME_RATE, m_UseInpFrameRate);
	CPersist::WriteInt(REG_SETTINGS, RK_QUEUE_JOB, m_QueueJob);
}

void CRecordDlg::FrameToTime(int Frame, float FrameRate, CString& Time)
{
	if (FrameRate > 0) {
		int	Secs = round(Frame / FrameRate);
		Time.Format(_T("%d:%02d:%02d"), Secs / 3600, Secs % 3600 / 60, Secs % 60);
	} else
		Time.Empty();
}

int CRecordDlg::TimeToFrame(LPCTSTR Time, float FrameRate)
{
	static const int PLACES = 3;	// hours, minutes, seconds
	int	ip[PLACES], op[PLACES];	// input and output place arrays
	ZeroMemory(op, sizeof(op));
	int	ps = _stscanf(Time, _T("%d%*[: ]%d%*[: ]%d"), &ip[0], &ip[1], &ip[2]);
	if (ps >= 0)
		CopyMemory(&op[PLACES - ps], ip, ps * sizeof(int));
	return(round(op[0] * FrameRate * 3600 + op[1] * FrameRate * 60 + op[2] * FrameRate));
}

void CRecordDlg::SecsToTime(int Secs, CString& Time)
{
	Time.Format(_T("%d:%02d:%02d"), Secs / 3600, Secs % 3600 / 60, Secs % 60);
}

int CRecordDlg::TimeToSecs(LPCTSTR Time)
{
	static const int PLACES = 3;	// hours, minutes, seconds
	int	ip[PLACES], op[PLACES];	// input and output place arrays
	ZeroMemory(op, sizeof(op));
	int	ps = _stscanf(Time, _T("%d%*[: ]%d%*[: ]%d"), &ip[0], &ip[1], &ip[2]);
	if (ps >= 0)
		CopyMemory(&op[PLACES - ps], ip, ps * sizeof(int));
	return(op[0] * 3600 + op[1] * 60 + op[2]);
}

void CRecordDlg::UpdateDuration()
{
	CString	s;
	if (m_DurationType == DT_AVI_LENGTH) {
		ASSERT(0);	// get duration from AVI file not supported in V2
	} else {
		if (m_DurationEdit.GetModify()) {	// if duration was modified
			m_DurationEdit.GetWindowText(s);
			if (m_DurationUnit == DU_SECONDS) {	// if unit is seconds
				m_CurDuration = TimeToSecs(s);	// assume s contains hh:mm:ss duration
				m_CurFrameCount = SecsToFrames(m_CurDuration);	// convert to frames
			} else {	// unit is frames
				m_CurFrameCount = _ttoi(s);	// assume s contains integer frame count
				m_CurDuration = FramesToSecs(m_CurFrameCount);	// convert to seconds
			}
		}
	}
}

void CRecordDlg::UpdateUI()
{
	// update duration controls
	UpdateData(TRUE);	// retrieve data from radio buttons
	UpdateDuration();
	m_DurationEdit.EnableWindow(m_DurationType == DT_CUSTOM);
	CString	s;
	if (m_DurationUnit == DU_SECONDS)	// if unit is seconds
		SecsToTime(m_CurDuration, s);	// display hh:mm:ss duration
	else	// unit is frames
		s.Format(_T("%d"), m_CurFrameCount);	// display integer frame count
	m_DurationEdit.SetWindowText(s);
	// update output frame size controls
	bool	CustomSz = m_OutFrameSize.IsCustomSize();
	bool	UseInpSz = m_UseInpFrameSizeBtn.GetCheck() != 0;
	if (UseInpSz)
		m_OutFrameSize.SetSize(theApp.GetEngine().GetFrameSize());
	m_OutFrameSizeCombo.EnableWindow(!UseInpSz);
	m_OutFrameWidth.EnableWindow(!UseInpSz && CustomSz);
	m_OutFrameHeight.EnableWindow(!UseInpSz && CustomSz);
	// update output frame rate controls
	bool	UseInpFR = m_UseInpFrameRateBtn.GetCheck() != 0;
	if (UseInpFR)
		m_OutFrameRateEdit.SetVal(m_InpFrameRate);
	m_OutFrameRateEdit.EnableWindow(!UseInpFR);
}

void CRecordDlg::InitBitCountCombo()
{
	CString	s;
	int	sel = PBC_24;
	for (int i = 0; i < PBC_PRESETS; i++) {
		s.Format(_T("%d bit"), m_PresetBitCount[i]);
		m_BitCountCombo.AddString(s);
		if (m_BitCount == m_PresetBitCount[i])
			sel = i;
	}
	m_BitCountCombo.SetCurSel(sel);
}

void CRecordDlg::GetInfo(CRecordInfo& Info) const
{
	Info.m_OutFrameSize		= m_OutFrameSize.m_Size;
	Info.m_OutFrameRate		= m_OutFrameRate;
	Info.m_BitCount			= m_BitCount;
	Info.m_Duration			= m_Duration;
	Info.m_FrameCount		= m_FrameCount;
	Info.m_Unlimited		= m_Unlimited;
	Info.m_UseAviLength		= m_UseAviLength;
	Info.m_UseInpFrameSize	= m_UseInpFrameSize;
	Info.m_UseInpFrameRate	= m_UseInpFrameRate;
	Info.m_QueueJob			= m_QueueJob;
}

void CRecordDlg::SetInfo(const CRecordInfo& Info)
{
	m_OutFrameSize.m_Size	= Info.m_OutFrameSize;
	m_OutFrameRate			= Info.m_OutFrameRate;
	m_BitCount				= Info.m_BitCount;
	m_Duration				= Info.m_Duration;
	m_FrameCount			= Info.m_FrameCount;
	m_Unlimited				= Info.m_Unlimited;
	m_UseAviLength			= Info.m_UseAviLength;
	m_UseInpFrameSize		= Info.m_UseInpFrameSize;
	m_UseInpFrameRate		= Info.m_UseInpFrameRate;
	m_QueueJob				= Info.m_QueueJob;
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordDlg)
	DDX_Control(pDX, IDC_REC_QUEUE_JOB, m_QueueJobChk);
	DDX_Control(pDX, IDC_REC_OUT_FRAME_RATE, m_OutFrameRateEdit);
	DDX_Control(pDX, IDC_REC_USE_INP_FRAME_RATE, m_UseInpFrameRateBtn);
	DDX_Control(pDX, IDC_REC_USE_INP_FRAME_SIZE, m_UseInpFrameSizeBtn);
	DDX_Control(pDX, IDC_REC_DURATION, m_DurationEdit);
	DDX_Control(pDX, IDC_REC_INP_FRAME_RATE, m_InpFrameRateStat);
	DDX_Control(pDX, IDC_REC_INP_FRAME_SIZE, m_InpFrameSizeStat);
	DDX_Control(pDX, IDC_REC_BIT_COUNT, m_BitCountCombo);
	DDX_Control(pDX, IDC_REC_OUT_FRAME_WIDTH, m_OutFrameWidth);
	DDX_Control(pDX, IDC_REC_OUT_FRAME_SIZE, m_OutFrameSizeCombo);
	DDX_Control(pDX, IDC_REC_OUT_FRAME_HEIGHT, m_OutFrameHeight);
	DDX_Radio(pDX, IDC_REC_DURATION_TYPE, m_DurationType);
	DDX_Radio(pDX, IDC_REC_DURATION_UNIT, m_DurationUnit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecordDlg, CDialog)
	//{{AFX_MSG_MAP(CRecordDlg)
	ON_CBN_SELCHANGE(IDC_REC_OUT_FRAME_SIZE, OnSelchangeFrameSize)
	ON_EN_KILLFOCUS(IDC_REC_DURATION, OnKillfocusDuration)
	ON_BN_CLICKED(IDC_REC_USE_INP_FRAME_SIZE, OnUseInpFrameSize)
	ON_BN_CLICKED(IDC_REC_USE_INP_FRAME_RATE, OnUseInpFrameRate)
	ON_BN_CLICKED(IDC_REC_DURATION_TYPE, OnDurationType)
	ON_BN_CLICKED(IDC_REC_DURATION_UNIT, OnDurationUnit)
	ON_BN_CLICKED(IDC_REC_DURATION_TYPE2, OnDurationType)
	ON_BN_CLICKED(IDC_REC_DURATION_TYPE3, OnDurationType)
	ON_BN_CLICKED(IDC_REC_DURATION_UNIT2, OnDurationUnit)
	ON_EN_KILLFOCUS(IDC_REC_OUT_FRAME_RATE, OnKillfocusOutFrameRate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg message handlers

BOOL CRecordDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// set input controls
	CString	s;
	m_InpFrameSize = theApp.GetEngine().GetFrameSize();
	s.Format(_T("%d x %d"), m_InpFrameSize.cx, m_InpFrameSize.cy);
	m_InpFrameSizeStat.SetWindowText(s);
	m_InpFrameRate = theApp.GetEngine().GetFrameRate();
	s.Format(_T("%.2f"), m_InpFrameRate);
	m_InpFrameRateStat.SetWindowText(s);
	// set duration controls
	m_CurOutFrameRate = m_UseInpFrameRate ? m_InpFrameRate : m_OutFrameRate;
	if (m_DurationUnit == DU_SECONDS) {	// if unit is seconds
		m_CurDuration = m_Duration;
		m_CurFrameCount = SecsToFrames(m_Duration);	// convert to frames
	} else {	// unit is frames
		m_CurDuration = FramesToSecs(m_FrameCount);	// convert to seconds
		m_CurFrameCount = m_FrameCount;
	}
	// disable get duration from AVI file option; not supported in V2
	GetDlgItem(IDC_REC_DURATION_TYPE3)->EnableWindow(FALSE);
	m_DurationType = m_Unlimited ? DT_UNLIMITED : DT_CUSTOM;
	UpdateData(FALSE);	// set radio buttons
	// set output controls
	m_UseInpFrameSizeBtn.SetCheck(m_UseInpFrameSize);
	m_OutFrameSize.InitCtrls();
	m_UseInpFrameRateBtn.SetCheck(m_UseInpFrameRate);
	m_OutFrameRateEdit.SetVal(m_OutFrameRate);
	m_OutFrameRateEdit.SetPrecision(2);
	InitBitCountCombo();
	m_QueueJobChk.SetCheck(m_QueueJob);
	UpdateUI();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecordDlg::OnOK() 
{
	if (m_QueueJobChk.GetCheck() && m_DurationType == DT_UNLIMITED) {
		AfxMessageBox(IDS_REC_CANT_QUEUE_UNLIM);
		return;
	}
	// if Enter key was pressed while duration edit control had focus,
	// OnKillfocusDuration hasn't run yet, so update duration explicitly
	UpdateDuration();
	CString	s;
	m_DurationEdit.GetWindowText(s);
	m_Duration = m_CurDuration;
	m_FrameCount = m_CurFrameCount;
	m_Unlimited = m_DurationType == DT_UNLIMITED;
	m_UseAviLength = m_DurationType == DT_AVI_LENGTH;
	m_UseInpFrameSize = m_UseInpFrameSizeBtn.GetCheck() != 0;
	m_OutFrameSize.OnOK();
	m_UseInpFrameRate = m_UseInpFrameRateBtn.GetCheck() != 0;
	m_OutFrameRate = float(m_OutFrameRateEdit.GetVal());
	int	sel = m_BitCountCombo.GetCurSel();
	m_BitCount = sel >= 0 ? m_PresetBitCount[sel] : 0;
	m_QueueJob = m_QueueJobChk.GetCheck() != 0;
	CDialog::OnOK();
}

void CRecordDlg::OnSelchangeFrameSize() 
{
	m_OutFrameSize.OnSelChange();
}

void CRecordDlg::OnKillfocusDuration() 
{
	UpdateUI();
}

void CRecordDlg::OnDurationType() 
{
	UpdateUI();
}

void CRecordDlg::OnUseInpFrameSize() 
{
	UpdateUI();
}

void CRecordDlg::OnUseInpFrameRate() 
{
	m_CurOutFrameRate = m_InpFrameRate;
	m_DurationEdit.SetModify();	// force duration recalc
	UpdateUI();
}

void CRecordDlg::OnDurationUnit() 
{
	UpdateUI();
}

void CRecordDlg::OnKillfocusOutFrameRate() 
{
	// m_OutFrameRate's CNumEdit calls us indirectly via CEdit::OnKillFocus
	// BEFORE updating m_Val, so we can't use GetVal because it will return
	// the previous value; convert the frame rate explicitly instead
	CString	s;
	m_OutFrameRateEdit.GetWindowText(s);
	m_CurOutFrameRate = float(_tstof(s));
	m_DurationEdit.SetModify();	// force duration recalc
	UpdateUI();
}
