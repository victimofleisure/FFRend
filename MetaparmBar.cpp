// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
		01		23nov07	support Unicode
		02		22dec07	in EditProps, notify undo before calling Reset
		03		22dec07	in LinkToPlugin, verify plugin is loaded
		04		22dec07	in SetInfo, move SetParm after LinkToPlugin
		05		23dec07	move rebuild to RebuildMetaparms
		06		25dec07	add metaparameter groups
		07		09jan08	decorate group name to distinguish slaves
		08		29jan08	in SaveUndoState, add casts to fix warnings
		09		01may10	port to refactored RowView
		10		29aug10	remove dirty view flag
		11		12may11	in UpdateView, update MIDI even if we're hidden
		12		11nov11	fix keyboard-triggered context menu

        metaplugin parameter control bar
 
*/

// MetaparmBar.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MetaparmBar.h"
#include "MetaparmRow.h"
#include "MainFrm.h"
#include "MetaparmPropsDlg.h"
#include "MetaparmGroupDlg.h"
#include "FFPluginEx.h"
#include "UndoCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetaparmBar dialog

IMPLEMENT_DYNCREATE(CMetaparmView, CRowView)
IMPLEMENT_DYNAMIC(CMetaparmBar, CMySizingControlBar);

const CRowView::COLINFO CMetaparmBar::m_ColInfo[COLUMNS] = {
	{IDC_METAPARM_NAME,		IDS_METAPARM_NAME},
	{IDC_METAPARM_SLIDER,	IDS_FF_PARM_SLIDER},
	{IDC_METAPARM_EDIT,		IDS_FF_PARM_EDIT},
	{IDC_METAPARM_GROUP,	IDS_METAPARM_GROUP},
};

CMetaparmBar::CMetaparmBar(CMetaparmArray& Metaparm, CWnd* pParent /*=NULL*/) :
	m_Metaparm(Metaparm)
{
	//{{AFX_DATA_INIT(CMetaparmBar)
	//}}AFX_DATA_INIT
    m_szVert = CSize(260, 400);	// size of bar when vertically docked
	m_View = NULL;
	m_Engine = NULL;
	m_Frm = NULL;
	m_CurRow = -1;
	m_MoveSrcIdx = 0;
}

inline void CMetaparmBar::SetVal(int ParmIdx, float Val)
{
	if (FastIsVisible())
		GetRow(ParmIdx)->SetVal(Val);
}

inline void CMetaparmBar::SetParm(int ParmIdx, const CMetaparm& Parm)
{
	if (FastIsVisible())
		GetRow(ParmIdx)->SetParm(Parm);
}

void CMetaparmBar::UpdateView()
{
	if (FastIsVisible())
		m_View->CreateRows(m_Metaparm.GetSize());
	UpdateMidi();	// regardless of whether we're visible
}

void CMetaparmBar::LinkToPlugin(CMetaparm& Parm)
{
	ASSERT(Parm.IsAssigned());	// assume we have a valid target
	int	TargetPlugin = Parm.GetTargetPlugin();
	if (TargetPlugin >= 0) {	// if target has an associated plugin
		CFFPluginEx	*slot = m_Engine->GetSlot(TargetPlugin);
		if (slot != NULL) {		// if slot is loaded
			Parm.m_PluginUID = slot->GetUID();	// copy plugin's UID
		} else {	// empty slot
			int	ParmIdx = m_Metaparm.Find(Parm);
			if (ParmIdx >= 0)	// if metaparameter is in our array
				m_Metaparm.Reset(ParmIdx);	// reset it via array's function
			else	// not found in our array
				Parm.Reset();	// reset metaparameter to default values
		}
	}
}

void CMetaparmBar::UpdateSlaves(CMetaparm& Parm)
{
	int	Slaves = Parm.GetSlaveCount();
	for (int i = 0; i < Slaves; i++) {	// for each slave
		int	SlaveIdx = Parm.m_Slave[i];
		CMetaparm&	slave = m_Metaparm[SlaveIdx];
		slave.m_Value = Parm.m_Value;	// assign master's value to slave
		m_Engine->ApplyMetaparm(slave);	// update slave's target
		SetVal(SlaveIdx, slave.m_Value);	// update row controls
	}
}

void CMetaparmBar::SetValue(int ParmIdx, float Value)
{
	CMetaparm&	parm = m_Metaparm[ParmIdx];
	parm.m_Value = Value;
	m_Engine->ApplyMetaparm(parm);	// update target
	SetVal(ParmIdx, Value);	// update row controls
	if (parm.IsMaster())
		UpdateSlaves(parm);
}

void CMetaparmBar::OnRowValChange(int ParmIdx, float Value)
{
	NotifyUndoableEdit(ParmIdx, UCODE_METAPARM_VALUE, UE_COALESCE);
	CMetaparm&	parm = m_Metaparm[ParmIdx];
	parm.m_Value = Value;
	m_Engine->ApplyMetaparm(parm);	// update target
	if (parm.IsMaster())
		UpdateSlaves(parm);
}

void CMetaparmBar::Add(const TARGET& Target)
{
	CMetaparm	parm;
	parm.m_Target = Target;
	CMetaparmPropsDlg::MakeMetaparmName(Target, parm.m_Name);	// synthesize name
	parm.m_Value = m_Engine->GetMidiProperty(Target);	// get value from target
	m_CurRow = GetRows();	// set current row
	LinkToPlugin(parm);		// link to target's associated plugin if any
	Insert(m_CurRow, parm);	// append to end of list
	NotifyUndoableEdit(m_CurRow, UCODE_METAPARM_INSERT);	// notify AFTER edit
}

void CMetaparmBar::Insert(int ParmIdx)
{
	CMetaparm	parm;
	m_Metaparm.InsertAt(ParmIdx, parm);
	UpdateView();
}

void CMetaparmBar::Insert(int ParmIdx, CMetaparm& Parm)
{
	m_Metaparm.InsertAt(ParmIdx, Parm);
	UpdateView();
}

void CMetaparmBar::Delete(int ParmIdx)
{
	m_Metaparm.RemoveAt(ParmIdx);
	UpdateView();
}

void CMetaparmBar::Move(int SrcRow, int DstRow)
{
	m_Metaparm.Move(SrcRow, DstRow);
	UpdateView();
}

void CMetaparmBar::OnDrop(int SrcRow, int DstRow)
{
	m_MoveSrcIdx = SrcRow;	// set source row for undo handler
	NotifyUndoableEdit(DstRow, UCODE_METAPARM_DRAG);	// notify undo manager
	Move(SrcRow, DstRow);	// move the row
}

void CMetaparmBar::Reset(int ParmIdx)
{
	bool	WasGrouped = m_Metaparm[ParmIdx].IsGrouped();
	m_Metaparm.Reset(ParmIdx);
	SetParm(ParmIdx, m_Metaparm[ParmIdx]);
	if (WasGrouped)
		UpdateGroupNames();
}

void CMetaparmBar::UpdateMidi()
{
	CMidiSetupBar&	msbar = m_Frm->GetMidiSetupBar();
	if (msbar.FastIsVisible()) {
		CMidiSetupDlg&	msdlg = msbar.GetDlg();
		if (msdlg.GetPageType() == MPT_METAPARM)
			msdlg.UpdateView();
	}
	m_Engine->MakeMidiMap();
}

void CMetaparmBar::SetMetaparm(int ParmIdx, const CMetaparm& Parm)
{
	CMetaparm&	dest = m_Metaparm[ParmIdx];
	dest = Parm;	// store edited properties
	if (dest.IsAssigned()) {	// if metaparameter has a target
		dest.m_Value = m_Engine->GetMidiProperty(dest.m_Target);	// get value from target
		LinkToPlugin(dest);	// link to target's associated plugin if any	
		SetParm(ParmIdx, dest);	// set row controls
	} else
		Reset(ParmIdx);	// reset metaparameter to default values
	UpdateMidi();
}

void CMetaparmBar::EditProps(int ParmIdx)
{
	CMetaparm	parm = m_Metaparm[ParmIdx];	// edit a copy, in case user cancels
	CMetaparmPropsDlg	dlg(parm);
	if (dlg.DoModal() == IDOK) {	// if user didn't cancel
		m_CurRow = ParmIdx;	// set current row
		NotifyUndoableEdit(ParmIdx, UCODE_METAPARM_PROPS);
		SetMetaparm(ParmIdx, parm);
	}
}

CString	CMetaparmBar::GetDecoratedGroupName(int ParmIdx)
{
	CString	s(m_Metaparm.GetGroupName(ParmIdx));
	if (m_Metaparm[ParmIdx].IsSlave())	// if slave
		s.Insert(0, _T("-> "));	// decorate name
	return(s);
}

void CMetaparmBar::UpdateGroupNames()
{
	if (FastIsVisible()) {
		int	parms = m_Metaparm.GetSize();
		for (int i = 0; i < parms; i++)
			GetRow(i)->SetGroupName(GetDecoratedGroupName(i));
	}
#ifdef METAGROUP_NATTER
	m_Metaparm.DumpGroups();
#endif
}

bool CMetaparmBar::EditGroup(int ParmIdx)
{
	CMetaparmArray	tmp;
	tmp.Copy(m_Metaparm);
	CMetaparmGroupDlg	dlg(tmp, ParmIdx);
	if (dlg.DoModal() == IDOK) {
		m_CurRow = ParmIdx;	// set current row
		NotifyUndoableEdit(ParmIdx, UCODE_METAPARM_GROUP);
		m_Metaparm.Copy(tmp);
		UpdateGroupNames();
		return(TRUE);
	}
	return(FALSE);
}

void CMetaparmBar::Ungroup(int ParmIdx)
{
	m_CurRow = ParmIdx;	// set current row
	NotifyUndoableEdit(ParmIdx, UCODE_METAPARM_UNGROUP);
	m_Metaparm.Ungroup(ParmIdx);
	UpdateGroupNames();
}

void CMetaparmBar::SetRowInfo(int RowIdx, CMetaparmRow& Row)
{
	if (RowIdx < m_Metaparm.GetSize()) {
		const CMetaparm& parm = m_Metaparm[RowIdx];
		bool	IsAssigned = parm.IsAssigned();
		CString	GroupName;
		if (IsAssigned) {	// if metaparameter has a target
			if (parm.IsGrouped())
				GroupName = GetDecoratedGroupName(RowIdx);
		} else	// no target
			Row.EnableCtrls(FALSE);	// disable row's controls
		Row.SetParm(parm);	// set parameter row's controls
		Row.SetGroupName(GroupName);
	}
}

CRowDlg *CMetaparmView::CreateRow(int Idx)
{
	CMetaparmRow	*rp = new CMetaparmRow;
	rp->Create(IDD_METAPARM_ROW);
	m_Parent->SetRowInfo(Idx, *rp);
	return(rp);
}

void CMetaparmView::UpdateRow(int Idx)
{
	CMetaparmRow	*rp = (CMetaparmRow *)GetRow(Idx);
	m_Parent->SetRowInfo(Idx, *rp);
	rp->Invalidate();
}

void CMetaparmView::OnDrop(int SrcRow, int DstRow)
{
	m_Parent->OnDrop(SrcRow, DstRow);
}

void CMetaparmBar::GetInfo(int ParmIdx, CUndoState& State) const
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetRows());
	CRefPtr<CUndoInfo>	uob;
	uob.CreateObj();
	CMetaparm&	parm = m_Metaparm[ParmIdx];
	uob->m_Parm = parm;
	if (parm.IsGrouped()) {
		CMetaparmGroup	*grp = new CMetaparmGroup;	// allocate group
		m_Metaparm.GetGroup(ParmIdx, *grp);	// store group
		uob->m_Group.SetPtr(grp);	// copy pointer into state
		uob->m_Parm.m_Master = -1;	// remove group info in m_Parm
		uob->m_Parm.m_Slave.RemoveAll();
	}
	if (State.GetCode() == UCODE_METAPARM_GROUP) {	// if editing a group
		CParmValArray	*pva = new CParmValArray;	// allocate values
		int	parms = m_Metaparm.GetSize();	// save metaparameter values
		pva->SetSize(parms);
		for (int i = 0; i < parms; i++)
			(*pva)[i] = m_Metaparm[i].m_Value;
		uob->m_ParmVal.SetPtr(pva);	// copy pointer into state
	}
	State.SetObj(uob);
}

void CMetaparmBar::SetInfo(int ParmIdx, const CUndoState& State)
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetRows());
	CMetaparm&	parm = m_Metaparm[ParmIdx];
	if (parm.IsGrouped())	// if metaparameter is grouped
		m_Metaparm.Unlink(ParmIdx);	// leave group
	const CUndoInfo	*uob = (const CUndoInfo *)State.GetObj();
	parm = uob->m_Parm;
	// During the undo/redo process, a plugin can be destroyed and recreated,
	// causing its unique ID (UID) to change. This is potentially a problem,
	// because metaparameters link to plugins by storing their UIDs. If the
	// metaparameter we're setting is linked to a plugin, we assume the link
	// is broken, and repair it, by storing the plugin's (possibly new) UID.
	if (parm.IsAssigned()) 	// if metaparameter has a target
		LinkToPlugin(parm);		// link to target's associated plugin if any
	if (uob->m_Group) {	// if state contains a group
		CMetaparmGroup	*grp = uob->m_Group;
		m_Metaparm.Ungroup(grp->m_Master);	// destroy group
		m_Metaparm.SetGroup(*grp);	// restore group
	}
	if (uob->m_ParmVal) {	// if state contains values
		CParmValArray	*pva = uob->m_ParmVal;
		int	parms = pva->GetSize();
		for (int i = 0; i < parms; i++) {	// for each value
			CMetaparm&	mp = m_Metaparm[i];
			mp.m_Value = (*pva)[i];	// store value
			SetVal(i, mp.m_Value);	// update row
			m_Engine->ApplyMetaparm(mp);	// update target
		}
	}
	SetParm(ParmIdx, parm);
	UpdateMidi();
}

void CMetaparmBar::SaveUndoState(CUndoState& State)
{
	short	ParmIdx = State.GetCtrlID();
	switch (State.GetCode()) {
	case UCODE_METAPARM_VALUE:
		UValValue(State) = m_Metaparm[ParmIdx].m_Value;
		break;
	case UCODE_METAPARM_INSERT:
		if (UndoMgrIsIdle()) {	// if initial state
			GetInfo(ParmIdx, State);
			UValInsert(State) = CUndoManager::UA_REDO;	// undo deletes, redo inserts
		}
		break;
	case UCODE_METAPARM_DELETE:
		if (UndoMgrIsIdle()) {	// if initial state
			GetInfo(ParmIdx, State);
			UValInsert(State) = CUndoManager::UA_UNDO;	// undo inserts, redo deletes
		}
		break;
	case UCODE_METAPARM_DRAG:
		if (UndoMgrIsIdle()) {	// if initial state
			// store reversed source and destination
			UValMoveSrc(State) = ParmIdx;
			UValMoveDst(State) = m_MoveSrcIdx;
		} else {	// exchange source and destination
			int	tmp = UValMoveSrc(State);
			UValMoveSrc(State) = UValMoveDst(State);
			UValMoveDst(State) = tmp;
		}
		break;
	case UCODE_METAPARM_PROPS:
	case UCODE_METAPARM_GROUP:
	case UCODE_METAPARM_UNGROUP:
		GetInfo(ParmIdx, State);
		break;
	default:
		NODEFAULTCASE;
	};
}

void CMetaparmBar::RestoreUndoState(const CUndoState& State)
{
	short	ParmIdx = State.GetCtrlID();
	switch (State.GetCode()) {
	case UCODE_METAPARM_VALUE:
		SetValue(ParmIdx, UValValue(State));
		break;
	case UCODE_METAPARM_INSERT:
	case UCODE_METAPARM_DELETE:
		{
			if (GetUndoAction() == UValInsert(State)) {
				Insert(ParmIdx);
				SetInfo(ParmIdx, State);
				if (m_Metaparm[ParmIdx].IsGrouped())
					UpdateGroupNames();
			} else
				Delete(ParmIdx);
		}
		break;
	case UCODE_METAPARM_DRAG:
		Move(UValMoveSrc(State), UValMoveDst(State));
		break;
	case UCODE_METAPARM_PROPS:
	case UCODE_METAPARM_GROUP:
	case UCODE_METAPARM_UNGROUP:
		SetInfo(ParmIdx, State);
		UpdateGroupNames();
		break;
	default:
		NODEFAULTCASE;
	};
}

CString CMetaparmBar::GetUndoTitle(const CUndoState& State)
{
	return(_T(""));
}

void CMetaparmBar::DoDataExchange(CDataExchange* pDX)
{
	CMySizingControlBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetaparmBar)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMetaparmBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CMetaparmBar)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_METAPARM_INSERT, OnInsert)
	ON_COMMAND(ID_METAPARM_DELETE, OnDelete)
	ON_COMMAND(ID_METAPARM_PROPS, OnProperties)
	ON_COMMAND(ID_METAPARM_GROUP, OnGroup)
	ON_COMMAND(ID_METAPARM_UNGROUP, OnUngroup)
	ON_UPDATE_COMMAND_UI(ID_METAPARM_DELETE, OnUpdateDelete)
	ON_UPDATE_COMMAND_UI(ID_METAPARM_GROUP, OnUpdateGroup)
	ON_UPDATE_COMMAND_UI(ID_METAPARM_UNGROUP, OnUpdateUngroup)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_METAPARM_PROPS, OnUpdateDelete)
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetaparmBar message handlers

int CMetaparmBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_Frm = theApp.GetMain();
	SetUndoManager(&m_Frm->GetUndoManager());
	m_Engine = &theApp.GetEngine();
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CMetaparmView);
	m_View = DYNAMIC_DOWNCAST(CMetaparmView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_METAPARM_VIEW, NULL))
		return -1;
	m_View->m_Parent = this;
	m_View->SetNotifyWnd(this);
	m_View->SetAccel(NULL, theApp.GetMain());
	m_View->CreateCols(COLUMNS, m_ColInfo, IDD_METAPARM_ROW);
	
	return 0;
}

void CMetaparmBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_View->MoveWindow(0, 0, cx, cy);
}

void CMetaparmBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		UpdateView();
}

void CMetaparmBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	m_View->FixContextMenuPos(point);
	CMenu	menu, *mp;
	menu.LoadMenu(IDR_METAPARM_CTX);
	mp = menu.GetSubMenu(0);
	CPoint	Cursor = point;
	ScreenToClient(&Cursor);
	m_CurRow = m_View->FindRow(Cursor);
	theApp.UpdateMenu(this, mp);
	mp->TrackPopupMenu(0, point.x, point.y, this);
}

void CMetaparmBar::OnInsert()
{
	if (m_CurRow < 0)	// if cursor wasn't on a row
		m_CurRow = GetRows();	// append to end of list
	Insert(m_CurRow);
	NotifyUndoableEdit(m_CurRow, UCODE_METAPARM_INSERT);	// notify AFTER edit
}

void CMetaparmBar::OnDelete()
{
	if (m_CurRow >= 0) {
		NotifyUndoableEdit(m_CurRow, UCODE_METAPARM_DELETE);
		Delete(m_CurRow);
	}
}

void CMetaparmBar::OnUpdateDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CurRow >= 0);
}

void CMetaparmBar::OnProperties()
{
	if (m_CurRow >= 0)
		EditProps(m_CurRow);
}

void CMetaparmBar::OnGroup()
{
	if (m_CurRow >= 0)
		EditGroup(m_CurRow);
}

void CMetaparmBar::OnUngroup()
{
	if (m_CurRow >= 0)
		Ungroup(m_CurRow);
}

void CMetaparmBar::OnUpdateGroup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CurRow >= 0 && m_Metaparm[m_CurRow].IsAssigned());
}

void CMetaparmBar::OnUpdateUngroup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CurRow >= 0 && m_Metaparm[m_CurRow].IsGrouped());
}
