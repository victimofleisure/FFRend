// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		06may10	initial version
		01		24jun10	override OnStall to ignore recorder forced stall
		02		19jul11	in Run, don't restart views while paused
		03		24nov11	pass previous slot to OnSelChange
		04		17mar12	remove OnStall override
		05		26mar12 add monitor source
		06		30mar12 clear monitor bar if plugin can't render
		07		13apr12	move monitor UID check into tap
		08		23may12	make monitor source a slot index
		09		01jun12	refactor monitor bypass and pause

        main frame engine
 
*/

#include "stdafx.h"
#include "FFRend.h"
#include "ClipEngine.h"
#include "FFPluginEx.h"
#include "MainFrm.h"
#include "FFRendDoc.h"
#include "FFRendView.h"
#include "Undoable.h"
#include "UndoCodes.h"
#include "PathStr.h"

// create array of engine error string resource IDs
#undef ENGERR_DEF
#define ENGERR_DEF(x) IDS_##x,
const int CMainEngine::m_EngineErrorID[FFENGINE_ERRORS] = {
#include "EngineErrs.h"
#include "FFEngineErrs.h"
};

CMainEngine::CMainEngine(CRenderer& Renderer) :
	CClipEngine(Renderer)
{
	m_Main = NULL;
	m_UndoMgr = NULL;
	// set clip player path; assume app folder unless registry overrides it
	CPathStr	ClipPlayerPath(theApp.GetAppFolder());
	ClipPlayerPath.Append(_T("PlayerFF.dll"));
	theApp.RdReg2String(_T("ClipPlayerPath"), ClipPlayerPath);
	SetClipPlayerPath(ClipPlayerPath);
	m_MonitorSource = -1;
}

bool CMainEngine::Create(CMainFrame *Main)
{
	m_Main = Main;
	m_UndoMgr = &Main->GetUndoManager();
	m_MonitorTap.SetOutputQueue(&m_Renderer.GetMonitorQueue());
	return(CClipEngine::Create());
}

bool CMainEngine::Run(bool Enable)
{
	if (Enable == m_IsRunning)
		return(TRUE);	// nothing to do
	CFastWaitCursor	wc;
	if (!Enable) {	// if stopping
		m_Main->RunViews(FALSE);	// stop views
#ifdef ENGINE_NATTER
		DumpState(stdout);
#endif
	}
	if (!CClipEngine::Run(Enable))	// call base class
		return(FALSE);
	if (m_Main->IsRecording()) {
		if (!m_Main->GetRecorder().Run(Enable))
			return(FALSE);
	}
	if (Enable) {	// if starting
		if (!IsPaused())	// if not paused
			m_Main->RunViews(TRUE);	// restart views
	} else	// stopping
		m_Renderer.GetMonitorQueue().Flush();	// flush monitor frame queue
	return(TRUE);
}

bool CMainEngine::RunInit()
{
	if (!CClipEngine::RunInit())
		return(FALSE);
	if (m_Main->IsRecording()) {
		if (!m_Main->GetRecorder().RunInit())
			return(FALSE);
	}
	m_MonitorTap.RunInit();
	return(TRUE);
}

bool CMainEngine::Pause(bool Enable)
{
	if (Enable == m_IsPaused)
		return(TRUE);	// nothing to do
	if (!CClipEngine::Pause(Enable))	// call base class
		return(FALSE);
	if (m_Main->IsRecording())
		m_Main->GetRecorder().Pause(Enable);
	m_Main->RunViews(!Enable);
	m_MonitorTap.Pause(Enable);
	return(TRUE);
}

void CMainEngine::OnBeginSlotChange()
{
	if (!m_ExtendSlotChange)
		theApp.FastBeginWaitCursor();
	CClipEngine::OnBeginSlotChange();
}

void CMainEngine::OnExtendSlotChange()
{
	if (!m_InSlotChange)
		theApp.FastBeginWaitCursor();
	CClipEngine::OnExtendSlotChange();
}

void CMainEngine::OnEndSlotChange()
{
	CClipEngine::OnEndSlotChange();
	if (!MonitoringOutput()) {	// if monitoring a plugin
		int	SlotIdx = m_MonitorTap.OnEndSlotChange();
		SetMonitorSource(SlotIdx);
	}
}

void CMainEngine::PostSlotChange()
{
	CClipEngine::PostSlotChange();
	m_Main->UpdateViews();
	theApp.FastEndWaitCursor();
}

void CMainEngine::OnError(int ErrorCode, LPCTSTR Context)
{
	ASSERT(ErrorCode >= 0 && ErrorCode < FFENGINE_ERRORS);
	CString	msg;
	AfxFormatString1(msg, m_EngineErrorID[ErrorCode], Context);
	AfxMessageBox(msg);
}

inline short CMainEngine::USLOT(int SlotIdx)
{
	ASSERT(SlotIdx >= SHRT_MIN && SlotIdx <= SHRT_MAX);
	return(static_cast<short>(SlotIdx));	// limited to 32K slots
}

inline void CMainEngine::NotifyUndoableEdit(int SlotIdx, WORD Code, UINT Flags)
{
	m_UndoMgr->NotifyEdit(USLOT(SlotIdx), Code, Flags);
}

inline void CMainEngine::CancelUndoableEdit(int SlotIdx, WORD Code)
{
	m_UndoMgr->CancelEdit(USLOT(SlotIdx), Code);
}

void CMainEngine::SetCurSel(int SlotIdx)
{
	if (SlotIdx == m_CurSel)
		return;
	if (m_UndoMgr->IsIdle()) {
		NotifyUndoableEdit(0, UCODE_SELECTION, 
			CUndoable::UE_COALESCE | CUndoable::UE_INSIGNIFICANT);
	}
	int	PrevSlotIdx = m_CurSel;
	CClipEngine::SetCurSel(SlotIdx);
	m_Main->OnSelChange(PrevSlotIdx, SlotIdx);
}

void CMainEngine::Bypass(int SlotIdx, bool Enable)
{
	CClipEngine::Bypass(SlotIdx, Enable);
	m_Main->OnBypass(SlotIdx, Enable);
}

void CMainEngine::DumpState(FILE *fp)
{
	CClipEngine::DumpState(fp);
	if (m_Main->IsRecording())
		m_Main->GetRecorder().DumpState(fp);
}

CString CMainEngine::GetQueueName(const CFrameQueue *Queue, bool Scope) const
{
	if (Queue == m_Main->GetRecorder().GetInputQueue())
		return(_T("Recorder"));
	return(CClipEngine::GetQueueName(Queue, Scope));
}

bool CMainEngine::OpenClipUndoable(int SlotIdx, LPCTSTR Path)
{
	NotifyUndoableEdit(SlotIdx, UCODE_OPEN_CLIP);
	bool	retc = GetSlot(SlotIdx)->OpenClip(Path);
	if (!retc)
		CancelUndoableEdit(SlotIdx, UCODE_OPEN_CLIP);
	return(retc);
}

bool CMainEngine::InsertClipPlayerUndoable(int SlotIdx, LPCTSTR Path)
{
	// save current selection; undo notification is post-insert
	m_Main->GetView()->SetPrevSel(m_CurSel);
	bool	retc = InsertClipPlayer(SlotIdx, Path);	
	if (retc)	
		NotifyUndoableEdit(SlotIdx, UCODE_INSERT);
	return(retc);
}

bool CMainEngine::LoadClipPlayerUndoable(int SlotIdx, LPCTSTR Path)
{
	NotifyUndoableEdit(SlotIdx, UCODE_LOAD);
	bool	retc = LoadClipPlayer(SlotIdx, Path);
	if (!retc)
		CancelUndoableEdit(SlotIdx, UCODE_LOAD);
	return(retc);
}

bool CMainEngine::OpenClip(int SlotIdx, LPCTSTR Path)
{
	bool	retc;
	if (!IsValidSlot(SlotIdx))	// if invalid slot
		SlotIdx = GetCurSel();	// try current selection instead
	if (IsValidSlot(SlotIdx) && !IsLoaded(SlotIdx)) {	// if valid but empty slot
		retc = LoadClipPlayerUndoable(SlotIdx, Path);	// load player
	} else {
		if (IsValidSlot(SlotIdx) && IsLoaded(SlotIdx) 
		&& GetSlot(SlotIdx)->IsClipPlayer()) {	// if slot contains a player
			retc = OpenClipUndoable(SlotIdx, Path);	// open clip in it
		} else {
			SlotIdx = FindFirstClipPlayer();	// search for player
			if (SlotIdx >= 0) {		// if player was found
				retc = OpenClipUndoable(SlotIdx, Path);	// open clip in it
			} else {	// player not found
				retc = InsertClipPlayerUndoable(0, Path);	// insert player in first slot
			}
		}
	}
	if (!retc) {
		CString	s;
		AfxFormatString1(s, IDS_MF_CANT_OPEN_CLIP, Path);
		LPCTSTR	Ext = PathFindExtension(Path);
		if (Ext != NULL && (!_tcsicmp(Ext, MPG_EXT) || !_tcsicmp(Ext, AVS_EXT)))
			s += LDS(IDS_MF_NEED_AVISYNTH);	// add AviSynth warning
		AfxMessageBox(s);
	}
	return(retc);
}

bool CMainEngine::Load(int SlotIdx, LPCTSTR Path)
{
	CExtendSlotChange	extend(*this);
	if (!CClipEngine::Load(SlotIdx, Path))
		return(FALSE);
	if (SlotIdx == m_MonitorSource)
		m_MonitorTap.SetPlugin(GetSlot(SlotIdx));
	return(TRUE);
}

bool CMainEngine::Load(int SlotIdx, const CFFPlugInfo& Info)
{
	CExtendSlotChange	extend(*this);
	if (!CClipEngine::Load(SlotIdx, Info))
		return(FALSE);
	if (SlotIdx == m_MonitorSource)
		m_MonitorTap.SetPlugin(GetSlot(SlotIdx));
	return(TRUE);
}

bool CMainEngine::SetMonitorSource(int SlotIdx)
{
	if (!(IsValidSlot(SlotIdx) && IsLoaded(SlotIdx)))	// if slot isn't a plugin
		SlotIdx = -1;	// default monitor source
	if (SlotIdx == m_MonitorSource)	// if source hasn't changed
		return(TRUE);	// nothing to do
	// renderer should monitor only if monitoring output and monitor bar is shown
	m_Renderer.Monitor(SlotIdx < 0 && m_Main->GetMonitorBar().FastIsVisible());
	m_MonitorSource = -1;	// failsafe
	if (SlotIdx >= 0) {
		CFFPluginEx	*plug = GetSlot(SlotIdx);
		if (!m_MonitorTap.Attach(*plug)) {	// attach monitor tap to plugin
			AfxMessageBox(IDS_MF_CANT_ATTACH_MONITOR);
			return(FALSE);
		}
		if (!plug->CanRender()) {	// if plugin can't render
			m_Renderer.FlushMonitorQueue();		// flush monitor queue first
			m_Main->GetMonitorBar().Clear();	// then clear monitor window
		}
	} else {	// default monitor source
		if (!m_MonitorTap.Detach()) {	// detach monitor tap
			AfxMessageBox(IDS_MF_CANT_DETACH_MONITOR);
			return(FALSE);
		}
		if (IsPaused())	// if paused
			m_Main->GetMonitorBar().UpdateView();	// restore pause frame
	}
	m_MonitorSource = SlotIdx;
	return(TRUE);
}
