// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan07	initial version
        01      19jan07	add OnDestroy to clear m_IsVisible
		02		21jan07	replace AfxGetMainWnd with GetThis
		03		29jul07	in OnSize, invalidate if app is paused
        04      01dec07	add context menu
		05		02may10	refactor for engine
		06		26mar12 restore monitor source context menu
		07		24apr12	in OnCreate, window class must specify cursor
		08		23may12	make monitor source a slot index
		09		31may12	in OnWindowPosChanged, clarify renderer monitor test
		10		01jun12	in OnWindowPosChanged, add monitor bypass
		
		monitor control bar
 
*/

// MonitorBar.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MonitorBar.h"
#include "FFRendDoc.h"
#include "FFRendView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMonitorBar dialog

IMPLEMENT_DYNAMIC(CMonitorBar, CMySizingControlBar);

CMonitorBar::CMonitorBar()
{
	m_FrameSize = CSize(0, 0);
}

bool CMonitorBar::CreateFrame(CSize FrameSize)
{
	CRect	r;
	GetClientRect(r);
	CalcMonitorRect(FrameSize, r);
	m_Monitor.MoveWindow(r);
	m_FrameSize = FrameSize;
	return(m_Monitor.CreateFrame(FrameSize));
}

void CMonitorBar::CalcMonitorRect(CSize FrmSz, CRect& r)
{
	CSize	BarSz = r.Size();
	if (BarSz.cx && BarSz.cy) {	// avoid divide by zero
		float	FrmAsp = float(FrmSz.cy) / FrmSz.cx;
		float	BarAsp = float(BarSz.cy) / BarSz.cx;
		if (FrmAsp > BarAsp)
			r.right = round(FrmSz.cx * (float(BarSz.cy) / FrmSz.cy));
		else
			r.bottom = round(FrmSz.cy * (float(BarSz.cx) / FrmSz.cx));
		if (r.right < BarSz.cx)
			r.OffsetRect((BarSz.cx - r.right) / 2, 0);
		if (r.bottom < BarSz.cy)
			r.OffsetRect(0, (BarSz.cy - r.bottom) / 2);
	}
}

BEGIN_MESSAGE_MAP(CMonitorBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CMonitorBar)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMonitorBar message handlers

int CMonitorBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect	r(0, 0, 0, 0);	// arbitrary initial size
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
	LPCTSTR	ClassName = AfxRegisterWndClass(0, theApp.LoadStandardCursor(IDC_ARROW));
	if (!m_Monitor.Create(ClassName, NULL, dwStyle, r, this, IDC_MONITOR_WND))
		return -1;

	return 0;
}

void CMonitorBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid) {
		CRect	r(0, 0, cx, cy);
		CalcMonitorRect(m_FrameSize, r);
		m_Monitor.MoveWindow(r);
	}
}

void CMonitorBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	UINT	flags = lpwndpos->flags;
	if (flags & (SWP_SHOWWINDOW | SWP_HIDEWINDOW)) {	// if showing or hiding
		bool	bShow = (flags & SWP_SHOWWINDOW) != 0;
		CMainFrame	*pMain = theApp.GetMain();
		pMain->GetEngine().SetMonitorBypass(!bShow);
		// renderer should monitor only if monitoring output and we're shown
		pMain->GetRenderer().Monitor(pMain->MonitoringOutput() && bShow);
		if (bShow)
			m_Monitor.UpdateView();
	}
}

void CMonitorBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	CMainFrame	*pMain = theApp.GetMain();
	int	SlotIdx = pMain->GetMonitorSource();
	int	PlugIdx = pMain->GetEngine().MonitorSlotToPlugin(SlotIdx);
	pMain->GetView()->MakePluginPopup(menu, 
		CFFRendView::ID_MONITOR_SOURCE_FIRST, PlugIdx);
	menu.TrackPopupMenu(0, point.x, point.y, pMain);
}
