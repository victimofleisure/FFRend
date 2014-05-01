// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      27jun05	initial version
		01		02aug05	add image support
		02		05aug05	add autoscroll
		03		23feb06	fix sloppy autoscroll boundary tests
		04		02aug07	convert for virtual list control
		05		29jan08	comment out unused wizard-generated locals
		06		30jan08	add CancelDrag
		07		06jan10	W64: make OnTimer 64-bit compatible

        virtual list control with drag reordering
 
*/

// DragVirtualListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DragVirtualListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragVirtualListCtrl

IMPLEMENT_DYNAMIC(CDragVirtualListCtrl, CListCtrl);

CDragVirtualListCtrl::CDragVirtualListCtrl()
{
	m_Dragging = FALSE;
	m_ScrollDelta = 0;
	m_ScrollTimer = 0;
	m_InsertPos = 0;
}

CDragVirtualListCtrl::~CDragVirtualListCtrl()
{
}

void CDragVirtualListCtrl::AutoScroll(const CPoint& Cursor)
{
	if (GetItemCount() > GetCountPerPage()) {	// if view is scrollable
		CRect	cr, ir, hr;
		GetClientRect(cr);
		GetHeaderCtrl()->GetClientRect(hr);
		cr.top += hr.Height() - 1;	// top scroll boundary is bottom of header
		GetItemRect(0, ir, LVIR_BOUNDS);
		int	Offset = ir.Height() / 2;	// vertical scrolling is quantized to lines
		if (Cursor.y < cr.top)	// if cursor is above top boundary
			m_ScrollDelta = Cursor.y - cr.top - Offset;	// start scrolling up
		else if (Cursor.y >= cr.bottom)	// if cursor is below bottom boundary
			m_ScrollDelta = Cursor.y - cr.bottom + Offset;	// start scrolling down
		else
			m_ScrollDelta = 0;	// stop scrolling
	} else
		m_ScrollDelta = 0;
	if (m_ScrollDelta && !m_ScrollTimer)
		m_ScrollTimer = SetTimer(TIMER_ID, SCROLL_DELAY, NULL);
}

void CDragVirtualListCtrl::UpdateCursor(CPoint point)
{
	if (ChildWindowFromPoint(point) == this)
		SetCursor(AfxGetApp()->LoadCursor(IDC_DRAG_SINGLE));
	else
 		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CDragVirtualListCtrl::CancelDrag()
{
	if (m_Dragging) {
		m_Dragging = FALSE;
		ReleaseCapture();
	}
}

BEGIN_MESSAGE_MAP(CDragVirtualListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CDragVirtualListCtrl)
	ON_NOTIFY_REFLECT_EX(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragVirtualListCtrl message handlers

BOOL CDragVirtualListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_Dragging = TRUE;
	SetCapture();
	int	ResID = GetSelectedCount() > 1 ? IDC_DRAG_MULTI : IDC_DRAG_SINGLE;
	SetCursor(AfxGetApp()->LoadCursor(ResID));
	*pResult = 0;
	return(FALSE);	// let parent handle notification too
}

void CDragVirtualListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CListCtrl::OnLButtonUp(nFlags, point);
	// NOTE that only report view is supported
	if (m_Dragging) {
		m_Dragging = FALSE;
		ReleaseCapture();
		UINT	flags;
		int	InsPos = HitTest(point, &flags);
		if (InsPos < 0) {
			if (flags & LVHT_ABOVE)
				InsPos = 0;
			else	// assume end of list
				InsPos = GetItemCount();	// this works, amazingly
		}
		m_InsertPos = InsPos;	// update insert position member
		// notify the parent window
		NMLISTVIEW	lvh;
		ZeroMemory(&lvh, sizeof(lvh));
		lvh.hdr.hwndFrom = m_hWnd;
		lvh.hdr.idFrom = GetDlgCtrlID();
		lvh.hdr.code = ULVN_REORDER;
		GetParent()->SendMessage(WM_NOTIFY, lvh.hdr.idFrom, long(&lvh));
		KillTimer(m_ScrollTimer);
		m_ScrollTimer = 0;
		EnsureVisible(min(InsPos, GetItemCount() - 1), FALSE);
	}
}

void CDragVirtualListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		UpdateCursor(point);
		AutoScroll(point);
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

void CDragVirtualListCtrl::OnTimer(W64UINT nIDEvent) 
{
	if (nIDEvent == TIMER_ID) {
		if (m_ScrollDelta)
			Scroll(CSize(0, m_ScrollDelta));
	} else
		CListCtrl::OnTimer(nIDEvent);
}
