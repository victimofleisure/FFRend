// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21nov03 initial version
		01		02dec03	right-click must always notify
		02		07dec03	during drag, notify for every mouse move
		03		26dec03	add show selection
		04		29dec03	show selection: notify parent, save and restore Z-pows
		05		08jan04	remove show/hide selection, implement custom draw
		06		26jan04	move normalization here
		07		29jan04	change get/set pos to use double
		08		17feb04	add move selection
		09		19feb04	when moving selection, center it around cursor
		10		22feb04	add undo
		11		19apr04	optimize custom draw; shadow range and number of tics
		12		24jun04	add SetNormDefault
		13		27jun04	fatten short selections so they're always visible
		14		27jun04	add selection change notification types
		15		26sep04	if new selection via right-click, notify both points
		16		29sep04	restore undo state notifies parent now
		17		06nov04	get disabled and selection brush colors from system
		18		27dec04	test SetNormSelection's args for -1 instead of equality
		19		29jul06	derive from CEditSliderCtrl, remove undo and custom draw
		20		22nov06	add undo
		21		07jan08	restore position via base class undo handler
		22		23apr10	remove undo again
		23		23apr10	fix untyped drag tolerance constant
		24		27may10	add HScroll; don't call base class for selection events

		slider with dynamic range selection 
 
*/

// SelectSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SelectSliderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectSliderCtrl

IMPLEMENT_DYNAMIC(CSelectSliderCtrl, CEditSliderCtrl);

static const int DRAG_TOLERANCE = 4;	// in logical coordinates

CSelectSliderCtrl::CSelectSliderCtrl()
{
	m_Dragging = m_Selecting = m_HasSelection = FALSE;
	m_DownPos = m_StartPos = m_EndPos = -1;
	m_Moving = FALSE;
	m_SelectLen = 0;
	m_RangeMin = 0;
	m_RangeMax = 100;
	m_NumTics = 0;
}

CSelectSliderCtrl::~CSelectSliderCtrl()
{
}

void CSelectSliderCtrl::SetRange(int nMin, int nMax)
{
	CEditSliderCtrl::SetRange(m_RangeMin = nMin, m_RangeMax = nMax);
}

void CSelectSliderCtrl::SetNormDefault(float Pos)
{
	CEditSliderCtrl::SetDefaultPos(NormToSlider(Pos));
}

void CSelectSliderCtrl::SetSelection(int nMin, int nMax)
{
	if (nMin == -1 && nMax == -1)
		ClearSel(TRUE);
	else {
		m_StartPos = nMin;
		m_EndPos = nMax;
		m_Selecting = FALSE;
		m_HasSelection = TRUE;
		UpdateSelection(SCN_NONE);	// don't notify
	}
}

void CSelectSliderCtrl::ClearSel(BOOL bRedraw)
{
	m_Selecting = m_HasSelection = FALSE;
	m_StartPos = m_EndPos = -1;
	CEditSliderCtrl::ClearSel(bRedraw);
}

void CSelectSliderCtrl::CreateTicks(int Count)
{
	m_NumTics = Count;
}

void CSelectSliderCtrl::UpdateSelection(int SelType)
{
	CEditSliderCtrl::SetSelection(m_StartPos, m_EndPos);
	CRect	r;
	GetChannelRect(r);
	InvalidateRect(r);	// only need to redraw channel area
	if (SelType != SCN_NONE)
		NotifySelection(SelType);
}

void CSelectSliderCtrl::NotifySelection(int SelType)
{
	GetParent()->PostMessage(WM_HSCROLL, 
		MAKELONG(SB_SLIDER_SELECTION, SelType), (LONG)m_hWnd);
}

void CSelectSliderCtrl::GetNormSelection(float& fMin, float& fMax) const
{
	if (HasSelection()) {
		fMin = float(SliderToNorm(m_StartPos));
		fMax = float(SliderToNorm(m_EndPos));
	} else
		fMin = fMax = -1;
}

void CSelectSliderCtrl::SetNormSelection(float fMin, float fMax)
{
	if (fMin == -1 && fMax == -1)
		ClearSel(TRUE);
	else
		SetSelection(NormToSlider(fMin), NormToSlider(fMax));
}

double CSelectSliderCtrl::GetNormPos() const
{
	return(SliderToNorm(GetPos()));
}

void CSelectSliderCtrl::SetNormPos(double Pos)
{
	SetPos(NormToSlider(Pos));
}

double CSelectSliderCtrl::SliderToNorm(int SliderPos) const
{
	int	RangeMin, RangeMax;
	GetRange(RangeMin, RangeMax);
	return((SliderPos - RangeMin) / double(RangeMax - RangeMin));
}

int CSelectSliderCtrl::NormToSlider(double NormPos) const
{
	int	RangeMin, RangeMax;
	GetRange(RangeMin, RangeMax);
	return(Round(NormPos * (RangeMax - RangeMin) + RangeMin));
}

void CSelectSliderCtrl::MoveSelection(CPoint point)
{
	int	pos = PointToPos(point);
	// clamp position so selection can't be clipped
	int	halfsel = m_SelectLen / 2;
	pos = max(halfsel, min(pos, GetRangeMax() - halfsel));
	m_StartPos = pos - halfsel;
	m_EndPos = pos + halfsel;
	UpdateSelection(SCN_MOVE);
}

BEGIN_MESSAGE_MAP(CSelectSliderCtrl, CEditSliderCtrl)
	//{{AFX_MSG_MAP(CSelectSliderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectSliderCtrl message handlers

void CSelectSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CEditSliderCtrl::OnLButtonDown(nFlags, point);
}

void CSelectSliderCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();
	if (nFlags & MK_CONTROL) {
		if (m_HasSelection) {
			// enter moving mode; mouse slides entire selection
			m_Moving = TRUE;
			m_SelectLen = m_EndPos - m_StartPos;
			// set cursor to horizontal two-headed arrow
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
			SetCapture();	// track mouse even if it leaves our window
			MoveSelection(point);
		}
	} else {
		// if outside slider channel, just clear selection
		CRect	r;
		GetChannelRect(r);
		if (!r.PtInRect(point))
			ClearSel(TRUE);
		else {
			// otherwise save cursor position and enter selecting mode
			m_DownPos = PointToPos(point);
			m_Selecting = TRUE;
			m_Dragging = FALSE;
			SetCapture();	// track mouse even if it leaves our window
		}
		NotifySelection(SCN_NONE);
	}
	CEditSliderCtrl::OnRButtonDown(nFlags, point);
}

void CSelectSliderCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (m_Selecting) {
		// if we never entered dragging mode, extend/trim current selection
		int	SelType = SCN_BOTH;
		if (!m_Dragging) {
			int pos = PointToPos(point);
			// if no current selection, select from current position
			if (!m_HasSelection) {
				m_StartPos = GetPos();
				m_EndPos = GetPos();
			}
			// if we're left of the middle of the current selection,
			// extend/trim start point, otherwise extend/trim end point
			int	midpoint = (m_EndPos + m_StartPos) / 2;
			if (pos < midpoint) {
				m_StartPos = pos;
				SelType = SCN_START;	// notify start point only
			} else {
				m_EndPos = pos;
				SelType = SCN_END;		// notify end point only
			}
			if (!m_HasSelection)		// new selection, notify both points
			   SelType = SCN_BOTH;
			SetSelection(m_StartPos, m_EndPos);
		}
		// exit selecting mode, redraw control, and notify parent
		m_Selecting = FALSE;
		m_HasSelection = TRUE;
		m_Dragging = FALSE;
		UpdateSelection(SelType);
		ReleaseCapture();
	} else {
		if (m_Moving) {
			m_Moving = FALSE;
			ReleaseCapture();
		}
	}
}

void CSelectSliderCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Selecting) {
		int pos = PointToPos(point);
		if (!m_Dragging) {
			// if mouse has moved too far away from 
			// button down position, enter drag mode
			if (abs(pos - m_DownPos) >= DRAG_TOLERANCE) {
				m_Dragging = TRUE;
				m_HasSelection = TRUE;	// drag in progress is a selection
			}
		}
		// if dragging mode, track mouse with selection bar
		if (m_Dragging) {
			m_StartPos = m_DownPos;
			m_EndPos = pos;
			if (m_StartPos > m_EndPos) {	// sort endpoints
				int	i = m_StartPos;
				m_StartPos = m_EndPos;
				m_EndPos = i;
			}
			UpdateSelection(SCN_BOTH);
		}
	} else {
		// if moving mode, mouse slides entire selection
		if (m_Moving)
			MoveSelection(point);
	}
	CEditSliderCtrl::OnMouseMove(nFlags, point);
}

void CSelectSliderCtrl::HScroll(UINT nSBCode, UINT nPos)
{
	if (!(m_Selecting || m_Moving) && nSBCode != SB_SLIDER_SELECTION)
		CEditSliderCtrl::HScroll(nSBCode, nPos);
}
