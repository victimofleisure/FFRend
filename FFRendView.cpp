// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		01may11	add load balance
        02      07may11	add sync oscillators
		03		30may11	in OnEditLoadBalance, add undo
		04		11nov11	fix keyboard-triggered context menu
        05      18nov11	convert load balance from dialog to bar
		06		24nov11	remember each plugin's scroll position
		07		01dec11	disable input popups for source plugins
		08		01dec11	in OnContextMenu, restrict use of tab control hit test
		09		11jan12	in SyncOscillators, if paused, update UI to show sync
		10		26mar12	add monitor source
		11		06apr12	in OnCreate, create row view with non-zero ID
		12		14may12	in SetMonitorSource, ignore unchanged source
		13		17may12	in UpdateView, fix scrolled tab control bug
		14		23may12	make monitor source a slot index

        FFRend view
 
*/

// FFRendView.cpp : implementation of the CFFRendView class
//

#include "stdafx.h"
#include "FFRend.h"

#include "FFRendDoc.h"
#include "FFRendView.h"

#include "FFPluginEx.h"
#include "FFPlugsRow.h"
#include "FocusEdit.h"
#include "UndoCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFFRendView

IMPLEMENT_DYNCREATE(CFFRowView, CRowView)
IMPLEMENT_DYNCREATE(CFFRendView, CView)

const CRowView::COLINFO CFFRendView::m_ColInfo[COLUMNS] = {
	{IDC_FF_PARM_NAME,		IDS_FF_PARM_NAME},
	{IDC_FF_PARM_SLIDER,	IDS_FF_PARM_SLIDER},
	{IDC_FF_PARM_EDIT,		IDS_FF_PARM_EDIT},
	{IDC_FF_MOD_ENAB,		IDS_FF_MOD_ENAB},
	{IDC_FF_MOD_WAVE,		IDS_FF_MOD_WAVE},
	{IDC_FF_MOD_FREQ,		IDS_FF_MOD_FREQ},
	{IDC_FF_MOD_PW,			IDS_FF_MOD_PW},
};

#undef UCODE_DEF
#define UCODE_DEF(name) IDS_UC_##name, 
const int CFFRendView::m_UndoTitleID[UNDO_CODES] = {
#include "UndoCodeData.h"
};

/////////////////////////////////////////////////////////////////////////////
// CFFRendView construction/destruction

CFFRendView::CFFRendView() :
	m_RecentPlugin(0, _T("Recent Plugin"), _T("Plugin%d"), MAX_RECENT_PLUGINS),
	m_Clipboard(NULL, NULL)	// clipboard format is set below
{
	m_Main = NULL;
	m_Engine = NULL;
	m_View = NULL;
	m_InContextMenu = FALSE;
	m_ContextTarget = CTX_NO_TARGET;
	m_RecentPlugin.ReadList();
	CString	CBFmt;	// clipboard format includes struct version number
	CBFmt.Format(_T("FFPLUG_INFO%d"), CFFPlugInfo::INFO_VERSION);
	m_Clipboard.SetFormat(CBFmt);
	m_PrevSel = 0;
	m_MoveSrcSlotIdx = 0;
	m_MoveDstSlotIdx = 0;
	m_ConnDstInpIdx = 0;
	m_TabCtrlImgList.Create(14, 12, ILC_COLOR | ILC_MASK, 1, 1);
	m_TabCtrlImgList.Add(LoadIcon(AfxGetApp()->m_hInstance, 
		MAKEINTRESOURCE(IDI_MONITOR)));
	m_MonSrcSlotIdx = -1;
}

CFFRendView::~CFFRendView()
{
	m_RecentPlugin.WriteList();
}

BOOL CFFRendView::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	return CView::PreCreateWindow(cs);
}

bool CFFRendView::Insert(int SlotIdx, LPCTSTR Path)
{
	m_PrevSel = m_Engine->GetCurSel();
	if (!m_Engine->Insert(SlotIdx, Path))
		return(FALSE);
	NotifyUndoableEdit(SlotIdx, UCODE_INSERT);
	m_RecentPlugin.Add(Path);
	return(TRUE);
}

bool CFFRendView::InsertEmpty(int SlotIdx)
{
	m_PrevSel = m_Engine->GetCurSel();
	if (!m_Engine->InsertEmpty(SlotIdx))
		return(FALSE);
	NotifyUndoableEdit(SlotIdx, UCODE_INSERT_EMPTY);
	return(TRUE);
}

bool CFFRendView::Delete(int SlotIdx)
{
	NotifyUndoableEdit(SlotIdx, UCODE_DELETE);
	if (!m_Engine->Delete(SlotIdx)) {
		CancelUndoableEdit(SlotIdx, UCODE_DELETE);
		return(FALSE);
	}
	return(TRUE);
}

bool CFFRendView::Load(int SlotIdx, LPCTSTR Path)
{
	NotifyUndoableEdit(SlotIdx, UCODE_LOAD);
	if (!m_Engine->Load(SlotIdx, Path)) {
		CancelUndoableEdit(SlotIdx, UCODE_LOAD);
		return(FALSE);
	}
	m_RecentPlugin.Add(Path);
	return(TRUE);
}

bool CFFRendView::LoadRecent(int MruIdx)
{
	ASSERT(MruIdx >= 0 && MruIdx < MAX_RECENT_PLUGINS);
	LPCTSTR	Path = m_RecentPlugin[MruIdx];
	bool	retc;
	int	SlotIdx = GetCurSel();
	if (SlotIdx >= 0)
		retc = Load(SlotIdx, Path);
	else
		retc = Insert(m_Engine->GetSlotCount(), Path);
	if (!retc)
		m_RecentPlugin.Remove(MruIdx);
	return(retc);
}

bool CFFRendView::Unload(int SlotIdx)
{
	NotifyUndoableEdit(SlotIdx, UCODE_UNLOAD);
	if (!m_Engine->Unload(SlotIdx)) {
		CancelUndoableEdit(SlotIdx, UCODE_UNLOAD);
		return(FALSE);
	}
	return(TRUE);
}

void CFFRendView::Bypass(int SlotIdx, bool Enable)
{
	NotifyUndoableEdit(SlotIdx, UCODE_BYPASS);
	m_Engine->Bypass(SlotIdx, Enable);
}

void CFFRendView::Solo(int SlotIdx)
{
	NotifyUndoableEdit(SlotIdx, UCODE_SOLO);
	if (m_Engine->InSolo())
		m_Engine->EndSolo();
	else {
		if (IsCurSelLoaded())
			m_Engine->Solo(SlotIdx);
	}
}

void CFFRendView::Copy(int SlotIdx)
{
	FFPLUG_INFO	*pInfo;
	DWORD	sz;
	if (m_Engine->IsLoaded(SlotIdx))
		 sz = m_Engine->GetSlot(SlotIdx)->GetInfo(pInfo);
	else {
		pInfo = new FFPLUG_INFO;
		sz = sizeof(FFPLUG_INFO);
		ZeroMemory(pInfo, sz);	// empty slot
	}
	m_Clipboard.Write(pInfo, sz);
	delete [] pInfo;	// clean up info
}

bool CFFRendView::Cut(int SlotIdx)
{
	Copy(SlotIdx);
	NotifyUndoableEdit(SlotIdx, UCODE_CUT);
	if (!m_Engine->Delete(SlotIdx)) {
		CancelUndoableEdit(SlotIdx, UCODE_CUT);
		return(FALSE);
	}
	return(TRUE);
}

bool CFFRendView::Paste(int SlotIdx)
{
	if (!m_Clipboard.HasData())
		return(FALSE);
	m_PrevSel = m_Engine->GetCurSel();
	DWORD	sz;
	FFPLUG_INFO	*pInfo = (FFPLUG_INFO *)m_Clipboard.Read(sz);
	CFFPlugInfo	info(*pInfo);	// convert from struct to object
	bool	retc;
	if (pInfo->Path[0])
		retc = m_Engine->Insert(SlotIdx, info);
	else	// empty slot
		retc = m_Engine->InsertEmpty(SlotIdx);
	delete [] pInfo;	// clean up info allocated by clipboard read
	if (retc)
		NotifyUndoableEdit(SlotIdx, UCODE_PASTE);
	return(retc);
}

bool CFFRendView::Move(int SrcSlotIdx, int DstSlotIdx)
{
	if (SrcSlotIdx == DstSlotIdx)
		return(TRUE);	// nothing to do
	// stash source and destination slots for SaveUndoState
	m_MoveSrcSlotIdx = SrcSlotIdx;
	m_MoveDstSlotIdx = DstSlotIdx;
	NotifyUndoableEdit(GetCurSel(), UCODE_MOVE);
	if (!m_Engine->Move(SrcSlotIdx, DstSlotIdx))
		return(FALSE);
	return(TRUE);
}

bool CFFRendView::Connect(int SrcPlugIdx, int DstSlotIdx, int DstInpIdx)
{
	// convert source from plugin index to slot index
	int	SrcSlotIdx = SrcPlugIdx >= 0 ?
		m_Engine->GetPlugin(SrcPlugIdx).GetSlotIdx() : -1;
	if (m_Engine->IsConnected(SrcSlotIdx, DstSlotIdx, DstInpIdx))
		return(TRUE);	// connection already exists
	// stash input index for SaveUndoState
	m_ConnDstInpIdx = DstInpIdx;
	NotifyUndoableEdit(DstSlotIdx, UCODE_CONNECT);
	if (!m_Engine->Connect(SrcSlotIdx, DstSlotIdx, DstInpIdx)) {
		CancelUndoableEdit(DstSlotIdx, UCODE_CONNECT);
		return(FALSE);
	}
	return(TRUE);
}

void CFFRendView::SetRowInfo(int RowIdx, CFFPlugsRow& Row)
{
	CFFPluginEx	*plug = m_Engine->GetCurPlugin();
	if (plug != NULL) {
		Row.SetName(plug->GetParmName(RowIdx));
		FFPARM_INFO	info;
		plug->GetParmInfo(RowIdx, info);
		Row.SetInfo(info);
	}
}

CRowDlg *CFFRowView::CreateRow(int Idx)
{
	CFFPlugsRow	*rp = new CFFPlugsRow;
	rp->Create(IDD_FF_PLUGS_ROW);
	m_Parent->SetRowInfo(Idx, *rp);
	return(rp);
}

void CFFRowView::UpdateRow(int Idx)
{
	CFFPlugsRow	*rp = (CFFPlugsRow *)GetRow(Idx);
	m_Parent->SetRowInfo(Idx, *rp);
}

void CFFRendView::UpdateRows()
{
	CFFPluginEx	*plug = m_Engine->GetCurPlugin();
	if (plug != NULL) {
		m_View->CreateRows(plug->GetParmCount(), 
			plug->GetScrollPos(CFFPluginEx::SPT_PARAMETER));
	} else
		m_View->CreateRows(0);
}

void CFFRendView::UpdateView()
{
	// update tab control
	OnMonitorSourceChange(-1);	// remove monitor source icon if any
	// if tab control is scrolled, removing tabs can cause scroll position to
	// exceed tab count, in which case tab control paints incorrectly, showing
	// missing tabs; workaround is reset scroll position before removing tabs
	m_TabCtrl.SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 0));
	int	tabs = m_TabCtrl.GetItemCount();
	int	slots = m_Engine->GetSlotCount();
	for (int SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
		CFFPluginEx	*slot = m_Engine->GetSlot(SlotIdx);
		LPCTSTR	name;
		bool	bypass;
		if (slot != NULL) {	// if slot is loaded
			name = slot->GetName();
			bypass = slot->GetBypass();
		} else {	// empty slot
			name = _T("");
			bypass = FALSE;
		}
		if (SlotIdx < tabs)	// if enough tabs
			m_TabCtrl.SetText(SlotIdx, name);	// update
		else	// not enough tabs
			m_TabCtrl.InsertItem(SlotIdx, name);	// append
		m_TabCtrl.SetHighlight(SlotIdx, bypass);
	}
	for (int TabIdx = slots; TabIdx < tabs; TabIdx++)	// remove extra tabs
		m_TabCtrl.DeleteItem(slots);
	m_TabCtrl.SetCurSel(GetCurSel());	// set current selection
	OnMonitorSourceChange(m_Main->GetMonitorSource());
	UpdateRows();	// update parameter rows
}

void CFFRendView::OnBypass(int SlotIdx, bool Enable)
{
	m_TabCtrl.SetHighlight(SlotIdx, Enable);
}

void CFFRendView::OnSelChange(int PrevSlotIdx, int SlotIdx)
{
	m_TabCtrl.SetCurSel(SlotIdx);	// update tab control
	if (PrevSlotIdx >= 0) {	// if previous slot index is valid
		CFFPluginEx	*plug = m_Engine->GetSlot(PrevSlotIdx);
		if (plug != NULL)	// if previous slot is loaded, save its scroll position
			plug->SetScrollPos(CFFPluginEx::SPT_PARAMETER, m_View->GetScrollPos());
	}
	UpdateRows();
}

void CFFRendView::ShowPluginContextMenu(CPoint point, int TargetSlot)
{
	m_ContextTarget = TargetSlot;
	CMenu	menu;
	menu.LoadMenu(IDR_VIEW_CTX);
	CMenu	*mp = menu.GetSubMenu(0);
	// update menu items to reflect target slot, not current selection
	m_InContextMenu = TRUE;	// GetCurSel will return m_ContextTarget
	theApp.UpdateMenu(this, &menu);
	m_InContextMenu = FALSE;	// restore default behavior
	mp->TrackPopupMenu(0, point.x, point.y, this);
}

int CFFRendView::FindSlot(CPoint pt) const
{
	int	SlotIdx = GetCurSel();	// default to current slot
	CRect	r;
	m_TabCtrl.GetWindowRect(r);	// assume pt is in screen coords
	if (r.PtInRect(pt)) {		// if pt is within tab control
		m_TabCtrl.ScreenToClient(&pt);	// convert to tab coords
		TCHITTESTINFO	hti;
		hti.pt = pt;
		SlotIdx = m_TabCtrl.HitTest(&hti);	// tab index, or -1 if not on a tab
	}
	return(SlotIdx);
}

int CFFRendView::GetSelectedRow() const
{
	int	rows = GetRows();
	CWnd	*wp = GetFocus();
	for (int i = 0; i < rows; i++) {
		if (GetRow(i)->IsChild(wp))
			return(i);
	}
	return(-1);
}

void CFFRendView::AddMetaparm(int PageType, int PropIdx)
{
	int	SlotIdx = GetCurSel();
	int	ParmIdx = GetSelectedRow();
	if (SlotIdx >= 0 && ParmIdx >= 0)
		m_Main->AddMetaparm(PageType, SlotIdx, ParmIdx, PropIdx);
}

bool CFFRendView::MakePluginPopup(CMenu& Popup, int BaseID, int CheckIdx) const
{
	if (!Popup.CreatePopupMenu())
		return(FALSE);
	int	Flags = CheckIdx < 0 ? (MF_STRING | MF_CHECKED) : MF_STRING;
	Popup.AppendMenu(Flags, BaseID, LDS(IDS_FF_INPUT_DEFAULT));
	CString	s;
	int	plugs = m_Engine->GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		const CFFPluginEx&	plug = m_Engine->GetPlugin(PlugIdx);
		s = plug.GetName();
		Flags = CheckIdx == PlugIdx ? (MF_STRING | MF_CHECKED) : MF_STRING;
		Popup.AppendMenu(Flags, BaseID + PlugIdx + 1, s);
	}
	return(TRUE);
}

int CFFRendView::InsertInputPopups(CMenu *Menu, int Pos, int SlotIdx) const
{
	ASSERT(Pos >= 0);
	int	InsertCount = 0;
	CFFPluginEx	*plug;
	// if slot index is valid, and slot is loaded and isn't a source plugin
	if (SlotIdx >= 0 && (plug = m_Engine->GetSlot(SlotIdx)) != NULL
	&& !plug->IsSource()) {
		int	NumInputs = plug->GetNumInputs();
		int	sources = m_Engine->GetPluginCount() + 1;	// plus one for default
		for (int InpIdx = 0; InpIdx < NumInputs; InpIdx++) {
			CMenu	Popup;
			int	BaseID = ID_PLUGIN_INPUT_FIRST + sources * InpIdx;
			int	CheckIdx = plug->GetInputPlugin(InpIdx);
			MakePluginPopup(Popup, BaseID, CheckIdx);
			CString	PopupName((LPCTSTR)ID_PLUGIN_INPUT);
			if (NumInputs > 1) {	// if multiple inputs
				TCHAR	c = static_cast<TCHAR>('A' + InpIdx);
				PopupName += " " + CString(c);	// label inputs A, B, etc.
			}
			HMENU	hPopup = Popup.Detach();	// detach popup so menu can own it
			// if insert succeeds, menu owns the popup and will dispose of it
			BOOL	retc = Menu->InsertMenu(Pos + InpIdx, 
				MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hPopup, PopupName);
			if (retc)
				InsertCount++;
			else	// insert failed, so we still own the detached popup
				DestroyMenu(hPopup);	// we're responsible for disposal
		}
	} else {	// disable input item
		BOOL	retc = Menu->InsertMenu(Pos, MF_BYPOSITION | MF_STRING | MF_GRAYED, 
			ID_PLUGIN_INPUT, LDS(ID_PLUGIN_INPUT));
		if (retc)
			InsertCount++;
	}
	return(InsertCount);
}

void CFFRendView::SetMonitorSource(int SlotIdx)
{
	if (SlotIdx == m_Main->GetMonitorSource())	// if source is unchanged
		return;	// nothing to do
	NotifyUndoableEdit(0, UCODE_MONITOR_SOURCE);
	m_Main->SetMonitorSource(SlotIdx);
}

void CFFRendView::OnMonitorSourceChange(int SlotIdx)
{
	if (SlotIdx == m_MonSrcSlotIdx)	// if monitor source is unchanged
		return;	// nothing to do
	if (m_MonSrcSlotIdx >= 0)	// if monitor source was a plugin
		m_TabCtrl.SetImage(m_MonSrcSlotIdx, -1);	// remove previous icon
	if (SlotIdx >= 0)	// if monitor source is a plugin
		m_TabCtrl.SetImage(SlotIdx, 0);	// show monitor icon on plugin's tab
	m_MonSrcSlotIdx = SlotIdx;	// update our shadow
}

void CFFRendView::SetParmVal(int ParmIdx, float Val)
{
	GetCurPlugin()->SetParmVal(ParmIdx, Val);
	GetRow(ParmIdx)->SetVal(Val);
}

void CFFRendView::SetModEnable(int ParmIdx, bool Enable)
{
	GetCurPlugin()->SetModEnable(ParmIdx, Enable);
	GetRow(ParmIdx)->SetModEnable(Enable);
}

void CFFRendView::SetModWave(int ParmIdx, int Wave)
{
	GetCurPlugin()->SetModWave(ParmIdx, Wave);
	GetRow(ParmIdx)->SetModWave(Wave);
}

void CFFRendView::SetModFreq(int ParmIdx, float Freq)
{
	GetCurPlugin()->SetModFreq(ParmIdx, Freq);
	GetRow(ParmIdx)->SetModFreq(Freq);
}

void CFFRendView::SetModPulseWidth(int ParmIdx, float PulseWidth)
{
	GetCurPlugin()->SetModPulseWidth(ParmIdx, PulseWidth);
	GetRow(ParmIdx)->SetModPulseWidth(PulseWidth);
}

void CFFRendView::SetModRange(int ParmIdx, CFFPlugInfo::FRANGE Range)
{
	GetCurPlugin()->SetModRange(ParmIdx, Range);
	GetRow(ParmIdx)->SetModRange(Range);
}

void CFFRendView::SetParmVal(int SlotIdx, int ParmIdx, float Val)
{
	m_Engine->SetCurSel(SlotIdx);
	NotifyUndoableEdit(ParmIdx, UCODE_PARM, UE_COALESCE);
	SetParmVal(ParmIdx, Val);
}

void CFFRendView::SetModEnable(int SlotIdx, int ParmIdx, bool Enable)
{
	m_Engine->SetCurSel(SlotIdx);
	NotifyUndoableEdit(ParmIdx, UCODE_MOD_ENABLE, UE_COALESCE);
	SetModEnable(ParmIdx, Enable);
}

void CFFRendView::SetModWave(int SlotIdx, int ParmIdx, int Wave)
{
	m_Engine->SetCurSel(SlotIdx);
	NotifyUndoableEdit(ParmIdx, UCODE_MOD_WAVE, UE_COALESCE);
	SetModWave(ParmIdx, Wave);
}

void CFFRendView::SetModFreq(int SlotIdx, int ParmIdx, float Freq)
{
	m_Engine->SetCurSel(SlotIdx);
	NotifyUndoableEdit(ParmIdx, UCODE_MOD_FREQ, UE_COALESCE);
	SetModFreq(ParmIdx, Freq);
}

void CFFRendView::SetModPulseWidth(int SlotIdx, int ParmIdx, float PulseWidth)
{
	m_Engine->SetCurSel(SlotIdx);
	NotifyUndoableEdit(ParmIdx, UCODE_MOD_PW, UE_COALESCE);
	SetModPulseWidth(ParmIdx, PulseWidth);
}

void CFFRendView::SetModRange(int SlotIdx, int ParmIdx, CFFPlugInfo::FRANGE Range)
{
	m_Engine->SetCurSel(SlotIdx);
	NotifyUndoableEdit(ParmIdx, UCODE_MOD_RANGE, UE_COALESCE);
	SetModRange(ParmIdx, Range);
}

bool CFFRendView::SyncOscillators(bool Undoable)
{
	STOP_ENGINE(*m_Engine);	// make save and sync atomic
	if (Undoable)
		NotifyUndoableEdit(0, UCODE_SYNC_OSCILLATORS);
	if (!m_Engine->SyncOscillators())
		return(FALSE);
	if (m_Engine->IsPaused())	// if paused, update UI to show sync
		m_Main->SendMessage(WM_TIMER, CMainFrame::VIEW_TIMER_ID);
	return(TRUE);
}

bool CFFRendView::GetOscillatorPhases(COscPhaseArray& OscPhase)
{
	STOP_ENGINE(*m_Engine);
	OscPhase.RemoveAll();
	int	plugs = m_Engine->GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plug = m_Engine->GetPlugin(PlugIdx);
		int	parms = plug.GetParmCount();
		for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
			if (plug.IsModulating(ParmIdx)) {
				OSC_PHASE	op;
				op.SlotIdx = plug.GetSlotIdx();
				op.ParmIdx = ParmIdx;
				op.Phase = plug.GetModPhase(ParmIdx);
				OscPhase.Add(op);
			}
		}
	}
	return(TRUE);
}

bool CFFRendView::SetOscillatorPhases(const COscPhaseArray& OscPhase)
{
	STOP_ENGINE(*m_Engine);
	int	vals = OscPhase.GetSize();
	int	CurSel = m_Engine->GetCurSel();
	for (int ValIdx = 0; ValIdx < vals; ValIdx++) {
		const OSC_PHASE&	op = OscPhase[ValIdx];
		CFFPluginEx	*plug = m_Engine->GetSlot(op.SlotIdx);
		if (plug != NULL) {	// if slot loaded
			if (op.ParmIdx < plug->GetParmCount()) {	// if valid parameter
				plug->SetModPhase(op.ParmIdx, op.Phase);
				if (op.SlotIdx == CurSel)	// if current slot
					GetRow(op.ParmIdx)->SetVal(plug->GetParmVal(op.ParmIdx));
			}
		}
	}
	return(TRUE);
}

void CFFRendView::GetInfo(int SlotIdx, CUndoState& State) const
{
	CRefPtr<CPluginUndoInfo>	uip;
	if (m_Engine->IsLoaded(SlotIdx)) {
		uip.CreateObj();
		m_Engine->GetSlot(SlotIdx)->GetInfo(uip->m_PlugInfo);
		m_Engine->GetRouting(SlotIdx, uip->m_Routing);
		uip->m_Metaparm.Copy(m_Engine->GetMetaplugin().m_Metaparm);
		uip->m_MonitorSource = m_Engine->GetMonitorSource();
	}
	State.SetObj(uip);
}

void CFFRendView::SetInfo(int SlotIdx, const CUndoState& State)
{
	const CPluginUndoInfo	*uip = (const CPluginUndoInfo *)State.GetObj();
	m_Engine->SetRouting(uip->m_Routing);
	m_Engine->GetMetaplugin().m_Metaparm.Copy(uip->m_Metaparm);
	m_Engine->UpdateMetaparmLinks();
	m_Main->SetMonitorSource(uip->m_MonitorSource);
}

void CFFRendView::SaveUndoState(CUndoState& State)
{
	short	ParmIdx = State.GetCtrlID();	// alias depends on case
	short	SlotIdx = State.GetCtrlID();
	switch (State.GetCode()) {
	case UCODE_SELECTION:
		UVCurSel(State) = m_Engine->GetCurSel();
		break;
	case UCODE_PARM:
		UVParmVal(State) = GetCurPlugin()->GetParmVal(ParmIdx);
		break;
	case UCODE_MOD_ENABLE:
		{
			CFFPluginEx	*plug = m_Engine->GetCurPlugin();
			UVModEnable(State) = plug->GetModEnable(ParmIdx);
			UVParmVal(State) = plug->GetParmVal(ParmIdx);
		}
		break;
	case UCODE_MOD_WAVE:
		UVModWave(State) = GetCurPlugin()->GetModWave(ParmIdx);
		break;
	case UCODE_MOD_FREQ:
		{
			CFFPluginEx	*plug = m_Engine->GetCurPlugin();
			UVModFreq(State) = plug->GetModFreq(ParmIdx);
			UVParmVal(State) = plug->GetParmVal(ParmIdx);
		}
		break;
	case UCODE_MOD_PW:
		UVModPW(State) = GetCurPlugin()->GetModPulseWidth(ParmIdx);
		break;
	case UCODE_MOD_RANGE:
		{
			CFFPlugInfo::FRANGE	range = GetCurPlugin()->GetModRange(ParmIdx);
			UVModStart(State) = range.Start;
			UVModEnd(State) = range.End;
		}
		break;
	case UCODE_BYPASS:
		UVBypass(State) = m_Engine->GetBypass(SlotIdx);
		break;
	case UCODE_SOLO:
		UVSolo(State) = m_Engine->InSolo();
		break;
	case UCODE_CUT:
	case UCODE_DELETE:
		if (UndoMgrIsIdle()) {	// if initial state
			GetInfo(SlotIdx, State);
			UVInsert(State) = CUndoManager::UA_UNDO;	// undo inserts, redo deletes
			UVCurSel(State) = m_Engine->GetCurSel();
		}
		break;
	case UCODE_PASTE:
	case UCODE_INSERT:
	case UCODE_INSERT_EMPTY:
		if (UndoMgrIsIdle()) {	// if initial state
			GetInfo(SlotIdx, State);
			UVInsert(State) = CUndoManager::UA_REDO;	// undo deletes, redo inserts
			UVCurSel(State) = m_PrevSel;	// we're notified post-action
		}
		break;
	case UCODE_LOAD:
	case UCODE_UNLOAD:
		GetInfo(SlotIdx, State);
		if (UndoMgrIsIdle())	// if initial state
			UVCurSel(State) = m_Engine->GetCurSel();
		break;
	case UCODE_MOVE:
		if (UndoMgrIsIdle()) {	// if initial state
			// store reversed source and destination
			UVMoveSrc(State) = m_MoveDstSlotIdx;
			UVMoveDst(State) = m_MoveSrcSlotIdx;
		} else {	// exchange source and destination
			int	tmp = UVMoveSrc(State);
			UVMoveSrc(State) = UVMoveDst(State);
			UVMoveDst(State) = tmp;
		}
		break;
	case UCODE_CONNECT:
		if (UndoMgrIsIdle()) {	// if initial state
			UVConnDst(State) = SlotIdx;
			UVConnInp(State) = USLOT(m_ConnDstInpIdx);
		}
		UVConnSrc(State) = USLOT(m_Engine->GetSlot(
			UVConnDst(State))->GetInputSlot(UVConnInp(State)));
		break;
	case UCODE_OPEN_CLIP:
		{
			CRefPtr<CClipUndoInfo>	uip;
			uip.CreateObj();
			uip->m_ClipPath = m_Engine->GetClipPath(SlotIdx);
			State.SetObj(uip);
		}
		break;
	case UCODE_MASTER_SPEED:
		UVSpeed(State) = m_Engine->GetSpeed();
		break;
	case UCODE_MIDI_SETUP:
		{
			CRefPtr<CMidiSetupUndoInfo>	uip;
			uip.CreateObj();
			m_Engine->GetMidiAssignments(uip->m_MidiAssign);
			State.SetObj(uip);
		}
		break;
	case UCODE_LOAD_BALANCE:
		UVThreadCount(State) = m_Engine->GetSlot(SlotIdx)->GetThreadCount();
		break;
	case UCODE_SYNC_OSCILLATORS:
		if (UndoMgrIsIdle()) {	// if initial state
			CRefPtr<CUndoSyncOscillatorsInfo>	uip;
			uip.CreateObj();
			GetOscillatorPhases(uip->m_OscPhase);
			State.SetObj(uip);
		}
		break;
	case UCODE_MONITOR_SOURCE:
		UVMonitorSrc(State) = m_Main->GetMonitorSource();
		break;
	default:
		m_Main->GetMetaparmBar().SaveUndoState(State);
	};
}

void CFFRendView::RestoreUndoState(const CUndoState& State)
{
	short	ParmIdx = State.GetCtrlID();	// alias depends on case
	short	SlotIdx = State.GetCtrlID();
	switch (State.GetCode()) {
	case UCODE_SELECTION:
		m_Engine->SetCurSel(UVCurSel(State));
		break;
	case UCODE_PARM:
		SetParmVal(ParmIdx, UVParmVal(State));
		break;
	case UCODE_MOD_ENABLE:
		SetModEnable(ParmIdx, UVModEnable(State));
		SetParmVal(ParmIdx, UVParmVal(State));	// also set parameter
		GetCurPlugin()->WaitForCommand();	// sync to avoid race with modulator
		break;
	case UCODE_MOD_WAVE:
		SetModWave(ParmIdx, UVModWave(State));
		break;
	case UCODE_MOD_FREQ:
		SetModFreq(ParmIdx, UVModFreq(State));
		SetParmVal(ParmIdx, UVParmVal(State));	// also set parameter
		GetCurPlugin()->WaitForCommand();	// sync to avoid race with modulator
		break;
	case UCODE_MOD_PW:
		SetModPulseWidth(ParmIdx, UVModPW(State));
		break;
	case UCODE_MOD_RANGE:
		{
			CFFPlugInfo::FRANGE	range = {UVModStart(State), UVModEnd(State)};
			SetModRange(ParmIdx, range);
		}
		break;
	case UCODE_BYPASS:
		m_Engine->Bypass(SlotIdx, UVBypass(State));
		break;
	case UCODE_SOLO:
		if (UVSolo(State))
			m_Engine->Solo(SlotIdx);
		else
			m_Engine->EndSolo();
		break;
	case UCODE_CUT:
	case UCODE_PASTE:
	case UCODE_INSERT:
	case UCODE_INSERT_EMPTY:
	case UCODE_DELETE:
	case UCODE_LOAD:
	case UCODE_UNLOAD:
		{
			const CPluginUndoInfo	*uip = 
				(const CPluginUndoInfo *)State.GetObj();
			bool	retc;
			if (UVInsert(State)) {	// if insert/delete
				if (GetUndoAction() == UVInsert(State)) {
					if (uip != NULL) {
						// extend slot change to include set Info
						CEngine::CExtendSlotChange	extend(*m_Engine);
						retc = m_Engine->Insert(SlotIdx, uip->m_PlugInfo);
						if (retc)
							SetInfo(SlotIdx, State);
					} else
						retc = m_Engine->InsertEmpty(SlotIdx);
				} else
					retc = m_Engine->Delete(SlotIdx);
			} else {	// load/unload
				if (uip != NULL) {
					// extend slot change to include set Info
					CEngine::CExtendSlotChange	extend(*m_Engine);
					retc = m_Engine->Load(SlotIdx, uip->m_PlugInfo);
					if (retc)
						SetInfo(SlotIdx, State);
				} else
					retc = m_Engine->Unload(SlotIdx);
			}
			if (retc) {
				if (IsUndoing())
					m_Engine->SetCurSel(UVCurSel(State));	// restore selection
			} else	// undo history is invalid
				ClearUndoHistory();
		}
		break;
	case UCODE_MOVE:
		if (m_Engine->Move(UVMoveSrc(State), UVMoveDst(State))) {
			if (IsUndoing())
				m_Engine->SetCurSel(SlotIdx);	// restore selection
		} else	// undo history is invalid
			ClearUndoHistory();
		break;
	case UCODE_CONNECT:
		if (!m_Engine->Connect(UVConnSrc(State), UVConnDst(State), UVConnInp(State)))
			ClearUndoHistory();	// undo history is invalid
		break;
	case UCODE_OPEN_CLIP:
		{
			const CClipUndoInfo	*uip = (const CClipUndoInfo *)State.GetObj();
			m_Engine->OpenClip(SlotIdx, uip->m_ClipPath);
		}
		break;
	case UCODE_MASTER_SPEED:
		m_Engine->SetSpeed(UVSpeed(State));
		m_Main->GetMasterBar().GetDlg().SetSpeed(UVSpeed(State));
		break;
	case UCODE_MIDI_SETUP:
		{
			CMidiSetupUndoInfo	*uip = (CMidiSetupUndoInfo *)State.GetObj();
			m_Engine->SetMidiAssignments(uip->m_MidiAssign);
			if (m_Main->GetMidiSetupBar().FastIsVisible())
				m_Main->GetMidiSetupBar().UpdateView();
		}
		break;
	case UCODE_LOAD_BALANCE:
		if (m_Engine->IsLoaded(SlotIdx)) {
			CPlugin	*slot = m_Engine->GetSlot(SlotIdx);
			slot->SetThreadCount(UVThreadCount(State));
		}
		break;
	case UCODE_SYNC_OSCILLATORS:
		if (IsUndoing()) {
			CUndoSyncOscillatorsInfo	*uip = 
				(CUndoSyncOscillatorsInfo *)State.GetObj();
			SetOscillatorPhases(uip->m_OscPhase);
		} else	// redoing
			SyncOscillators(FALSE);	// not undoable
		break;
	case UCODE_MONITOR_SOURCE:
		m_Main->SetMonitorSource(UVMonitorSrc(State));
		break;
	default:
		m_Main->GetMetaparmBar().RestoreUndoState(State);
	};
}

CString CFFRendView::GetUndoTitle(const CUndoState& State)
{
	ASSERT(State.GetCode() >= 0 && State.GetCode() < UNDO_CODES);
	CString	Title;
	Title.LoadString(m_UndoTitleID[State.GetCode()]);
	return(Title);
}

/////////////////////////////////////////////////////////////////////////////
// CFFRendView drawing

void CFFRendView::OnDraw(CDC* pDC)
{
}

void CFFRendView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

/////////////////////////////////////////////////////////////////////////////
// CFFRendView printing

BOOL CFFRendView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFFRendView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CFFRendView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CFFRendView diagnostics

#ifdef _DEBUG
void CFFRendView::AssertValid() const
{
	CView::AssertValid();
}

void CFFRendView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFFRendDoc* CFFRendView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFFRendDoc)));
	return (CFFRendDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFFRendView message map

BEGIN_MESSAGE_MAP(CFFRendView, CView)
	//{{AFX_MSG_MAP(CFFRendView)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_COMMAND(ID_EDIT_INSERT_EMPTY, OnEditInsertEmpty)
	ON_COMMAND(ID_EDIT_MOD_RANGE_EDIT, OnEditModRangeEdit)
	ON_COMMAND(ID_EDIT_MOD_RANGE_GOTO_END, OnEditModRangeGotoEnd)
	ON_COMMAND(ID_EDIT_MOD_RANGE_GOTO_START, OnEditModRangeGotoStart)
	ON_COMMAND(ID_EDIT_MOD_RANGE_REMOVE, OnEditModRangeRemove)
	ON_COMMAND(ID_EDIT_MOD_RANGE_SET_END, OnEditModRangeSetEnd)
	ON_COMMAND(ID_EDIT_MOD_RANGE_SET_START, OnEditModRangeSetStart)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_SYNC_OSCILLATORS, OnEditSyncOscillators)
	ON_WM_INITMENUPOPUP()
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_METAPARM_ADD_ALL_PARAMS, OnMetaparmAddAllParams)
	ON_COMMAND(ID_METAPARM_ADD_BYPASS, OnMetaparmAddBypass)
	ON_COMMAND(ID_METAPARM_ADD_MOD_ENAB, OnMetaparmAddModEnab)
	ON_COMMAND(ID_METAPARM_ADD_MOD_FREQ, OnMetaparmAddModFreq)
	ON_COMMAND(ID_METAPARM_ADD_MOD_PW, OnMetaparmAddModPW)
	ON_COMMAND(ID_METAPARM_ADD_MOD_WAVE, OnMetaparmAddModWave)
	ON_COMMAND(ID_METAPARM_ADD_PARAM, OnMetaparmAddParam)
	ON_COMMAND(ID_PLUGIN_BYPASS, OnPluginBypass)
	ON_COMMAND(ID_PLUGIN_LOAD, OnPluginLoad)
	ON_COMMAND(ID_PLUGIN_MONITOR, OnPluginMonitor)
	ON_COMMAND(ID_PLUGIN_PROPERTIES, OnPluginProperties)
	ON_COMMAND(ID_PLUGIN_SOLO, OnPluginSolo)
	ON_COMMAND(ID_PLUGIN_UNLOAD, OnPluginUnload)
	ON_NOTIFY(TCN_SELCHANGE, IDC_FF_TAB_CTRL, OnSelchangeTabCtrl)
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOD_RANGE_REMOVE, OnUpdateEditModRangeRemove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOD_RANGE_SET_START, OnUpdateEditModRangeSet)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SYNC_OSCILLATORS, OnUpdateEditSyncOscillators)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_BYPASS, OnUpdatePluginBypass)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_LOAD, OnUpdatePluginLoad)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_MONITOR, OnUpdatePluginMonitor)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_MRU_FILE1, OnUpdatePluginMru)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_SOLO, OnUpdatePluginSolo)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_UNLOAD, OnUpdatePluginUnload)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOD_RANGE_SET_END, OnUpdateEditModRangeSet)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOD_RANGE_GOTO_START, OnUpdateEditModRangeRemove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOD_RANGE_GOTO_END, OnUpdateEditModRangeRemove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOD_RANGE_EDIT, OnUpdateEditModRangeSet)
	ON_UPDATE_COMMAND_UI(ID_PLUGIN_PROPERTIES, OnUpdatePluginUnload)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_PLUGIN_MRU_FILE1, ID_PLUGIN_MRU_FILE4, OnPluginMruFile)
	ON_MESSAGE(UWM_TABCTRLDRAG, OnTabCtrlDrag)
	ON_MESSAGE(UWM_FFROWEDIT, OnFFRowEdit)
	ON_COMMAND_RANGE(ID_PLUGIN_INPUT_FIRST, ID_PLUGIN_INPUT_LAST, OnPluginInput)
	ON_COMMAND_RANGE(ID_MONITOR_SOURCE_FIRST, ID_MONITOR_SOURCE_LAST, OnMonitorSource)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFRendView message handlers

int CFFRendView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_Main = theApp.GetMain();
	m_Engine = &m_Main->GetEngine();
	m_Main->SetView(this);	// connect to main frame
	SetUndoManager(&m_Main->GetUndoManager());
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create row view
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CFFRowView);
	m_View = DYNAMIC_DOWNCAST(CFFRowView, pFactory->CreateObject());
	DWORD	dwStyle = WS_CHILD | WS_VISIBLE;
    CRect rc(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, rc, this, IDC_FF_PARM_VIEW, NULL))
		return -1;
	m_View->m_Parent = this;
	m_View->SetNotifyWnd(this);
	m_View->SetAccel(NULL, m_Main);
	m_View->CreateCols(COLUMNS, m_ColInfo, IDD_FF_PLUGS_ROW);
	// create tab control
	dwStyle = TCS_FOCUSONBUTTONDOWN | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;
	if (!m_TabCtrl.Create(dwStyle, rc, this, IDC_FF_TAB_CTRL))
		return -1;
	m_TabCtrl.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	m_TabCtrl.SetImageList(&m_TabCtrlImgList);
	// for modulation enable column, show play icon in header instead of text
	if (!m_HdrImgList.Create(5, 9, ILC_MASK, 1, 1))
		return(FALSE);
	HICON hPlay = AfxGetApp()->LoadIcon(IDI_FF_HDR_PLAY);
	m_HdrImgList.Add(hPlay);
	m_View->GetHeader().SetImageList(&m_HdrImgList);
	HDITEM	hdi;
	hdi.mask = HDI_FORMAT | HDI_IMAGE;
	hdi.fmt = HDF_IMAGE | HDF_CENTER;
	hdi.iImage = 0;
	m_View->GetHeader().SetItem(COL_MOD_ENAB, &hdi);

	return 0;
}

void CFFRendView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	m_View->MoveWindow(0, TAB_CTRL_HEIGHT, cx, cy - TAB_CTRL_HEIGHT);
	// stretch tab control's bottom edge over row view's column header
	m_TabCtrl.MoveWindow(0, 0, cx, TAB_CTRL_HEIGHT + 0);
}

void CFFRendView::OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	SlotIdx = m_TabCtrl.GetCurSel();
	m_Engine->SetCurSel(SlotIdx);
	*pResult = 0;
}

LRESULT CFFRendView::OnTabCtrlDrag(WPARAM wParam, LPARAM lParam)
{
	int	SrcSlot = wParam;
	int	DstSlot = m_TabCtrl.CursorHitTest();
	if (DstSlot >= 0)
		Move(SrcSlot, DstSlot);
	return(0);
}

BOOL CFFRendView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (m_ContextTarget != CTX_NO_TARGET) {	// if command came from context menu 
		m_Engine->SetCurSel(m_ContextTarget);	// select target slot
		m_ContextTarget = CTX_NO_TARGET;	// reset state
	}
	return CView::OnCommand(wParam, lParam);
}

void CFFRendView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	int	TargetSlot = GetCurSel();	// target defaults to currently selected slot
	if (point.x == -1 && point.y == -1) {	// if menu triggered via keyboard
		CRect	r;
		if (TargetSlot >= 0) {	// if selection
			m_TabCtrl.GetItemRect(TargetSlot, r);	// position menu over selected tab
			m_TabCtrl.ClientToScreen(r);
		} else	// no selection
			m_TabCtrl.GetWindowRect(r);	// position menu over tab control
		point = r.TopLeft() + CSize(10, 10);	// offset looks nicer
	} else {	// menu triggered via mouse
		// only use tab control hit test if cursor is within tab control
		CRect	r;
		m_TabCtrl.GetWindowRect(r);	// point is in screen coords
		if (r.PtInRect(point)) {	// if cursor is within tab control
			TCHITTESTINFO	hti;
			hti.pt = point;
			ScreenToClient(&hti.pt);	// hit test wants client coords
			TargetSlot = m_TabCtrl.HitTest(&hti);	// target is selected tab if any
		}
	}
	ShowPluginContextMenu(point, TargetSlot);
}

void CFFRendView::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	// search for input popup's placeholder
	int	pos = theApp.FindMenuItem(pPopupMenu, ID_PLUGIN_INPUT);
	if (pos >= 0) {	// if placeholder found, replace with input popup(s)
		CPoint	pt;
		GetCursorPos(&pt);
		int	SlotIdx = m_Main->FindSlot(pt);
		pPopupMenu->RemoveMenu(pos, MF_BYPOSITION);
		InsertInputPopups(pPopupMenu, pos, SlotIdx);
	}
 	CView::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

void CFFRendView::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	// allow our context menu to display hints in the status bar
	m_Main->SendMessage(WM_SETMESSAGESTRING, nItemID);	// show hint for this menu item
}

LRESULT CFFRendView::OnFFRowEdit(WPARAM wParam, LPARAM lParam)
{
	CFFPluginEx	*plug = m_Engine->GetCurPlugin();
	if (plug != NULL) {
		int	ParmIdx = wParam;
		switch (lParam) {
		case IDC_FF_PARM_EDIT:
			NotifyUndoableEdit(ParmIdx, UCODE_PARM, UE_COALESCE);
			plug->SetParmVal(ParmIdx, GetRow(ParmIdx)->GetVal());
			break;
		case IDC_FF_MOD_ENAB:
			NotifyUndoableEdit(ParmIdx, UCODE_MOD_ENABLE, UE_COALESCE);
			plug->SetModEnable(ParmIdx, GetRow(ParmIdx)->GetModEnable());
			break;
		case IDC_FF_MOD_WAVE:
			NotifyUndoableEdit(ParmIdx, UCODE_MOD_WAVE, UE_COALESCE);
			plug->SetModWave(ParmIdx, GetRow(ParmIdx)->GetModWave());
			break;
		case IDC_FF_MOD_FREQ:
			NotifyUndoableEdit(ParmIdx, UCODE_MOD_FREQ, UE_COALESCE);
			plug->SetModFreq(ParmIdx, GetRow(ParmIdx)->GetModFreq());
			break;
		case IDC_FF_MOD_PW:
			NotifyUndoableEdit(ParmIdx, UCODE_MOD_PW, UE_COALESCE);
			plug->SetModPulseWidth(ParmIdx, GetRow(ParmIdx)->GetModPulseWidth());
			break;
		case IDC_FF_PARM_SLIDER:
			if (UndoMgrIsIdle())
				NotifyUndoableEdit(ParmIdx, UCODE_MOD_RANGE, UE_COALESCE);
			plug->SetModRange(ParmIdx, GetRow(ParmIdx)->GetModRange());
			break;
		}
	}
	return(0);
}

void CFFRendView::OnEditCopy() 
{
	if (!CFocusEdit::Copy()) {
		int	SlotIdx = GetCurSel();
		if (SlotIdx >= 0)
			Copy(SlotIdx);
	}
}

void CFFRendView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	if (!CFocusEdit::UpdateCopy(pCmdUI))
		pCmdUI->Enable(HaveCurSel());
}

void CFFRendView::OnEditCut() 
{
	if (!CFocusEdit::Cut()) {
		int	SlotIdx = GetCurSel();
		if (SlotIdx >= 0)
			Cut(SlotIdx);
	}
}

void CFFRendView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	if (!CFocusEdit::UpdateCut(pCmdUI))
		pCmdUI->Enable(HaveCurSel());
}

void CFFRendView::OnEditPaste() 
{
	if (!CFocusEdit::Paste()) {
		int	SlotIdx = GetInsPos();
		Paste(SlotIdx);
	}
}

void CFFRendView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if (!CFocusEdit::UpdatePaste(pCmdUI))
		pCmdUI->Enable(m_Clipboard.HasData());
}

void CFFRendView::OnEditInsert() 
{
	CString	Path;
	if (m_Main->PromptFile(CMainFrame::DIR_PLUGINS, TRUE,	// open file dialog
	NULL, IDS_VIEW_INSERT_PLUGIN, Path)) {
		if (!Insert(GetInsPos(), Path))
			AfxMessageBox(IDS_VIEW_CANT_INSERT_PLUGIN);
	}
}

void CFFRendView::OnEditInsertEmpty() 
{
	InsertEmpty(GetInsPos());
}

void CFFRendView::OnEditDelete() 
{
	if (HaveCurSel())
		Delete(GetCurSel());
}

void CFFRendView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveCurSel());
}

void CFFRendView::OnEditModRangeEdit() 
{
	if (HaveCurSel()) {
		int	SlotIdx = GetCurSel();
		int	RowIdx = GetSelectedRow();
		if (RowIdx >= 0) {
			CString	Name, Caption;
			Name = m_Engine->GetSlot(SlotIdx)->GetParmName(RowIdx);
			AfxFormatString1(Caption, IDS_FF_CAPTION_MOD_RANGE, Name);
			GetRow(RowIdx)->EditModRange(Caption);
		}
	}
}

void CFFRendView::OnEditModRangeRemove() 
{
	int	RowIdx = GetSelectedRow();
	if (RowIdx >= 0)
		GetRow(RowIdx)->RemoveModRange();
}

void CFFRendView::OnEditModRangeSetStart() 
{
	int	RowIdx = GetSelectedRow();
	if (RowIdx >= 0)
		GetRow(RowIdx)->SetModRangeStart();
}

void CFFRendView::OnEditModRangeSetEnd() 
{
	int	RowIdx = GetSelectedRow();
	if (RowIdx >= 0)
		GetRow(RowIdx)->SetModRangeEnd();
}

void CFFRendView::OnEditModRangeGotoStart() 
{
	int	RowIdx = GetSelectedRow();
	if (RowIdx >= 0)
		GetRow(RowIdx)->GotoModRangeStart();
}

void CFFRendView::OnEditModRangeGotoEnd() 
{
	int	RowIdx = GetSelectedRow();
	if (RowIdx >= 0)
		GetRow(RowIdx)->GotoModRangeEnd();
}

void CFFRendView::OnUpdateEditModRangeSet(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetSelectedRow() >= 0);
}

void CFFRendView::OnUpdateEditModRangeRemove(CCmdUI* pCmdUI) 
{
	int	RowIdx = GetSelectedRow();
	pCmdUI->Enable(RowIdx >= 0 && GetRow(RowIdx)->SliderHasSel());
}

void CFFRendView::OnEditSyncOscillators() 
{
	SyncOscillators(TRUE);	// undoable
}

void CFFRendView::OnUpdateEditSyncOscillators(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Engine->GetPluginCount());
}

void CFFRendView::OnPluginLoad() 
{
	if (HaveCurSel()) {
		CString	Path;
		if (m_Main->PromptFile(CMainFrame::DIR_PLUGINS, TRUE,	// open file dialog
		NULL, IDS_VIEW_LOAD_PLUGIN, Path)) {
			if (!Load(GetCurSel(), Path))
				AfxMessageBox(IDS_VIEW_CANT_LOAD_PLUGIN);
		}
	}
}

void CFFRendView::OnPluginUnload() 
{
	if (IsCurSelLoaded())
		Unload(GetCurSel());
}

void CFFRendView::OnUpdatePluginUnload(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsCurSelLoaded());
}

void CFFRendView::OnUpdatePluginLoad(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(HaveCurSel());
}

void CFFRendView::OnPluginMonitor() 
{
	if (IsCurSelLoaded()) {
		int	SlotIdx = GetCurSel();
		if (SlotIdx == m_MonSrcSlotIdx)	// if slot already monitored
			SlotIdx = -1;	// unmonitor it
		SetMonitorSource(SlotIdx);
	}	
}

void CFFRendView::OnUpdatePluginMonitor(CCmdUI* pCmdUI) 
{
	bool	Enable = IsCurSelLoaded();
	pCmdUI->Enable(Enable);
	pCmdUI->SetCheck(Enable && GetCurSel() == m_MonSrcSlotIdx);
}

void CFFRendView::OnPluginBypass() 
{
	if (IsCurSelLoaded()) {
		int	SlotIdx = GetCurSel();
		Bypass(SlotIdx, !m_Engine->GetBypass(SlotIdx));
	}
}

void CFFRendView::OnUpdatePluginBypass(CCmdUI* pCmdUI) 
{
	bool	Enable = IsCurSelLoaded();
	pCmdUI->Enable(Enable);
	pCmdUI->SetCheck(Enable && m_Engine->GetBypass(GetCurSel()));
}

void CFFRendView::OnPluginSolo() 
{
	Solo(GetCurSel());
}

void CFFRendView::OnUpdatePluginSolo(CCmdUI* pCmdUI) 
{
	bool	InSolo = m_Engine->InSolo();
	pCmdUI->Enable(IsCurSelLoaded() || InSolo);
	pCmdUI->SetCheck(InSolo);
}

void CFFRendView::OnPluginInput(UINT nID)
{
	int	idx = nID - ID_PLUGIN_INPUT_FIRST;
	int	sources = m_Engine->GetPluginCount() + 1;	// plus one for default
	int	InpIdx = idx / sources;
	int	PlugIdx = (idx % sources) - 1;
	if (IsCurSelLoaded())
		Connect(PlugIdx, GetCurSel(), InpIdx);
}

void CFFRendView::OnMonitorSource(UINT nID)
{
	int	PlugIdx = (nID - ID_MONITOR_SOURCE_FIRST) - 1;	// first item is default
	int	SlotIdx = m_Engine->MonitorPluginToSlot(PlugIdx);
	SetMonitorSource(SlotIdx);
}

void CFFRendView::OnPluginProperties() 
{
	CFFPluginEx	*plug = GetCurPlugin();
	if (plug != NULL)
		m_Main->ShowPluginProps(plug->GetPath());
}

void CFFRendView::OnUpdatePluginMru(CCmdUI* pCmdUI) 
{
	m_RecentPlugin.UpdateMenu(pCmdUI);
}

void CFFRendView::OnPluginMruFile(UINT nID) 
{
	int	MruIdx = nID - ID_PLUGIN_MRU_FILE1;
	if (!LoadRecent(MruIdx))
		AfxMessageBox(IDS_VIEW_CANT_LOAD_PLUGIN);
}

void CFFRendView::OnMetaparmAddParam()
{
	AddMetaparm(MPT_PARAM, MP_PARAM);
}

void CFFRendView::OnMetaparmAddModEnab()
{
	AddMetaparm(MPT_PARAM, MP_MOD_ENAB);
}

void CFFRendView::OnMetaparmAddModWave()
{
	AddMetaparm(MPT_PARAM, MP_MOD_WAVE);
}

void CFFRendView::OnMetaparmAddModFreq()
{
	AddMetaparm(MPT_PARAM, MP_MOD_FREQ);
}

void CFFRendView::OnMetaparmAddModPW()
{
	AddMetaparm(MPT_PARAM, MP_MOD_PW);
}

void CFFRendView::OnMetaparmAddBypass()
{
	AddMetaparm(MPT_PLUGIN, MP_BYPASS);
}

void CFFRendView::OnMetaparmAddAllParams()
{
	int	SlotIdx = GetCurSel();
	if (SlotIdx >= 0)
		m_Main->AddAllMetaparms(SlotIdx);
}
