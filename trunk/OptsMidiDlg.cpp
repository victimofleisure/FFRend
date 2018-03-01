// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04may11	initial version

        MIDI options dialog
 
*/

// OptsMidiDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "OptsMidiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg dialog

#define RK_MIDI_DEVICE		_T("MidiDevice")

COptsMidiDlg::COptsMidiDlg(COptionsInfo& Info)
	: CPropertyPage(IDD),
	m_oi(Info)
{
	//{{AFX_DATA_INIT(COptsMidiDlg)
	//}}AFX_DATA_INIT
	CString	DevName = theApp.RdRegString(RK_MIDI_DEVICE);
	SetMidiDeviceName(DevName);
}

COptsMidiDlg::~COptsMidiDlg()
{
	theApp.WrRegString(RK_MIDI_DEVICE, GetMidiDeviceName());
}

CString COptsMidiDlg::GetMidiDeviceName() const
{
	CString	DevName;
	if (m_MidiDev != CMidiIO::NO_DEVICE)
		DevName = m_DevList[m_MidiDev];
	return(DevName);
}

bool COptsMidiDlg::SetMidiDeviceName(const CString& DevName)
{
	m_MidiDev = CMidiIO::NO_DEVICE;
	int	Devs = CMidiIO::GetInputDeviceNames(m_DevList);
	for (int i = 0; i < Devs; i++) {
		if (m_DevList[i] == DevName) {
			m_MidiDev = i;
			return(TRUE);
		}
	}
	return(FALSE);
}

void COptsMidiDlg::UpdateDevCombo()
{
	m_MidiDevCombo.ResetContent();
	m_MidiDevCombo.AddString(LDS(IDS_OPTS_NO_MIDI_IN));
	for (int i = 0; i < m_DevList.GetSize(); i++)
		m_MidiDevCombo.AddString(m_DevList[i]);
	m_MidiDevCombo.SetCurSel(m_MidiDev + 1);
}

void COptsMidiDlg::UpdateUI()
{
}

void COptsMidiDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptsMidiDlg)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_OPTS_MIDI_DEV, m_MidiDevCombo);
}

BEGIN_MESSAGE_MAP(COptsMidiDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptsMidiDlg)
	ON_BN_CLICKED(IDC_OPTS_MIDI_REFRESH, OnMidiRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg message handlers

BOOL COptsMidiDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	UpdateDevCombo();
	UpdateUI();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptsMidiDlg::OnOK() 
{
	CPropertyPage::OnOK();
	m_MidiDev = m_MidiDevCombo.GetCurSel() - 1;
}

void COptsMidiDlg::OnMidiRefresh() 
{
	CString	DevName;
	int	DevIdx = m_MidiDevCombo.GetCurSel() - 1;
	if (DevIdx >= 0)
		DevName = m_DevList[DevIdx];
	m_MidiDev = CMidiIO::NO_DEVICE;
	theApp.GetMain()->UpdateMidiDevice();	// close previous device
	SetMidiDeviceName(DevName);	// refresh device list
	UpdateDevCombo();
}
