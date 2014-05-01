// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		28jul06	initial version
		01		23dec06	in GetText, must release buffer
		02		23nov07	support Unicode
		03		28nov07	add CancelDrag
		04		07dec07	add get/set image
		05		23apr10	in GetText, remove unused constant
		06		17may12	add scrolling during drag
		07		18may12	derive from extended tab control

		tab control with drag reordering

*/

// DragTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DragTabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragTabCtrl

IMPLEMENT_DYNAMIC(CDragTabCtrl, CTabCtrlEx);

CDragTabCtrl::CDragTabCtrl()
{
	m_State = DTS_NONE;
	m_Origin = CPoint(0, 0);
	m_TabIdx = -1;
	m_ScrollDelta = 0;
}

CDragTabCtrl::~CDragTabCtrl()
{
}

void CDragTabCtrl::UpdateCursor(CPoint point)
{
	TCHITTESTINFO	hti;
	hti.pt = point;
	if (HitTest(&hti) >= 0)
		SetCursor(AfxGetApp()->LoadCursor(IDC_DRAG_SINGLE));
	else
 		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CDragTabCtrl::CancelDrag()
{
	if (IsDragging()) {
		m_State = DTS_NONE;
		KillTimer(TIMER_ID);
		ReleaseCapture();
	}
}

BEGIN_MESSAGE_MAP(CDragTabCtrl, CTabCtrlEx)
	//{{AFX_MSG_MAP(CDragTabCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragTabCtrl message handlers

void CDragTabCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	TCHITTESTINFO	hti;
	hti.pt = point;
	int	idx = HitTest(&hti);
	if (idx >= 0) {
		m_State = DTS_TRACK;
		m_Origin = point;
		m_TabIdx = idx;
	}
	SetCapture();
	CTabCtrlEx::OnLButtonDown(nFlags, point);
}

void CDragTabCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	switch (m_State) {
	case DTS_TRACK:
		m_State = DTS_NONE;
		break;
	case DTS_DRAG:
		GetParent()->SendMessage(UWM_TABCTRLDRAG, m_TabIdx, GetDlgCtrlID());
		CancelDrag();
		break;
	}
	ReleaseCapture();
	CTabCtrlEx::OnLButtonUp(nFlags, point);
}

void CDragTabCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch (m_State) {
	case DTS_TRACK:
		if (abs(m_Origin.x - point.x) > DRAG_THRESHOLD) {
			UpdateCursor(point);
			m_State = DTS_DRAG;
			m_ScrollDelta = 0;
		}
		break;
	case DTS_DRAG:
		{
			int	Pos, Range;
			HWND	hSpinCtrl = GetTabScrollPos(Pos, Range);
			if (hSpinCtrl != NULL) {	// if spin control is visible
				CRect	r;
				GetClientRect(r);
				int	delta = 0;	// assume no scrolling
				if (point.x < r.left) {	// if cursor is left of us
					if (Pos > 0)	// if scrolling left is possible
						delta = -1;	// scroll left
				} else {
					if (Pos < Range) {	// if scrolling right is possible
						if (point.x > r.right)	// if cursor is right of us
							delta = 1;	// scroll right
						else {
							::GetWindowRect(hSpinCtrl, r);
							ScreenToClient(r);
							if (r.PtInRect(point))	// if cursor is on spin control
								delta = 1;	// scroll right
						}
					}
				}
				if (delta ^ m_ScrollDelta) {	// if scrolling state changed
					m_ScrollDelta = delta;
					if (delta) {	// if scrolling is needed
						OnTimer(TIMER_ID);
						SetTimer(TIMER_ID, TIMER_PERIOD, NULL);
					} else	// stop scrolling
						KillTimer(TIMER_ID);
				}
			}
		}
		if (!m_ScrollDelta)	// don't compete with timer's cursor update
			UpdateCursor(point);
		break;
	}
	CTabCtrlEx::OnMouseMove(nFlags, point);
}

void CDragTabCtrl::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == TIMER_ID && m_ScrollDelta) {
		int	Pos, Range;
		if (GetTabScrollPos(Pos, Range) != NULL) {	// if spin control is visible
			Pos += m_ScrollDelta;	// update scroll position
			if (Pos >= 0 && Pos <= Range)	// if new scroll position is valid
				SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, Pos));
			else {	// stop scrolling
				KillTimer(TIMER_ID);
				m_ScrollDelta = 0;
			}
			CPoint	pt;
			GetCursorPos(&pt);
			UpdateCursor(pt);
		}
	}
	CTabCtrlEx::OnTimer(nIDEvent);
}
