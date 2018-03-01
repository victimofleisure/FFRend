// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03nov06	initial version
		01		05nov06	restart timer in OnShowWindow
		02		24nov06	use SetCurSelUndoable
		03		26nov06	fix plugin page's non-trailing empty slots crash
        04      29dec06	remove OnShowWindow, set visible in dialog resource
		05		19jan07	convert from dialog to control bar
		06		21jan07	replace AfxGetMainWnd with GetThis
		07		31jan07	move FastIsVisible into base class
		08		11jul07	in UpdateView, remove set focus
		09		23nov07	support Unicode
		10		22dec07	in UpdateView, invalidate row selection
		11		22dec07	add ParmToRow so plugin page can handle empty slots
		12		23dec07	add metaparameter page
		13		27apr10	refactor for engine
		14		24nov11	remember each plugin's scroll position
		15		13jan12	use NotifyEdit wrapper
		16		09apr12	in SetScrollPos, fix double negation of special slot

		MIDI setup dialog
 
*/

// MidiSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MidiSetupDlg.h"
#include "MidiIO.h"
#include "FFPluginEx.h"
#include "UndoCodes.h"
#include "Undoable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg dialog

IMPLEMENT_DYNCREATE(CMidiSetupView, CRowView);
IMPLEMENT_DYNAMIC(CMidiSetupDlg, CDialog);

const CRowView::COLINFO CMidiSetupDlg::m_ColInfo[COLS] = {
	{IDC_MS_TITLE,	IDS_FF_PARM_NAME},
	{IDC_MS_RANGE,	IDS_MS_RANGE},
	{IDC_MS_EVENT,	IDS_MS_EVENT},
	{IDC_MS_CHAN,	IDS_MS_CHAN},
	{IDC_MS_CTRL,	IDS_MS_CTRL},
	{IDC_MS_VALUE,	IDS_MS_VALUE}
};

const CRect	CMidiSetupDlg::m_PropRect(CPoint(PROP_COMBO_X, PROP_COMBO_Y), 
	CSize(PROP_COMBO_WIDTH, PROP_COMBO_HEIGHT));
const CRect	CMidiSetupDlg::m_LearnRect(CPoint(LEARN_CHK_X, LEARN_CHK_Y), 
	CSize(LEARN_CHK_WIDTH, LEARN_CHK_HEIGHT));

CMidiSetupDlg::CMidiSetupDlg(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CMidiSetupDlg)
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_Engine = NULL;
	m_PageType = MPT_MISC;	// misc page's rows are guaranteed to exist
	m_PlugSel = SPI_MISC;	// misc page's special plugin index
	m_RowSel = -1;			// no row selection
	m_PropSel = 0;
	ZeroMemory(m_PropSelList, sizeof(m_PropSelList));
	m_PluginCount = 0;
	m_ShowRowSel = FALSE;
	m_Learn = FALSE;
	m_InitTabCtrlY = 0;
	ZeroMemory(m_ScrollPos, sizeof(m_ScrollPos));
}

void CMidiSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMidiSetupDlg)
	DDX_Control(pDX, IDC_MS_LEARN, m_LearnChk);
	DDX_Control(pDX, IDC_MS_PROP, m_PropCombo);
	DDX_Control(pDX, IDC_MS_TAB, m_TabCtrl);
	//}}AFX_DATA_MAP
}

void CMidiSetupDlg::SelectRow(int RowIdx)
{
	if (RowIdx != m_RowSel) {
		if (m_ShowRowSel) {
			if (m_RowSel >= 0)
				GetRow(m_RowSel)->SetSelected(FALSE);	// remove previous selection
			if (RowIdx >= 0)
				GetRow(RowIdx)->SetSelected(TRUE);
		}
		m_RowSel = RowIdx;
	}
}

void CMidiSetupDlg::ShowRowSel(bool Enable)
{
	if (Enable != m_ShowRowSel) {
		if (Enable) {
			m_ShowRowSel = Enable;	// order matters
			int	row = m_RowSel;	// save selection
			m_RowSel = -1;	// force SelectRow to update
			SelectRow(row);	// show selection
		} else {
			SelectRow(-1);	// hide selection
			m_ShowRowSel = Enable;	// order matters
		}
	}
}

int CMidiSetupDlg::GetRowCount(int SlotIdx) const
{
	if (SlotIdx == SPI_PLUGIN)	// if plugin page
		return(m_PluginCount);	// use our adjusted plugin count
	return(m_Engine->GetMidiParmCount(SlotIdx));	// otherwise defer to main
}

inline int CMidiSetupDlg::RowToParm(int SlotIdx, int RowIdx) const
{
	return(SlotIdx == SPI_PLUGIN ? m_TabPlug[RowIdx] : RowIdx);
}

inline int CMidiSetupDlg::ParmToRow(int SlotIdx, int ParmIdx) const
{
	return(SlotIdx == SPI_PLUGIN ? m_Engine->SlotToPlugin(ParmIdx) : ParmIdx);
}

inline int CMidiSetupDlg::GetPageType(int SlotIdx) const
{
	return(SlotIdx >= 0 ? MPT_PARAM : -SlotIdx);
}

inline int CMidiSetupDlg::GetTabIdx(int PageType) const
{
	return(PageType == MPT_PARAM ? 0 : m_PluginCount - 1 + PageType);
}

CPoint CMidiSetupDlg::GetScrollPos(int SlotIdx) const
{
	if (SlotIdx >= 0) {	// if normal slot index
		CFFPluginEx	*plug = m_Engine->GetSlot(SlotIdx);
		if (plug != NULL)
			return(plug->GetScrollPos(CFFPluginEx::SPT_MIDI_SETUP));
		return(CPoint(0, 0));
	} else {	// special slot index
		SlotIdx = -SlotIdx;
		ASSERT(SlotIdx >= 0 && SlotIdx < MIDI_PAGE_TYPES);
		return(m_ScrollPos[SlotIdx]);
	}
}

void CMidiSetupDlg::SetScrollPos(int SlotIdx, CPoint ScrollPos)
{
	if (SlotIdx >= 0) {	// if normal slot index
		CFFPluginEx	*plug = m_Engine->GetSlot(SlotIdx);
		if (plug != NULL)
			plug->SetScrollPos(CFFPluginEx::SPT_MIDI_SETUP, ScrollPos);
	} else {	// special slot index
		SlotIdx = -SlotIdx;
		ASSERT(SlotIdx >= 0 && SlotIdx < MIDI_PAGE_TYPES);
		m_ScrollPos[SlotIdx] = ScrollPos;
	}
}

void CMidiSetupDlg::UpdateMidiInfo()
{
	int	rows = GetRowCount(m_PlugSel);
	CMidiInfo	Info;
	for (int i = 0; i < rows; i++) {
		int	ParmSel = RowToParm(m_PlugSel, i);	// map dialog row to MIDI param index
		m_Engine->GetMidiInfo(m_PlugSel, ParmSel, m_PropSel, Info);
		CMidiSetupRow	*rp = GetRow(i);
		rp->SetInfo(Info);
		rp->SetValue(0);	// reset MIDI value
	}
}

void CMidiSetupDlg::SetRowInfo(int RowIdx, CMidiSetupRow& Row)
{
	int	ParmSel = RowToParm(m_PlugSel, RowIdx);	// map dialog row to MIDI param index
	CString	s;
	m_Engine->GetMidiParmName(m_PlugSel, ParmSel, s);
	if (s.GetLength())
		s += _T(":");
	Row.SetCaption(s);
	CMidiInfo	Info;
	m_Engine->GetMidiInfo(m_PlugSel, ParmSel, m_PropSel, Info);
	Row.SetInfo(Info);
	Row.SetSelected(FALSE);
}

void CMidiSetupDlg::PopulatePropCombo(int PageType)
{
	m_PropCombo.ResetContent();
	int	props = m_Engine->GetMidiPropCount(PageType);
	CString	s;
	for (int i = 0; i < props; i++) {
		m_Engine->GetMidiPropName(PageType, i, s);
		m_PropCombo.AddString(s);
	}
	m_PropCombo.EnableWindow(m_PropCombo.GetCount());
}

void CMidiSetupDlg::SelectTab(int TabIdx)
{
	ASSERT(TabIdx >= 0 && TabIdx < m_TabCtrl.GetItemCount());
	m_TabCtrl.SetCurSel(TabIdx);
	SetScrollPos(m_PlugSel, m_View->GetScrollPos());
	m_PlugSel = m_TabPlug[TabIdx];
	int	PageType = GetPageType(m_PlugSel);
	if (PageType != m_PageType) {	// if selection changed
		PopulatePropCombo(PageType);
		m_PropSel = m_PropSelList[PageType];
		m_PropCombo.SetCurSel(m_PropSel);
		m_PageType = PageType;
	}
	if (m_PlugSel >= 0) 	// if tab corresponds to a plugin parameter
		m_Engine->SetCurSel(m_PlugSel);
	m_View->CreateRows(GetRowCount(m_PlugSel), GetScrollPos(m_PlugSel));
	m_RowSel = -1;	// invalidate row selection
}

void CMidiSetupDlg::Assign(int RowIdx, const CMidiInfo& Info)
{
	int	PrevPlug, PrevParm, PrevProp;
	int	ParmSel = RowToParm(m_PlugSel, RowIdx);	// map dialog row to param index
	// if MIDI message is already mapped to a different row on this page
	if (m_Engine->GetMidiMapping(Info, PrevPlug, PrevParm, PrevProp)
	&& PrevParm != ParmSel && PrevPlug == m_PlugSel && PrevProp == m_PropSel) {
		int	PrevRow = ParmToRow(PrevPlug, PrevParm);
		GetRow(PrevRow)->SetEvent(MET_OFF);	// reset previous mapping's event type
	}
	theApp.GetMain()->NotifyEdit(0, UCODE_MIDI_SETUP, CUndoable::UE_COALESCE);
	m_Engine->AssignMidi(m_PlugSel, ParmSel, m_PropSel, Info);	// create new mapping
	GetRow(RowIdx)->Assign(Info.m_Event, Info.m_Chan, Info.m_Ctrl);
}

void CMidiSetupDlg::UpdateView()
{
	int	TabIdx;
	int	SelTab = -1;
	m_TabPlug.RemoveAll();
	m_TabCtrl.DeleteAllItems();
	// add tabs for all loaded plugins
	int	plugs = m_Engine->GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plug = m_Engine->GetPlugin(PlugIdx);
		int	SlotIdx = plug.GetSlotIdx();
		TabIdx = m_TabPlug.Add(SlotIdx);
		m_TabCtrl.InsertItem(TabIdx, plug.GetName());
		if (SlotIdx == m_Engine->GetCurSel())
			SelTab = TabIdx;
	}
	m_PluginCount = plugs;
	// add tab for plugin rows
	TabIdx = m_TabPlug.Add(SPI_PLUGIN);
	m_TabCtrl.InsertItem(TabIdx, LDS(IDS_MS_TAB_PLUGIN));
	// add tab for miscellaneous rows
	TabIdx = m_TabPlug.Add(SPI_MISC);
	m_TabCtrl.InsertItem(TabIdx, LDS(IDS_MS_TAB_MISC));
	// add tab for metaparameter rows
	TabIdx = m_TabPlug.Add(SPI_METAPARM);
	m_TabCtrl.InsertItem(TabIdx, LDS(IDS_MS_TAB_METAPARM));
	// fix up tab selection if necessary
	if (m_PageType == MPT_PARAM) {	// if parameter page
		if (SelTab < 0) {				// but no selected plugin was found
			m_PageType = MPT_PLUGIN;			// select plugin page instead
			SelTab = GetTabIdx(m_PageType);
		}
	} else
		SelTab = GetTabIdx(m_PageType);
	m_PlugSel = m_TabPlug[SelTab];
	m_TabCtrl.SetCurSel(SelTab);
	PopulatePropCombo(m_PageType);
	m_PropSel = m_PropSelList[m_PageType];
	m_PropCombo.SetCurSel(m_PropSel);
	m_View->CreateRows(GetRowCount(m_PlugSel), GetScrollPos(m_PlugSel));
	m_RowSel = -1;	// invalidate row selection
}

void CMidiSetupDlg::SetLearn(bool Enable)
{
	m_Learn = Enable;
	m_LearnChk.SetCheck(Enable);
	ShowRowSel(Enable);
}

void CMidiSetupDlg::SetDefaults()
{
	m_PageType = MPT_PARAM;	// so our selection will match project's if possible
	m_PlugSel = 0;
	m_RowSel = -1;	// no row selection
	m_PropSel = 0;
	ZeroMemory(m_PropSelList, sizeof(m_PropSelList));
	m_PluginCount = 0;
	m_ShowRowSel = FALSE;
	SetLearn(FALSE);
}

CRowDlg *CMidiSetupView::CreateRow(int Idx)
{
	CMidiSetupRow	*rp = new CMidiSetupRow;
	rp->Create(IDD_MIDI_SETUP_ROW);
	m_Parent->SetRowInfo(Idx, *rp);
	return(rp);
}

void CMidiSetupView::UpdateRow(int Idx)
{
	CMidiSetupRow	*rp = (CMidiSetupRow *)GetRow(Idx);
	m_Parent->SetRowInfo(Idx, *rp);
	rp->Invalidate();
}

BEGIN_MESSAGE_MAP(CMidiSetupDlg, CDialog)
	//{{AFX_MSG_MAP(CMidiSetupDlg)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MS_TAB, OnSelchangeTab)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_MS_LEARN, OnLearn)
	ON_CBN_SELCHANGE(IDC_MS_PROP, OnSelchangeProp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_MIDIROWEDIT, OnMidiRowEdit)
	ON_MESSAGE(UWM_MIDIROWSEL, OnMidiRowSel)
	ON_MESSAGE(UWM_MIDIIN, OnMidiIn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg message handlers

BOOL CMidiSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Engine = &theApp.GetEngine();
	// create row view
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CMidiSetupView);
	m_View = DYNAMIC_DOWNCAST(CMidiSetupView, pFactory->CreateObject());
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER;
	CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_MIDI_SETUP_VIEW, NULL))
		return -1;
	m_View->m_Parent = this;
	m_View->SetNotifyWnd(this);
	m_View->SetAccel(NULL, theApp.GetMain());
	m_View->CreateCols(COLS, m_ColInfo, IDD_MIDI_SETUP_ROW);
	m_TabCtrl.GetWindowRect(r);
	ScreenToClient(r);
	m_InitTabCtrlY = r.top;

	return TRUE;	// return TRUE unless you set the focus to a control
}

void CMidiSetupDlg::OnOK()
{
}

void CMidiSetupDlg::OnCancel()
{
}

void CMidiSetupDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	TabIdx = m_TabCtrl.GetCurSel();
	if (TabIdx >= 0) {
		m_TabCtrl.SetFocus();	// force row edit notification
		SelectTab(TabIdx);
	}
	*pResult = 0;
}

void CMidiSetupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if (m_TabCtrl.m_hWnd) {
		int	yofs = m_InitTabCtrlY + TAB_CTRL_HEIGHT;
		m_View->MoveWindow(0, yofs, cx, cy - yofs);
		// tuck tab control's bottom edge under row view's column header
		// to reduce flicker, resize tab control after view and use defer erase
		m_TabCtrl.SetWindowPos(m_View, 0, m_InitTabCtrlY, 
			cx, TAB_CTRL_HEIGHT + TAB_CTRL_OVERLAP, SWP_DEFERERASE);
	}
}

void CMidiSetupDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	// suppress base class size-limiting behavior
}

LRESULT CMidiSetupDlg::OnMidiRowEdit(WPARAM wParam, LPARAM lParam)
{
	CMidiInfo	Info;
	GetRow(wParam)->GetInfo(Info);
	Assign(wParam, Info);
	return TRUE;
}

LRESULT CMidiSetupDlg::OnMidiRowSel(WPARAM wParam, LPARAM lParam)
{
	SelectRow(wParam);
	return TRUE;
}

LRESULT CMidiSetupDlg::OnMidiIn(WPARAM wParam, LPARAM lParam)
{
	MIDI_MSG	msg;
	msg.dw = wParam;
	int	event = CMidiInfo::CmdToEvent[(msg.s.cmd >> 4)];
	if (event >= 0) {
		CMidiInfo	Info(0, event, msg.s.cmd & 0x0f, msg.s.p1);
		if (m_Learn) {
			if (m_RowSel >= 0) {
				Info.m_Range = GetRow(m_RowSel)->GetRange();
				Assign(m_RowSel, Info);
			}
		}
		// if MIDI message is mapped to a row on this page
		int	SlotIdx, ParmIdx, PropIdx;
		if (m_Engine->GetMidiMapping(Info, SlotIdx, ParmIdx, PropIdx)
		&& SlotIdx == m_PlugSel && PropIdx == m_PropSel) {
			int	RowIdx = ParmToRow(SlotIdx, ParmIdx);
			GetRow(RowIdx)->SetValue(msg.s.p2);
		}
	}
	return TRUE;
}

void CMidiSetupDlg::OnLearn() 
{
	m_Learn ^= 1;
	ShowRowSel(m_Learn);
}

void CMidiSetupDlg::OnSelchangeProp() 
{
	int	PropIdx = m_PropCombo.GetCurSel();
	if (PropIdx >= 0) {
		m_PropSel = PropIdx;
		m_PropSelList[m_PageType] = PropIdx;
		UpdateMidiInfo();
	}
}

BOOL CMidiSetupDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		// if non-system key down and main accelerators, give main a try
		if (pMsg->message == WM_KEYDOWN
		&& AfxGetMainWnd()->SendMessage(UWM_HANDLEDLGKEY, (WPARAM)pMsg))
			return(TRUE);
	}
	return CDialog::PreTranslateMessage(pMsg);
}
