// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		26jul06	initial version
		01		07oct06	add modulation enable
		02		03nov06	add MIDI info
		03		22nov06	add undo
		04		24nov06	add MIDI to parm and plug info
		05		26nov06	if modulating, parm edit must update oscillator
		06		11jul07	in OnParmName, set focus to parm slider
		07		21jul07	in OnContextMenu, set focus to parm slider
		08		23nov07	support Unicode
		09		15jan08	replace OnNotify with individual handlers
		10		29jan08	in GetInfo, add static cast to fix warning
		11		19apr10	remove oscillator
		12		30may11	in OnHScroll remove legacy thumb track case
		13		11nov11	fix keyboard-triggered context menu

		freeframe parameter row dialog

*/

// FFPlugsRow.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "FFPlugsRow.h"
#include "SliderSelectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFFPlugsRow dialog

IMPLEMENT_DYNAMIC(CFFPlugsRow, CRowDlg);

const CEditSliderCtrl::INFO CFFPlugsRow::m_SliderInfo = {
//	Range	Range	Log		Slider	Default	Tic		Edit	Edit
//	Min		Max		Base	Scale	Pos		Count	Scale	Precision
	0,		100,	0,		100,	50,		0,		1,		-2
};

// this list must match the Oscillator::WAVEFORM enum
const int CFFPlugsRow::m_WaveID[WAVEFORMS] = {
	IDS_WAVE_TRIANGLE,
	IDS_WAVE_SINE,
	IDS_WAVE_RAMP_UP,
	IDS_WAVE_RAMP_DOWN,
	IDS_WAVE_SQUARE,
	IDS_WAVE_PULSE,
	IDS_WAVE_RANDOM,
	IDS_WAVE_RANDOM_RAMP
};

CFFPlugsRow::CFFPlugsRow()
	: CRowDlg(CFFPlugsRow::IDD)
{
	//{{AFX_DATA_INIT(CFFPlugsRow)
	//}}AFX_DATA_INIT
}

void CFFPlugsRow::GetInfo(FFPARM_INFO& Info) const
{
	Info.Val = GetVal();
	Info.ModEnab = m_ModEnabChk.GetCheck() != 0;
	Info.ModWave = static_cast<WORD>(m_ModWave.GetCurSel());
	Info.ModFreq = float(m_ModFreq.GetVal());
	Info.ModPW = float(m_ModPW.GetVal());
	m_ParmSlider.GetNormSelection(Info.ModRange.Start, Info.ModRange.End);
}

void CFFPlugsRow::SetInfo(const FFPARM_INFO& Info)
{
	SetVal(Info.Val);
	SetModEnable(Info.ModEnab);
	SetModWave(Info.ModWave);
	SetModFreq(Info.ModFreq);
	SetModPulseWidth(Info.ModPW);
	m_ParmSlider.SetNormSelection(Info.ModRange.Start, Info.ModRange.End);
}

void CFFPlugsRow::SetName(LPCTSTR Name)
{
	m_Name.SetWindowText(CString(Name) + ":");
}

void CFFPlugsRow::SetVal(float Val)
{
	m_ParmSlider.SetVal(Val);
}

void CFFPlugsRow::SetModEnable(bool Enable)
{
	m_ModEnabChk.SetCheck(Enable);
}

void CFFPlugsRow::SetModWave(int Wave)
{
	m_ModWave.SetCurSel(Wave);
}

void CFFPlugsRow::SetModFreq(float Freq)
{
	m_ModFreq.SetVal(Freq);
	UpdateOscFreq();
}

void CFFPlugsRow::SetModPulseWidth(float PulseWidth)
{
	m_ModPW.SetVal(PulseWidth);
}

void CFFPlugsRow::NotifyEdit(int CtrlID)
{
	CWnd	*NotifyWnd = GetNotifyWnd();
	NotifyWnd->SendMessage(UWM_FFROWEDIT, m_RowIdx, CtrlID);
}

bool CFFPlugsRow::EditModRange(LPCTSTR Caption)
{
	CSliderSelectionDlg	dlg;
	GetModRange(dlg.m_Start, dlg.m_End);
	dlg.m_Caption = Caption;
	if (dlg.DoModal() != IDOK)
		return(FALSE);
	if (dlg.m_Start < 0 || dlg.m_End < 0)
		RemoveModRange();
	else {
		if (dlg.m_Start > dlg.m_End) {	// if range is reversed, sort it
			float	t = dlg.m_Start;
			dlg.m_Start = dlg.m_End;
			dlg.m_End = t;
		}
		SetModRange(dlg.m_Start, dlg.m_End);
	}
	return(TRUE);
}

void CFFPlugsRow::SetModRange(float Start, float End)
{
	m_ParmSlider.SetNormSelection(Start, End);
	m_ParmSlider.GetNormSelection(Start, End);	// get quantized values
	if (Start == End)
		m_ParmSlider.ClearSel(TRUE);
	SetValFromSlider();
	NotifyEdit(IDC_FF_PARM_SLIDER);
}

void CFFPlugsRow::SetModRangeStart()
{
	float	Start, End;
	m_ParmSlider.GetNormSelection(Start, End);
	SetModRange(GetVal(), End >= 0 ? End : 1);
}

void CFFPlugsRow::SetModRangeEnd()
{
	float	Start, End;
	m_ParmSlider.GetNormSelection(Start, End);
	SetModRange(Start >= 0 ? Start : 0, GetVal());
}

void CFFPlugsRow::RemoveModRange()
{
	if (SliderHasSel()) {
		m_ParmSlider.ClearSel(TRUE);
		SetValFromSlider();
		NotifyEdit(IDC_FF_PARM_SLIDER);
	}
}

void CFFPlugsRow::GotoModRangeStart()
{
	if (SliderHasSel()) {
		float	Start, End;
		m_ParmSlider.GetNormSelection(Start, End);
		SetVal(Start);
	}
}

void CFFPlugsRow::GotoModRangeEnd()
{
	if (SliderHasSel()) {
		float	Start, End;
		m_ParmSlider.GetNormSelection(Start, End);
		SetVal(End);
	}
}

void CFFPlugsRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFFPlugsRow)
	DDX_Control(pDX, IDC_FF_MOD_ENAB, m_ModEnabChk);
	DDX_Control(pDX, IDC_FF_MOD_PW_SPIN, m_ModPWSpin);
	DDX_Control(pDX, IDC_FF_MOD_PW, m_ModPW);
	DDX_Control(pDX, IDC_FF_MOD_FREQ_SPIN, m_ModFreqSpin);
	DDX_Control(pDX, IDC_FF_MOD_WAVE, m_ModWave);
	DDX_Control(pDX, IDC_FF_MOD_FREQ, m_ModFreq);
	DDX_Control(pDX, IDC_FF_PARM_SPIN, m_ParmSpin);
	DDX_Control(pDX, IDC_FF_PARM_EDIT, m_ParmEdit);
	DDX_Control(pDX, IDC_FF_PARM_SLIDER, m_ParmSlider);
	DDX_Control(pDX, IDC_FF_PARM_NAME, m_Name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFFPlugsRow, CRowDlg)
	//{{AFX_MSG_MAP(CFFPlugsRow)
	ON_CBN_SELCHANGE(IDC_FF_MOD_WAVE, OnSelchangeModWave)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_FF_PARM_NAME, OnParmName)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_FF_MOD_ENAB, OnModEnab)
	ON_NOTIFY(NEN_CHANGED, IDC_FF_PARM_EDIT, OnChangedParmEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_FF_MOD_FREQ, OnChangedModFreq)
	ON_NOTIFY(NEN_CHANGED, IDC_FF_MOD_PW, OnChangedModPW)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFPlugsRow message handlers

BOOL CFFPlugsRow::OnInitDialog() 
{
	CRowDlg::OnInitDialog();

	m_ParmSlider.SetInfo(m_SliderInfo, &m_ParmEdit);
	m_ParmEdit.SetRange(0, 1);
	m_ParmSpin.SetDelta(.01);
	m_ModEnabChk.SetCheck(TRUE);
	CString	s;
	for (int i = 0; i < WAVEFORMS; i++) {
		s.LoadString(m_WaveID[i]);
		m_ModWave.AddString(s);
	}
	m_ModWave.SetCurSel(0);
	m_ModFreq.SetRange(0, INT_MAX);
	m_ModFreqSpin.SetDelta(.01);
	m_ModPW.SetRange(0, 1);
	m_ModPW.SetVal(.5);
	m_ModPWSpin.SetDelta(.01);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFFPlugsRow::UpdateOscFreq()
{
}

void CFFPlugsRow::OnChangedParmEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NotifyEdit(IDC_FF_PARM_EDIT);
	*pResult = 0;
}

void CFFPlugsRow::OnChangedModFreq(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateOscFreq();
	NotifyEdit(IDC_FF_MOD_FREQ);
	*pResult = 0;
}

void CFFPlugsRow::OnChangedModPW(NMHDR* pNMHDR, LRESULT* pResult)
{
	NotifyEdit(IDC_FF_MOD_PW);
	*pResult = 0;
}

void CFFPlugsRow::OnModEnab() 
{
	NotifyEdit(IDC_FF_MOD_ENAB);
}

void CFFPlugsRow::OnSelchangeModWave()
{
	NotifyEdit(IDC_FF_MOD_WAVE);
}

void CFFPlugsRow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == (CScrollBar *)&m_ParmSlider) {
		switch (nSBCode) {
		case SB_SLIDER_SELECTION:
			NotifyEdit(IDC_FF_PARM_SLIDER);
			break;
		}
	}
	CRowDlg::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CFFPlugsRow::OnParmName() 
{
	m_ParmSlider.SetFocus();
}

void CFFPlugsRow::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	m_ParmSlider.SetFocus();
	if (point.x == -1 && point.y == -1) {	// if menu triggered via keyboard
		CRect	r;
		m_ParmSlider.GetWindowRect(r);	// position menu over slider
		point = r.TopLeft() + CSize(10, 10);	// offset looks nicer
	}
	CMenu	menu;
	menu.LoadMenu(IDR_VIEW_ROW_CTX);
	CMenu	*mp = menu.GetSubMenu(0);
	CWnd	*NotifyWnd = GetNotifyWnd();
	theApp.UpdateMenu(NotifyWnd, &menu);
	mp->TrackPopupMenu(0, point.x, point.y, NotifyWnd);
}
