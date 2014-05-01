// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29mar10	initial version
		01      04may11	convert from dialog to property sheet
		02		01dec11	add run while loading, frame memory limit
		03		23jan12	add check for updates

        options dialog
 
*/

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

IMPLEMENT_DYNAMIC(COptionsDlg, CPropertySheet)

const ENGINE_OPTIONS COptionsDlg::m_DefaultEngineOptions = {
	{320, 240},	// m_FrameSize
	25,			// m_FrameRate
	32,			// m_ColorDepth
	1,			// m_RandSeed
	FALSE,		// m_RandUseTime
	CEngine::FRAME_TIMEOUT,	// m_FrameTimeout
	0,			// m_Reserved
};

const OPTIONS_INFO COptionsDlg::m_DefaultOptionsInfo = {
	TRUE,		// m_SaveChgsWarn
	TRUE,		// m_UndoUnlimited
	TRUE,		// m_CheckForUpdates
	100,		// m_UndoLevels
	256,		// m_HistorySize
	25,			// m_ViewFreq
	TRUE,		// m_CacheThumbs
	{96, 72},	// m_ThumbSize
	FALSE,		// m_MonitorQuality
	TRUE,		// m_LockFrameRate
	FALSE,		// m_UseMMTimer
	TRUE,		// m_RunWhileLoading
	0x1000,		// m_FrameMemoryLimit
};

const COptionsInfo COptionsDlg::m_Defaults(m_DefaultEngineOptions, m_DefaultOptionsInfo);

COptionsDlg::COptionsDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	m_oi(m_Defaults),
	m_EngineDlg(m_oi),
	m_ViewDlg(m_oi),
	m_MidiDlg(m_oi)
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;
	AddPage(&m_EngineDlg);
	AddPage(&m_ViewDlg);
	AddPage(&m_MidiDlg);
	m_CurPage = 0;
}

void COptionsDlg::CreateResetAllButton()
{
	CRect	r, rt;
	GetDlgItem(IDOK)->GetWindowRect(r);
	GetTabControl()->GetWindowRect(rt);
	ScreenToClient(r);
	ScreenToClient(rt);
	int	w = r.Width();
	r.left = rt.left;
	r.right = rt.left + w;
	CString	Title(LPCTSTR(IDS_OPTS_RESET_ALL));
	m_ResetAll.Create(Title, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		r, this, IDS_OPTS_RESET_ALL);
	m_ResetAll.SetFont(GetFont());
	// adjust tab order so new button comes last
	CWnd	*pCancelBtn = GetDlgItem(IDCANCEL);	// assume cancel is now last
	if (pCancelBtn != NULL)
		m_ResetAll.SetWindowPos(pCancelBtn, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void COptionsDlg::RestoreDefaults()
{
	m_oi = m_Defaults;
	m_MidiDlg.SetMidiDeviceName("");
}

BEGIN_MESSAGE_MAP(COptionsDlg, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDS_OPTS_RESET_ALL, OnResetAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

BOOL COptionsDlg::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	CreateResetAllButton();	// create reset all button
	SetActivePage(m_CurPage);	// set current page
	GetInfo(m_PrevInfo);	// save current state; restore on cancel
	return bResult;
}

W64INT COptionsDlg::DoModal() 
{
	W64INT	retc = CPropertySheet::DoModal();
	if (retc == IDCANCEL) {
		SetInfo(m_PrevInfo);	// restore previous state
	}
	return(retc);
}

void COptionsDlg::OnDestroy() 
{
	m_CurPage = GetActiveIndex();
	CPropertySheet::OnDestroy();
}

void COptionsDlg::OnResetAll() 
{
	if (AfxMessageBox(IDS_OPTS_RESTORE_DEFAULTS, MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		EndDialog(IDOK);
		RestoreDefaults();
	}
}
