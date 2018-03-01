// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03nov06	initial version
		01		23nov07	support Unicode
		02		15jan08	replace OnNotify with individual handlers
		03		29jan08	in GetInfo, use SetMsg to fix warnings
		04		27apr10	rename row dialog base class

		MIDI setup dialog row
 
*/

// MidiSetupRow.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MidiSetupRow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupRow dialog

IMPLEMENT_DYNAMIC(CMidiSetupRow, CRowDlg);

const COLORREF CMidiSetupRow::m_SelColor = RGB(0, 255, 0);
const CBrush CMidiSetupRow::m_SelBrush(m_SelColor);

CMidiSetupRow::CMidiSetupRow(CWnd* pParent /*=NULL*/)
	: CRowDlg(CMidiSetupRow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMidiSetupRow)
	//}}AFX_DATA_INIT
	m_Selected = FALSE;
}

void CMidiSetupRow::SetCaption(LPCTSTR Title)
{
	m_Title.SetWindowText(Title);
}

void CMidiSetupRow::GetInfo(CMidiInfo& Info) const
{
	Info.m_Range = float(m_Range.GetVal());
	Info.SetMsg(m_Event.GetCurSel(), m_Chan.GetIntVal() - 1, m_Ctrl.GetIntVal());
}

void CMidiSetupRow::SetInfo(const CMidiInfo& Info)
{
	m_Range.SetVal(Info.m_Range);
	Assign(Info.m_Event, Info.m_Chan, Info.m_Ctrl);
}

void CMidiSetupRow::SetValue(int Val)
{
	m_Value.SetVal(Val);
}

void CMidiSetupRow::Assign(int Event, int Chan, int Ctrl)
{
	m_Event.SetCurSel(Event);
	m_Chan.SetVal(Chan + 1);
	m_Ctrl.SetNoteEntry(Event == MET_NOTE);
	m_Ctrl.SetVal(Ctrl);
}

void CMidiSetupRow::SetEvent(int Event)
{
	m_Event.SetCurSel(Event);
}

void CMidiSetupRow::SetSelected(bool Enable)
{
	m_Selected = Enable;
	Invalidate();
	m_Title.Invalidate();	// necessary because we're clipping children
}

void CMidiSetupRow::DoDataExchange(CDataExchange* pDX)
{
	CRowDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiSetupRow)
	DDX_Control(pDX, IDC_MS_RANGE_SPIN, m_RangeSpin);
	DDX_Control(pDX, IDC_MS_CTRL_SPIN, m_CtrlSpin);
	DDX_Control(pDX, IDC_MS_CHAN_SPIN, m_ChanSpin);
	DDX_Control(pDX, IDC_MS_EVENT, m_Event);
	DDX_Control(pDX, IDC_MS_VALUE, m_Value);
	DDX_Control(pDX, IDC_MS_CHAN, m_Chan);
	DDX_Control(pDX, IDC_MS_CTRL, m_Ctrl);
	DDX_Control(pDX, IDC_MS_TITLE, m_Title);
	DDX_Control(pDX, IDC_MS_RANGE, m_Range);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMidiSetupRow, CRowDlg)
	//{{AFX_MSG_MAP(CMidiSetupRow)
	ON_CBN_SELCHANGE(IDC_MS_EVENT, OnSelchangeEvent)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_PARENTNOTIFY()
	ON_NOTIFY(NEN_CHANGED, IDC_MS_RANGE, OnChangedNumEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_MS_CHAN, OnChangedNumEdit)
	ON_NOTIFY(NEN_CHANGED, IDC_MS_CTRL, OnChangedNumEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupRow message handlers

BOOL CMidiSetupRow::OnInitDialog() 
{
	CRowDlg::OnInitDialog();

	m_RangeSpin.SetDelta(.01);
	m_Event.SetCurSel(0);
	m_Chan.SetRange(1, 16);
	m_Chan.SetFormat(CNumEdit::DF_INT);
	m_Ctrl.SetRange(0, 127);
	m_Ctrl.SetFormat(CNumEdit::DF_INT);

	CWnd	*wp = GetWindow(GW_CHILD);
	while (wp != NULL) {	// enable parent notify for all our controls
		wp->ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0);
		wp = wp->GetNextWindow();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMidiSetupRow::OnChangedNumEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	CWnd	*NotifyWnd = GetNotifyWnd();
	if (NotifyWnd != NULL)
		NotifyWnd->SendMessage(UWM_MIDIROWEDIT, m_RowIdx, pNMHDR->idFrom);
	*pResult = 0;
}

void CMidiSetupRow::OnSelchangeEvent() 
{
	CWnd	*NotifyWnd = GetNotifyWnd();
	if (NotifyWnd != NULL)
		NotifyWnd->SendMessage(UWM_MIDIROWEDIT, m_RowIdx, IDC_MS_EVENT);
}

HBRUSH CMidiSetupRow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CRowDlg::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_Selected && (pWnd == this || pWnd == &m_Title)) {
		if (pWnd == &m_Title)
			pDC->SetBkColor(m_SelColor);	// set title's background color
		hbr	= m_SelBrush;	// set row dialog's background color
	}
	return hbr;
}

void CMidiSetupRow::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd	*NotifyWnd = GetNotifyWnd();
	if (NotifyWnd != NULL)
		NotifyWnd->SendMessage(UWM_MIDIROWSEL, m_RowIdx);
	CWnd	*wp = GetFocus();
	// if another row has focus, give focus to the same column in our row
	if (wp != NULL && !IsChild(wp)) {	// if focus window isn't one of our controls
		int	id = wp->GetDlgCtrlID();	// get focus window's control ID
		wp = GetDlgItem(id);			// find one of our controls with that ID
	}
	if (wp == NULL)		
		wp = &m_Event;	// default focus to Event column
	wp->SetFocus();
	CEdit	*ep = DYNAMIC_DOWNCAST(CEdit, wp);
	if (ep != NULL)		// if an edit control has focus
		ep->SetSel(0, -1);	// select entire text
	CRowDlg::OnLButtonDown(nFlags, point);
}

void CMidiSetupRow::OnParentNotify(UINT message, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN) {
		CWnd	*NotifyWnd = GetNotifyWnd();
		if (NotifyWnd != NULL)
			NotifyWnd->SendMessage(UWM_MIDIROWSEL, m_RowIdx);
	}
	CRowDlg::OnParentNotify(message, lParam);
}
