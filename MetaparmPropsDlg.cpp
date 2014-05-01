// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
		01		01may10	refactor for engine
		02		19jul11	add special handling for plugins with no parameters

        metaparameter properties dialog
 
*/

// MetaparmPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MetaparmPropsDlg.h"
#include "Metaparm.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetaparmPropsDlg dialog

IMPLEMENT_DYNAMIC(CMetaparmPropsDlg, CDialog);

const int CMetaparmPropsDlg::m_ParamPropNameId[PARAM_MIDI_PROPS] = {
	0,							// parameter
	IDS_METAPARM_PP_MOD_ENAB,	// modulation enable
	IDS_METAPARM_PP_MOD_WAVE,	// modulation waveform
	IDS_METAPARM_PP_MOD_FREQ,	// modulation frequency
	IDS_METAPARM_PP_MOD_PW		// modulation pulse width
};

#define MPTC(x) IDS_METAPARM_TARG_CAP_##x

// this table defines a different set of target captions for each MIDI page type
const CMetaparmPropsDlg::TARGET_CAPTION CMetaparmPropsDlg::m_TargCap[MIDI_PAGE_TYPES] = {
//	plugin			parameter		property
	{MPTC(PLUG),	MPTC(PARM),		MPTC(PROP)},	// MPT_PARAM
	{MPTC(TYPE),	MPTC(PLUG),		MPTC(PROP)},	// MPT_PLUGIN
	{MPTC(TYPE),	MPTC(PROP),		0},				// MPT_MISC
};

CMetaparmPropsDlg::CMetaparmPropsDlg(CMetaparm& Parm, CWnd* pParent /*=NULL*/)
	: CDialog(CMetaparmPropsDlg::IDD, pParent),
	m_Parm(Parm)
{
	//{{AFX_DATA_INIT(CMetaparmPropsDlg)
	//}}AFX_DATA_INIT
	m_Frm = NULL;
	m_PrevPageType = -1;
}

bool CMetaparmPropsDlg::IsAssigned() const
{
	return(m_PluginCombo.GetCurSel() > 0);	// first item is unassigned state
}

void CMetaparmPropsDlg::InitCombos()
{
	m_PluginCombo.ResetContent();
	m_PlugIdx.RemoveAll();
	int	slots = theApp.GetEngine().GetSlotCount();
	for (int SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
		if (theApp.GetEngine().IsLoaded(SlotIdx))
			m_PlugIdx.Add(SlotIdx);
	}
	if (m_PlugIdx.GetSize())	// if no plugins, don't include plugin line
		m_PlugIdx.Add(SPI_PLUGIN);
	m_PlugIdx.Add(SPI_MISC);
	// first plugin combo item is reserved for unassigned state; this shifts normal
	// plugin combo items up one, so plugin item indices starts with one, not zero
	m_PluginCombo.AddString(LDS(IDS_METAPARM_UNASSIGNED));
	int	PluginItems = m_PlugIdx.GetSize();
	int	CurSel = 0;
	for (int i = 0; i < PluginItems; i++) {
		CString	s;
		int	PlugIdx = m_PlugIdx[i];
		theApp.GetEngine().GetMidiPlugName(PlugIdx, s);
		if (PlugIdx < 0)	// if special plugin index
			s += LDS(IDS_METAPARM_PROP_SUFFIX);	// add property suffix to name
		m_PluginCombo.AddString(s);
		if (PlugIdx == m_Parm.m_Target.SlotIdx)
			CurSel = i + 1;	// first item is unassigned state
	}
	if (m_Parm.m_Name.IsEmpty())
		CurSel = 0;
	m_PluginCombo.SetCurSel(CurSel);
	SetPlugin(CurSel);
}

bool CMetaparmPropsDlg::GetTarget(TARGET& Target) const
{
	int	PlugSel = m_PluginCombo.GetCurSel();
	if (PlugSel <= 0)
		return(FALSE);	// target is unassigned
	Target.SlotIdx = m_PlugIdx[PlugSel - 1];	// first item is unassigned state
	Target.ParmIdx = m_ParamCombo.GetCurSel();
	Target.PropIdx = m_PropCombo.GetCurSel();
	return(TRUE);
}

void CMetaparmPropsDlg::SetPlugin(int PlugSel)
{
	int	PrevProp = m_PropCombo.GetCurSel();
	PrevProp = max(PrevProp, 0);	// in case previous selection was invalid
	m_ParamCombo.ResetContent();
	m_PropCombo.ResetContent();
	CString	s;
	int	parms = 0;
	int	props = 0;
	if (PlugSel > 0) {	// if target is assigned
		int	PlugIdx = m_PlugIdx[PlugSel - 1];	// first item is unassigned state	
		int	PageType = theApp.GetEngine().GetMidiPageType(PlugIdx);
		parms = theApp.GetEngine().GetMidiParmCount(PlugIdx);
		props = theApp.GetEngine().GetMidiPropCount(PageType);
		if (parms) {
			for (int i = 0; i < parms; i++) {
				theApp.GetEngine().GetMidiParmName(PlugIdx, i, s);
				m_ParamCombo.AddString(s);
			}
			m_ParamCombo.SetCurSel(0);
		} else {	// plugin has no parameters
			if (PageType == MPT_PARAM)	// if showing parameters page
				props = 0;	// suppress properties to avoid confusion
		}
		if (props) {
			for (int i = 0; i < props; i++) {
				theApp.GetEngine().GetMidiPropName(PageType, i, s);
				m_PropCombo.AddString(s);
			}
			if (PageType != m_PrevPageType)	// if page type changed
				PrevProp = 0;	// reset property, else preserve it
			m_PropCombo.SetCurSel(PrevProp);
		}
		m_PrevPageType = PageType;
	} else	// target is unassigned
		m_PrevPageType = -1;
	m_ParamCombo.EnableWindow(parms);
	m_PropCombo.EnableWindow(props);
	UpdateUI();
}

void CMetaparmPropsDlg::MakeMetaparmName(const TARGET& Target, CString& Name)
{
	CString	Suffix;
	int	PageType = theApp.GetEngine().GetMidiPageType(Target.SlotIdx);
	theApp.GetEngine().GetMidiParmName(Target.SlotIdx, Target.ParmIdx, Name);
	if (Target.SlotIdx >= 0) {	// if target is a plugin parameter property
		ASSERT(Target.PropIdx >= 0 && Target.PropIdx < PARAM_MIDI_PROPS);
		Suffix.LoadString(m_ParamPropNameId[Target.PropIdx]);
	} else {
		switch (Target.SlotIdx) {
		case SPI_PLUGIN:	// if target is a plugin property
			theApp.GetEngine().GetMidiPropName(PageType, Target.PropIdx, Suffix);
			break;
		}
	}
	if (Suffix.GetLength()) {	// if suffix isn't empty
		int NameLen = FF_MAX_PARAM_NAME - (Suffix.GetLength() + 1);	// count separator
		Name = Name.Left(NameLen);	// trim name to make room for suffix
		Name.TrimRight();			// remove trailing whitespace from name
		Name += " " + Suffix;		// append separator and suffix to name
	}
}

void CMetaparmPropsDlg::UpdateUI()
{
	int	PageType;
	CString	Name;
	TARGET	Target;
	bool	IsAssigned = GetTarget(Target);
	bool	CanSave = TRUE;
	if (IsAssigned) {
		PageType = theApp.GetEngine().GetMidiPageType(Target.SlotIdx);
		// if showing parameters page but plugin has no parameters,
		// do special handling to prevent possible access violation
		if (PageType == MPT_PARAM 
		&& !theApp.GetEngine().GetMidiParmCount(Target.SlotIdx)) {
			IsAssigned = FALSE;	// target is invalid
			CanSave = FALSE;	// disable save to avoid bogus assignment
		} else
			MakeMetaparmName(Target, Name);
	} else
		PageType = MPT_PARAM;
	// set name, truncated to maximum parameter name length specified by freeframe
	m_NameEdit.SetWindowText(Name.Left(FF_MAX_PARAM_NAME));
	// enable controls if we have a target
	m_NameEdit.EnableWindow(IsAssigned);
	m_RangeStartEdit.EnableWindow(IsAssigned);
	m_RangeStartSpin.EnableWindow(IsAssigned);
	m_RangeEndEdit.EnableWindow(IsAssigned);
	m_RangeEndSpin.EnableWindow(IsAssigned);
	// set target combo box captions
	m_TargCapPlug.SetWindowText(LDS(m_TargCap[PageType].PlugId));
	m_TargCapParm.SetWindowText(LDS(m_TargCap[PageType].ParmId));
	m_TargCapProp.SetWindowText(LDS(m_TargCap[PageType].PropId));
	GetDlgItem(IDOK)->EnableWindow(CanSave);
}

void CMetaparmPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetaparmPropsDlg)
	DDX_Control(pDX, IDC_METAPARM_TARG_CAP_PROP, m_TargCapProp);
	DDX_Control(pDX, IDC_METAPARM_TARG_CAP_PLUG, m_TargCapPlug);
	DDX_Control(pDX, IDC_METAPARM_TARG_CAP_PARM, m_TargCapParm);
	DDX_Control(pDX, IDC_METAPARM_RANGE_START_SPIN, m_RangeStartSpin);
	DDX_Control(pDX, IDC_METAPARM_RANGE_END_SPIN, m_RangeEndSpin);
	DDX_Control(pDX, IDC_METAPARM_RANGE_END_EDIT, m_RangeEndEdit);
	DDX_Control(pDX, IDC_METAPARM_RANGE_START_EDIT, m_RangeStartEdit);
	DDX_Control(pDX, IDC_METAPARM_PROP, m_PropCombo);
	DDX_Control(pDX, IDC_METAPARM_PLUGIN, m_PluginCombo);
	DDX_Control(pDX, IDC_METAPARM_PARAM, m_ParamCombo);
	DDX_Control(pDX, IDC_METAPARM_NAME, m_NameEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMetaparmPropsDlg, CDialog)
	//{{AFX_MSG_MAP(CMetaparmPropsDlg)
	ON_CBN_SELCHANGE(IDC_METAPARM_PLUGIN, OnSelchangePlugin)
	ON_CBN_SELCHANGE(IDC_METAPARM_PARAM, OnSelchangeParam)
	ON_CBN_SELCHANGE(IDC_METAPARM_PROP, OnSelchangeProp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetaparmPropsDlg message handlers

BOOL CMetaparmPropsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Frm = theApp.GetMain();
	InitCombos();
	if (m_Parm.IsAssigned()) {	// if target is assigned
		if (m_Parm.m_Target.ParmIdx < m_ParamCombo.GetCount())
			m_ParamCombo.SetCurSel(m_Parm.m_Target.ParmIdx);
		if (m_Parm.m_Target.PropIdx < m_PropCombo.GetCount())
			m_PropCombo.SetCurSel(m_Parm.m_Target.PropIdx);
	}
	m_NameEdit.SetWindowText(m_Parm.m_Name);
	m_NameEdit.SetLimitText(FF_MAX_PARAM_NAME);
	m_RangeStartEdit.SetVal(m_Parm.m_RangeStart);
	m_RangeStartEdit.SetRange(0, INT_MAX);
	m_RangeStartSpin.SetDelta(.01);
	m_RangeEndEdit.SetVal(m_Parm.m_RangeEnd);
	m_RangeEndEdit.SetRange(0, INT_MAX);
	m_RangeEndSpin.SetDelta(.01);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMetaparmPropsDlg::OnOK() 
{
	if (IsAssigned() && !m_NameEdit.LineLength()) {
		AfxMessageBox(IDS_METAPARM_NO_PARM_NAME);
		m_NameEdit.SetFocus();
		return;	// don't close dialog
	}
	CDialog::OnOK();	// get data from controls
	m_NameEdit.GetWindowText(m_Parm.m_Name);
	GetTarget(m_Parm.m_Target);
	m_Parm.m_RangeStart = float(m_RangeStartEdit.GetVal());
	m_Parm.m_RangeEnd = float(m_RangeEndEdit.GetVal());
}

void CMetaparmPropsDlg::OnSelchangePlugin() 
{
	SetPlugin(m_PluginCombo.GetCurSel());
}

void CMetaparmPropsDlg::OnSelchangeParam() 
{
	UpdateUI();
}

void CMetaparmPropsDlg::OnSelchangeProp() 
{
	UpdateUI();
}
