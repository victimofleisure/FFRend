// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08aug06	initial version
		01		25oct06	add shutdown
		02		27oct06	if no duration, disable shutdown
		03		21jan07	replace AfxGetMainWnd with GetThis
		04		06aug07	in TimerHook, use output frame rate
		05		12aug07	in OnAbort, use StopRecordCheck
		06		23nov07	support Unicode
		07		20may10	update for V2

        record status dialog
 
*/

// RecStatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "RecStatDlg.h"
#include "RecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecStatDlg dialog

#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CRecStatDlg, CToolDlg);

CRecStatDlg::CRecStatDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(CRecStatDlg::IDD, IDR_MAINFRAME, _T("RectStatDlg"), pParent)
{
	//{{AFX_DATA_INIT(CRecStatDlg)
	//}}AFX_DATA_INIT
	m_Main = NULL;
}

void CRecStatDlg::SetTime(CStatic& Ctrl, int Secs)
{
	CString	s;
	CRecordDlg::SecsToTime(Secs, s);
	Ctrl.SetWindowText(s);
}

void CRecStatDlg::Reset()
{
	m_Duration.SetWindowText(_T(""));
	m_Recorded.SetWindowText(_T(""));
	m_Remaining.SetWindowText(_T(""));
	m_Elapsed.SetWindowText(_T(""));
	m_Progress.SetPos(0);
	m_Clock.Reset();
	m_RunAvgFR.Reset();
}

void CRecStatDlg::Start()
{
	Reset();
	CRecordDlg&	RecDlg = m_Main->GetRecordDlg();
	int		dur = RecDlg.GetDuration();
	if (dur) {
		SetTime(m_Duration, dur);
		SetTime(m_Remaining, 0);
	} else {
		m_Duration.SetWindowText(_T("N/A"));
		m_Remaining.SetWindowText(_T("N/A"));
	}
	SetTime(m_Elapsed, 0);
	SetTime(m_Recorded, 0);
	m_AbortBtn.EnableWindow(TRUE);
	m_AbortBtn.SetFocus();
	m_ShutdownChk.EnableWindow(dur && !m_Main->GetBatchMode());
	m_ShutdownChk.SetCheck(FALSE);
}

void CRecStatDlg::Stop()
{
	TimerHook();	// set final values in dialog 
	m_AbortBtn.EnableWindow(FALSE);
	m_ShutdownChk.EnableWindow(FALSE);
}

void CRecStatDlg::TimerHook()
{
	const CRecorder&	Recorder = m_Main->GetRecorder();
	const CRecordInfo&	Info = Recorder.GetInfo();
	int		dur = m_Main->GetRecordDlg().GetDuration();
	int		FramesRecd = Recorder.GetFrameCounter();
	float	FrameRate = Info.m_OutFrameRate;
	float	RecTime = FramesRecd / FrameRate;
	if (dur) {
		m_RunAvgFR.Update(m_Main->GetActualFrameRate());
		float	AvgFR = m_RunAvgFR.GetAvg();
		int		FramesRemain = Info.m_FrameCount - FramesRecd;
		SetTime(m_Remaining, round(AvgFR ? FramesRemain / AvgFR : 0));
		m_Progress.SetPos(round(RecTime / dur * 100));
	}
	SetTime(m_Recorded, round(RecTime));
	SetTime(m_Elapsed, round(m_Clock.Elapsed()));
}

bool CRecStatDlg::Shutdown() const
{
	return(m_ShutdownChk.GetCheck() != 0);
}

void CRecStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecStatDlg)
	DDX_Control(pDX, IDC_RST_SHUTDOWN, m_ShutdownChk);
	DDX_Control(pDX, IDC_RST_ABORT, m_AbortBtn);
	DDX_Control(pDX, IDC_RST_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_RST_REMAINING, m_Remaining);
	DDX_Control(pDX, IDC_RST_RECORDED, m_Recorded);
	DDX_Control(pDX, IDC_RST_ELAPSED, m_Elapsed);
	DDX_Control(pDX, IDC_RST_DURATION, m_Duration);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRecStatDlg, CToolDlg)
	//{{AFX_MSG_MAP(CRecStatDlg)
	ON_BN_CLICKED(IDC_RST_ABORT, OnAbort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecStatDlg message handlers

BOOL CRecStatDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();
	
	m_Main = theApp.GetMain();
	Reset();
	m_RunAvgFR.Create(RUNAVG_SIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecStatDlg::OnAbort() 
{
	if (m_Main->GetBatchMode())
		m_Main->GetJobControlDlg().SkipJob();
	else
		m_Main->Record(FALSE);
}
