// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		23apr10	initial version

        master bar
 
*/

// MasterBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MasterBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMasterBar

IMPLEMENT_DYNAMIC(CMasterBar, CMySizingControlBar);

CMasterBar::CMasterBar()
{
}

BOOL CMasterBar::HasGripper() const
{
	return(FALSE);	// gripper looks ugly on a toolbar, use caption instead
}

/////////////////////////////////////////////////////////////////////////////
// CMasterBar message map

BEGIN_MESSAGE_MAP(CMasterBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CMasterBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PARENTNOTIFY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMasterBar message handlers

int CMasterBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_Dlg.Create(IDD_MASTER, this))
		return -1;

	CRect	r;
	m_Dlg.GetWindowRect(r);
	ScreenToClient(r);
	r.BottomRight() += CSize(8, 10);
    m_szMinHorz = r.Size();
    m_szMinVert = r.Size();
    m_szMinFloat = r.Size();
    m_szFloat = r.Size();
	
	return 0;
}

void CMasterBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_Dlg.MoveWindow(0, 0, cx, cy);
}

CSize CMasterBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CMySizingControlBar::CalcFixedLayout(bStretch, bHorz);	// must call base class
	// don't stretch to frame width when docked; use floating size
	return m_szFloat;
}

void CMasterBar::OnParentNotify(UINT message, LPARAM lParam) 
{
	if (message == WM_LBUTTONDOWN) {
		// since we have no gripper, use slider caption as a gripper
		POINTS	pts = MAKEPOINTS(lParam);
		CPoint	pt;
		POINTSTOPOINT(pt, pts);
		CRect	r;
		m_Dlg.GetDlgItem(IDC_MA_SPEED_CAP)->GetWindowRect(r);
		ScreenToClient(r);
		if (r.PtInRect(pt))	// if click on caption
			PostMessage(WM_LBUTTONDOWN, 0, lParam);	// enter drag mode
	}
	CMySizingControlBar::OnParentNotify(message, lParam);
}
