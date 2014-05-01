// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27apr10	initial version
		01		29aug10	remove dirty view flag
		02		18nov11	inline UpdateView

        MIDI setup bar
 
*/

// MidiSetupBar.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MidiSetupBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupBar

IMPLEMENT_DYNAMIC(CMidiSetupBar, CMySizingControlBar);

CMidiSetupBar::CMidiSetupBar()
{
}

CMidiSetupBar::~CMidiSetupBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupBar message map

BEGIN_MESSAGE_MAP(CMidiSetupBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CMidiSetupBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupBar message handlers

int CMidiSetupBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_Dlg.Create(IDD_MIDI_SETUP, this))
		return -1;

	return 0;
}

void CMidiSetupBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_Dlg.MoveWindow(0, 0, cx, cy);
}

void CMidiSetupBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		UpdateView();
}
