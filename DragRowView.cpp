// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan07	initial version
        01      07mar07	make FindRow const
		02		23nov07	support Unicode
		03		28nov07	add CancelDrag

        drag reorderable row view
 
*/

// DragRowView.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "DragRowView.h"
#include "RowForm.h"
#include "RowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDragRowView dialog

IMPLEMENT_DYNCREATE(CDragRowView, CRowView);

CDragRowView::CDragRowView()
{
	m_DragSrcRow = 0;
	m_ScrollDelta = 0;
	m_ScrollTimer = 0;
	m_ContextSlot = -1;
	m_State = DTS_NONE;
	m_Origin = CPoint(0, 0);
}

void CDragRowView::AutoScroll(CPoint Cursor)
{
	CRect	cr, hr;
	GetClientRect(cr);
	m_Hdr.GetClientRect(hr);
	cr.top += hr.Height() - 1;	// top scroll boundary is bottom of header
	if (Cursor.y < cr.top)	// if cursor is above top boundary
		m_ScrollDelta = Cursor.y - cr.top;	// start scrolling up
	else if (Cursor.y >= cr.bottom)	// if cursor is below bottom boundary
		m_ScrollDelta = Cursor.y - cr.bottom;	// start scrolling down
	else
		m_ScrollDelta = 0;	// stop scrolling
	if (m_ScrollDelta && !m_ScrollTimer)
		m_ScrollTimer = SetTimer(SCROLL_TIMER_ID, SCROLL_DELAY, NULL);
}

int CDragRowView::FindRow(CPoint pt) const
{
	int	rows = GetRows();
	CRect	r;
	for (int i = 0; i < rows; i++) {
		GetRow(i)->GetWindowRect(r);
		ScreenToClient(r);
		if (r.PtInRect(pt))
			return(i);
	}
	return(-1);
}

void CDragRowView::UpdateCursor(CPoint point)
{
	if (m_Form == ChildWindowFromPoint(point))
		SetCursor(AfxGetApp()->LoadCursor(IDC_DRAG_SINGLE));
	else
		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CDragRowView::BeginDrag(int RowIdx)
{
	SetCapture();
	m_DragSrcRow = RowIdx;
	m_State = DTS_TRACK;
	GetCursorPos(&m_Origin);
	ScreenToClient(&m_Origin);
}

void CDragRowView::EndDrag(CPoint Cursor)
{
	if (m_State != DTS_DRAG)
		return;
	ReleaseCapture();
	if (m_ScrollTimer) {
		KillTimer(m_ScrollTimer);
		m_ScrollTimer = 0;
	}
	m_State = DTS_NONE;
	if (m_Form == ChildWindowFromPoint(Cursor)) {
		int	DstRow = FindRow(Cursor);	// find row that contains cursor
		if (DstRow < 0)
			DstRow = GetRows() - 1;	// default to last row
		if (m_DragSrcRow != DstRow)
			OnDrop(m_DragSrcRow, DstRow);
	}
}

void CDragRowView::CancelDrag()
{
	if (IsDragging()) {
		m_DragSrcRow = -1;
		EndDrag(0);
	}
}

void CDragRowView::OnDrop(int SrcRow, int DstRow)
{
}

BEGIN_MESSAGE_MAP(CDragRowView, CRowView)
	//{{AFX_MSG_MAP(CDragRowView)
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDragRowView message handlers

void CDragRowView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	switch (m_State) {
	case DTS_TRACK:
		m_State = DTS_NONE;
		ReleaseCapture();
		break;
	case DTS_DRAG:
		EndDrag(point);
		break;
	}
	CRowView::OnLButtonUp(nFlags, point);
}

void CDragRowView::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch (m_State) {
	case DTS_TRACK:
		if (abs(m_Origin.y - point.y) > DRAG_THRESHOLD) {	// only vertical counts
			UpdateCursor(point);
			m_State = DTS_DRAG;
		}
		break;
	case DTS_DRAG:
		UpdateCursor(point);
		AutoScroll(point);
		break;
	}
	CRowView::OnMouseMove(nFlags, point);
}

void CDragRowView::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == SCROLL_TIMER_ID) {	// other timers may exist
		if (m_ScrollDelta) {
			CPoint	pos = m_Form->GetScrollPosition();
			pos += CSize(0, m_ScrollDelta);
			m_Form->ScrollToPosition(pos);
		}
	}
	CRowView::OnTimer(nIDEvent);
}
