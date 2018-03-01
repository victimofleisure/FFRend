// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jan07	initial version
		01		23feb09	add non-blocking mode
		02		21mar09	add system menu item to enable/disable docking
		03		08dec09	caption click should always activate and update
		04		06jan10	W64: in GetSizingBar, cast array size to 32-bit
		05		29apr10	derive from SCB miniframe for floating dynamic resize

		sizing bar dock frame that doesn't require idle time
 
*/

// SizingDockFrame.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SizingDockFrame.h"
#include "MySizingControlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizingDockFrame dialog

IMPLEMENT_DYNCREATE(CSizingDockFrame, CSCBMiniDockFrameWnd);

CSizingDockFrame::CSizingDockFrame()
{
	//{{AFX_DATA_INIT(CSizingDockFrame)
	//}}AFX_DATA_INIT
	m_IsNCMoving = FALSE;
	m_IsNonBlocking = FALSE;
}

BEGIN_MESSAGE_MAP(CSizingDockFrame, CSCBMiniDockFrameWnd)
	//{{AFX_MSG_MAP(CSizingDockFrame)
	ON_WM_SYSCOMMAND()
	ON_WM_PARENTNOTIFY()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_INITMENUPOPUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSizingDockFrame message handlers

int CSizingDockFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int	retc = CSCBMiniDockFrameWnd::OnCreate(lpCreateStruct);
	if (!retc) {
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		if (pSysMenu != NULL) {
			CString	name((LPCTSTR)IDS_DOCKABLE);
			pSysMenu->AppendMenu(MF_STRING, USC_DOCKABLE, name);
		}
	}
	return(retc);
}

CMySizingControlBar *CSizingDockFrame::GetSizingBar()
{
	// search our bar list for a sizing control bar; assume there's only one
	int	bars = INT64TO32(m_wndDockBar.m_arrBars.GetSize());
	for (int i = 0; i < bars; i++) {
		CControlBar	*pCB = (CControlBar *)m_wndDockBar.m_arrBars[i];
		CMySizingControlBar	*pSCB = DYNAMIC_DOWNCAST(CMySizingControlBar, pCB);
		if (pSCB != NULL)
			return(pSCB);	// found it
	}
	return(NULL);	// shouldn't happen
}

void CSizingDockFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	// in the default implementation, if there's no idle time, the close button
	// doesn't work: the bar remains visible, and left-clicking the edge of its
	// frame causes the bar to shrink to a tiny rectangle
	switch (nID & 0xFFF0) {	// mask off unreliable low nibble
	case SC_CLOSE:
		{
			// hiding our window via ShowWindow leaves bar quasi-visible
			CMySizingControlBar	*pSCB = GetSizingBar();
			if (pSCB != NULL)
				ShowControlBar(pSCB, FALSE, 0);	// most reliable method
		}
		break;
	case USC_DOCKABLE:
		{
			CMySizingControlBar	*pSCB = GetSizingBar();
			if (pSCB != NULL) {	// if we have a docked sizing bar
				bool	Dockable = (pSCB->m_dwDockStyle & CBRS_ALIGN_ANY) != 0;
				Dockable ^= 1;	// toggle its dockability
				pSCB->EnableDocking(Dockable ? CBRS_ALIGN_ANY : 0);
			}
		}
		break;
	default:
		CSCBMiniDockFrameWnd::OnSysCommand(nID, lParam);
	}
}

void CSizingDockFrame::OnParentNotify(UINT message, LPARAM lParam)
{
	// if left-click in client area, bring bar to front of Z-order
	if (message == WM_LBUTTONDOWN)
		SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	CSCBMiniDockFrameWnd::OnParentNotify(message, lParam);
}

void CSizingDockFrame::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	SetActiveWindow();	// base class doesn't activate us, though it should
	UpdateWindow();	// else client area doesn't fully paint until button up
	if (m_IsNonBlocking) {
		// If you left-click in the caption bar, the message loop is blocked until
		// you release the mouse button, move the mouse, or one second elapses. If
		// you left-click the close (X) button, the message loop is blocked until 
		// you release the mouse button. In an app that drives refresh with timer
		// messages, these default behaviors effectively pause the display, so we
		// must override them, emulating the standard UI as closely as possible.
		switch (nHitTest) {
		case HTCAPTION:
			// SC_MOVE makes cursor jump to middle of caption bar; workaround is
			// store current cursor position, and restore it in OnEnterSizeMove.
			m_NCLBDownPos = point;
			m_IsNCMoving = TRUE;
			ShowCursor(FALSE);	// hide cursor, else jump is briefly visible
			SendMessage(WM_SYSCOMMAND, SC_MOVE);
			break;
		case HTCLOSE:
			// we could do nothing here and close in OnNcLButtonUp, but then button
			// doesn't depress when clicked, which looks like a bug, so instead we
			// close on button down; it's non-standard but it beats being blocked
			{
				// sending ourself SC_CLOSE would be cute, but it doesn't quite
				// work; bar hides, but its FastIsVisible attribute remains true
				CMySizingControlBar	*pSCB = GetSizingBar();
				if (pSCB != NULL)
					ShowControlBar(pSCB, FALSE, 0);	// most reliable method
			}
			break;
		case HTMAXBUTTON:
			ShowWindow(IsZoomed() ? SW_RESTORE : SW_MAXIMIZE);
			break;
		case HTMINBUTTON:
			ShowWindow(IsIconic() ? SW_RESTORE : SW_MINIMIZE);
			break;
		default:
			CSCBMiniDockFrameWnd::OnNcLButtonDown(nHitTest, point);
		}
	} else	// default behavior
		CSCBMiniDockFrameWnd::OnNcLButtonDown(nHitTest, point);
}

void CSizingDockFrame::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
	SetActiveWindow();	// base class doesn't activate us, though it should
	UpdateWindow();	// else client area doesn't fully paint until button up
	if (m_IsNonBlocking) {
		// If you right-click in the caption bar, the message loop is blocked until
		// you release the mouse button. This is unacceptable in a timer-driven app.
		switch (nHitTest) {
		case HTCAPTION:
			// OnNcRButtonUp isn't called for some reason, so the non-client context
			// menu has to be displayed now, instead of when the button is released
			SendMessage(WM_CONTEXTMENU, (LONG)m_hWnd, MAKELONG(point.x, point.y));
			break;
		default:
			CSCBMiniDockFrameWnd::OnNcRButtonDown(nHitTest, point);
		}
	} else	// default behavior
		CSCBMiniDockFrameWnd::OnNcRButtonDown(nHitTest, point);
}

LRESULT CSizingDockFrame::OnEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	if (m_IsNCMoving) {	// if move was initiated by left-click in caption bar
		SetCursorPos(m_NCLBDownPos.x, m_NCLBDownPos.y);	// from OnNcLButtonDown
		ShowCursor(TRUE);
		m_IsNCMoving = FALSE;
	}
	return(0);
}

void CSizingDockFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CMySizingControlBar	*pSCB = GetSizingBar();
	if (pSCB != NULL) {	// if we have a docked sizing bar
		bool	Dockable = (pSCB->m_dwDockStyle & CBRS_ALIGN_ANY) != 0;
		pPopupMenu->CheckMenuItem(USC_DOCKABLE, MF_BYCOMMAND | 
			Dockable ? MF_CHECKED : 0);
	}
}
