// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      12sep10	in Run/Pause, remove worker thread stop event
        02      14sep10	add plugin helpers
        03      04mar11	make history size unsigned
		04		16apr11	in RunInit, adjust helper frame count for process copy
		05		30apr11	add thread iterator
		06		03may11	resize queues instead of recreating them
		07		10may11	in AllocFrames, add OnDeleteFrames
		08		01dec11	in AllocFrames, catch memory exception
		09		01dec11	add frame memory limit
		10		07may12	in Load, attempt to preserve inputs
		11		01jun12	add FlushQueue

		parallel plugin engine
 
*/

#include "stdafx.h"
#include "Engine.h"
#include "Renderer.h"
#include "Spider.h"

const LPCTSTR CEngine::m_QueueReturnName[CFrameQueue::RETURN_CODES] = {
	_T("Success"), _T("Timeout"), _T("Escaped"), _T("Error")};

CEngine::CRun::CRun(CEngine& Engine, bool Enable) : 
	m_Engine(Engine)
{
	m_PrevRun = Engine.IsRunning();	// save state
	// enter desired state, if not already in it
	m_Succeeded = Enable != m_PrevRun ? Engine.Run(Enable) : TRUE;
}

CEngine::CRun::~CRun()
{
	if (m_Succeeded && m_PrevRun != m_Engine.IsRunning())	// if ctor changed state
		m_Engine.Run(m_PrevRun);	// restore previous state
}

CEngine::CPause::CPause(CEngine& Engine, bool Enable) : 
	m_Engine(Engine)
{
	m_PrevPause = Engine.IsPaused();	// save state
	// enter desired state, if not already in it
	m_Succeeded = Enable != m_PrevPause ? Engine.Pause(Enable) : TRUE;
}

CEngine::CPause::~CPause()
{
	if (m_Succeeded && m_PrevPause != m_Engine.IsPaused())	// if ctor changed state
		m_Engine.Pause(m_PrevPause);	// restore previous state
}

CEngine::CSlotChange::CSlotChange(CEngine& Engine) :
	CRun(Engine, FALSE)	// stop engine during slot change
{
	if (m_Succeeded) {	// if engine stopped ok
		m_PrevInChange = Engine.m_InSlotChange;	// save slot change state
		if (!m_PrevInChange)	// if not nested within previous slot change
			m_Engine.OnBeginSlotChange();	// begin slot change
	}
}

CEngine::CSlotChange::~CSlotChange()
{
	if (m_Succeeded) {	// if engine stopped ok
		if (m_Engine.m_ExtendSlotChange)	// if extending slot change
			m_PrevRun = FALSE;	// don't restart engine
		else {	// no extension
			if (!m_PrevInChange) {	// if not nested within previous slot change
				m_Engine.OnEndSlotChange();	// end slot change
				m_Engine.Run(m_PrevRun);	// restore run state
				m_Engine.PostSlotChange();
			}
		}
	}
}

CEngine::CExtendSlotChange::CExtendSlotChange(CEngine& Engine) :
	m_Engine(Engine)
{
	m_PrevRun = Engine.m_IsRunning;	// save states
	m_PrevInChange = Engine.m_InSlotChange;
	m_PrevExtend = Engine.m_ExtendSlotChange;
	if (!m_PrevExtend) {	// if not nested within previous extension
		m_Engine.OnExtendSlotChange();	// begin extension
		Engine.m_ExtendSlotChange = TRUE;	// enable extension
	}
}

CEngine::CExtendSlotChange::~CExtendSlotChange()
{
	if (!m_PrevExtend) {	// if not nested within previous extension
		// if in slot change, and not nested within previous slot change
		if (m_Engine.m_InSlotChange && !m_PrevInChange) {
			m_Engine.OnEndSlotChange();	// end slot change
			m_Engine.Run(m_PrevRun);	// restore run state
			m_Engine.PostSlotChange();
		}
		m_Engine.m_ExtendSlotChange = FALSE;	// disable extension
	}
}

CEngine::CThreadIter::CThreadIter(CEngine& Engine) : m_Engine(Engine)
{
	m_PlugIdx = 0;
	m_ThreadIdx = 0;
	m_Thread = NULL;
}

bool CEngine::CThreadIter::End()
{
	while (m_PlugIdx < m_Engine.GetPluginCount()) {
		CPlugin&	plug = m_Engine.GetPlugin(m_PlugIdx);
		int	threads = plug.GetThreadCount();
		while (m_ThreadIdx < threads) {
			if (threads > 1)
				m_Thread = &plug.GetHelper(m_ThreadIdx);
			else
				m_Thread = &plug;
			m_ThreadIdx++;
			return(FALSE);
		}
		m_PlugIdx++;
		m_ThreadIdx = 0;
	}
	if (m_PlugIdx == m_Engine.GetPluginCount()) {
		m_PlugIdx++;
		m_Thread = &m_Engine.GetRenderer();
		return(FALSE);
	}
	return(TRUE);
}

CEngine::CEngine(CRenderer& Renderer) :
	m_Renderer(Renderer)
{
	m_IsRunning = FALSE;
	m_IsPaused = FALSE;
	m_InSlotChange = FALSE;
	m_ExtendSlotChange = FALSE;
	m_StallCount = 0;
	m_OptimizeRouting = TRUE;
	m_HistorySize = 0;
	m_Priority = 0;
	m_FrameLength = 0;
	m_FrameTimeout = FRAME_TIMEOUT;
	m_ThreadCount = 0;
	m_FrameMemoryLimit = 0x1000;
	m_MaxFrameCount = INT_MAX;
}

CEngine::~CEngine()
{
}

bool CEngine::Create()
{
	if (!m_Stop.Create(NULL, TRUE, FALSE, NULL)) {	// manual
		OnError(ENGERR_CANT_CREATE_EVENT);
		return(FALSE);
	}
	if (!(m_FreeQueue.Create(PLUGIN_QUEUE_SIZE, m_Stop, m_FrameTimeout)
	&& m_RenderQueue.Create(PLUGIN_QUEUE_SIZE, m_Stop, m_FrameTimeout))) {
		OnError(ENGERR_CANT_CREATE_QUEUE);
		return(FALSE);
	}
	return(TRUE);
}

bool CEngine::Destroy()
{
	if (!Run(FALSE))
		return(FALSE);
	RemoveAll();	// delete all plugins
	DestroyFrames();
	return(TRUE);
}

bool CEngine::SetHistorySize(UINT Size)
{
	if (Size == m_HistorySize)
		return(TRUE);
	STOP_ENGINE(*this);
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++)	// for each plugin
		m_Plugin[PlugIdx]->SetHistorySize(Size);
	m_Renderer.SetHistorySize(Size);
	m_HistorySize = Size;
	return(TRUE);
}

bool CEngine::SetFrameLength(int Length)
{
	if (Length == m_FrameLength)
		return(TRUE);
	STOP_ENGINE(*this);
	m_FrameLength = Length;
	UpdateMaxFrameCount();
	DestroyFrames();	// recreate all frames
	return(TRUE);
}

bool CEngine::SetPriority(int Priority)
{
	if (Priority == m_Priority)
		return(TRUE);
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {	// for each plugin
		if (!m_Plugin[PlugIdx]->SetPriority(Priority))
			return(FALSE);
	}
	m_Priority = Priority;
	return(TRUE);
}

void CEngine::SetFrameTimeout(DWORD Timeout)
{
	if (Timeout == m_FrameTimeout)
		return;
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++)	// for each plugin
		m_Plugin[PlugIdx]->SetTimeout(Timeout);
	m_Renderer.SetTimeout(Timeout);
	m_FreeQueue.SetTimeout(Timeout);
	m_RenderQueue.SetTimeout(Timeout);
	m_FrameTimeout = Timeout;
}

bool CEngine::SetFrameMemoryLimit(UINT Size)
{
	if (Size == m_FrameMemoryLimit)
		return(TRUE);
	STOP_ENGINE(*this);
	m_FrameMemoryLimit = Size;
	UpdateMaxFrameCount();
	return(TRUE);
}

CString	CEngine::GetQueueName(const CFrameQueue *Queue, bool Scope) const
{
	if (Queue == NULL)
		return(_T(""));
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {	// for each plugin
		CPlugin&	plug = *m_Plugin[PlugIdx];
		int	Inputs = plug.GetNumInputs();
		for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {	// for each input
			if (Queue == plug.GetInputQueue(InpIdx)) {
				CString	s, t;
				if (Scope)
					s.Format(_T("%s."), plug.GetName());
				t.Format(_T("In%d"), InpIdx);
				return(s + t);
			}
		}
	}
	if (Queue == &m_FreeQueue)
		return(_T("Free"));
	if (Queue == &m_RenderQueue)
		return(_T("Render"));
	return(_T("Unknown"));
}

bool CEngine::FlushQueue(CFrameQueue& Queue)
{
	if (IsRunning()) {	// if running
		if (Queue.GetTimeout())
			return(FALSE);	// queue must have zero timeout
		PFRAME	Frame;
		while (Queue.Read(Frame) == CFrameQueue::SUCCESS) {
			// assume caller has a reference to frame; subtract a reference
			if (!InterlockedDecrement(&Frame->RefCount)) {	// if no more refs
				if (m_FreeQueue.Write(Frame) != CFrameQueue::SUCCESS) {	// free frame
					OnError(ENGERR_CANT_WRITE_QUEUE);
					return(FALSE);
				}
			}
		}
	} else	// stopped
		Queue.Flush();
	return(TRUE);
}

LPCTSTR CEngine::GetInputName(int SlotIdx) const
{
	if (SlotIdx < 0)
		return(_T("<default>"));
	if (!IsLoaded(SlotIdx))
		return(_T("<INVALID>"));
	return(GetSlot(SlotIdx)->GetName());
}

void CEngine::DumpStallInfo(FILE *fp, LPCTSTR Tag, const STALL_INFO& Info)
{
	static const LPCTSTR QFuncName[CEngineThread::QUEUE_FUNCTIONS] = 
		{_T("Read"), _T("Write")};
	CString	RetcName(GetQueueReturnName(Info.QueueRetc));
	if (Info.Queue != NULL) {
		bool	Scope = Info.QueueFunc == CEngineThread::QF_WRITE;
		_ftprintf(fp, _T("%s: %s %s %s"), Tag, RetcName, QFuncName[Info.QueueFunc], 
			GetQueueName(Info.Queue, Scope));
	} else
		_ftprintf(fp, _T("%s: %s"), Tag, RetcName);
	if (Info.Frame != NULL)
		_ftprintf(fp, _T(": %d(%d)"), Info.Frame->Idx, Info.Frame->RefCount);
	_fputtc('\n', fp);
}

void CEngine::DumpState(FILE *fp)
{
	_fputtc('\n', fp);
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++)	// for each plugin
		m_Plugin[PlugIdx]->DumpState(fp);
	m_Renderer.DumpState(fp);
	_fputts(_T("  Render: "), fp);
	PFRAME	CurFrame = m_Renderer.GetCurFrame();
	if (CurFrame != NULL)
		_ftprintf(fp, _T("[%d(%d)] "), CurFrame->Idx, CurFrame->RefCount);
	m_RenderQueue.DumpState(fp);
	_fputts(_T("  Free: "), fp);
	m_FreeQueue.DumpState(fp);
}

void CEngine::DestroyFrames()
{
	m_Renderer.OnDeleteFrames();	// notify renderer
	int	frames = m_Frame.GetSize();
	for (int i = 0; i < frames; i++)
		delete m_Frame[i];
	m_Frame.RemoveAll();
}

bool CEngine::AllocFrames(int FrameCount)
{
	int	PrevCount = m_Frame.GetSize();
	TRY {
		// grow or shrink frame array
		if (FrameCount > PrevCount) {	// if growing
			m_Frame.SetSize(FrameCount);
			UINT	FrameBytes = sizeof(FRAME_HEADER) + m_FrameLength;
			for (int i = PrevCount; i < FrameCount; i++)
				m_Frame[i] = (PFRAME)new BYTE[FrameBytes];
		} else {	// shrinking
			m_Renderer.OnDeleteFrames();	// notify renderer
			for (int i = FrameCount; i < PrevCount; i++)
				delete m_Frame[i];
			m_Frame.SetSize(FrameCount);
		}
	}
	CATCH (CMemoryException, e) {
		DestroyFrames();
		m_IsPaused = TRUE;
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	if (!m_FreeQueue.SetSize(FrameCount)) {
		OnError(ENGERR_CANT_CREATE_QUEUE);
		return(FALSE);
	}
	// add frames to free queue
	for (int i = 0; i < FrameCount; i++) {
		PFRAME&	Frame = m_Frame[i];
		ZeroMemory(Frame, sizeof(FRAME));
		Frame->Idx = i;
		if (m_FreeQueue.Write(Frame) != CFrameQueue::SUCCESS) {
			OnError(ENGERR_CANT_WRITE_QUEUE);
			return(FALSE);
		}
	}
	return(TRUE);
}

void CEngine::UpdateMaxFrameCount()
{
	int	FrameLengthKB = m_FrameLength >> 10;	// convert from bytes to KB
	if (FrameLengthKB)	// avoid divide by zero
		m_MaxFrameCount = (m_FrameMemoryLimit << 10) / FrameLengthKB;
	else	// no limit
		m_MaxFrameCount = INT_MAX;
}

bool CEngine::RunInit()
{
	int	plugs = GetPluginCount();
	int	PlugIdx;
	for (PlugIdx = 0; PlugIdx < plugs; PlugIdx++)	// for each plugin
		m_Plugin[PlugIdx]->ResetQueues();
	int	TotalFrames = 0;
	if (plugs) {
		CSpider	spider(*this);
		spider.Crawl(m_OptimizeRouting);
	}
	int	RenderFrameCounter = m_Renderer.GetFrameCounter();
	for (PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {	// for each plugin
		CPlugin&	plug = *m_Plugin[PlugIdx];
		plug.SetFrameCounter(RenderFrameCounter);
		if (plug.CanRender()) {	// if plugin can render
			int	Inputs = plug.GetNumInputs();
			if (Inputs > 1 && !plug.GetProcessCopy()) {
				OnError(ENGERR_NEED_PROCESS_COPY, plug.GetName());
				return(FALSE);
			}
			for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {	// for each input
				int	InQSize = plug.GetInputQueueSize(InpIdx);
				if (!plug.GetInputQueue(InpIdx)->SetSize(InQSize)) {
					OnError(ENGERR_CANT_CREATE_QUEUE, plug.GetName());
					return(FALSE);
				}
				if (!plug.IsSource()) {	// if plugin is an effect
					CPlugin	*src = plug.GetInputSource(InpIdx);
					if (src != NULL)	// if input source is non-default
						plug.ConnectInput(InpIdx, *src);	// connect input
				}
				TotalFrames += InQSize + 1;	// input queue size, plus one for input buffer
			}
			TotalFrames++;	// plus one for output buffer, or possible copy if in-place
			int	helpers = plug.GetHelperCount();
			if (helpers) {
				// each helper has input buffers, and an output if using process copy
				TotalFrames += helpers * (Inputs + plug.UsingProcessCopy());
				// if using process copy, plugin doesn't use its output buffer
				TotalFrames -= plug.UsingProcessCopy();	// deduct one in that case
			}
		}
	}
	TotalFrames += 2;	// plus one for render queue, and one for render buffer
	// TotalFrames ignores memory limits and allows for the theoretical worst
	// case, so the free queue can't underrun even if every seat is occupied
	if (!AllocFrames(min(TotalFrames, m_MaxFrameCount)))
		return(FALSE);
	if (plugs)
		m_Plugin[plugs - 1]->ConnectOutput(m_RenderQueue);
	return(TRUE);
}

bool CEngine::Run(bool Enable)
{
	if (Enable == m_IsRunning)
		return(TRUE);	// nothing to do
	int	PlugIdx;
	int	plugs = GetPluginCount();
	if (Enable) {	// if starting
		if (m_IsPaused)	// if paused
			return(TRUE);	// succeed without actually starting
		if (!RunInit())
			return(FALSE);
		m_Stop.Reset();
	} else {	// stopping
		m_Stop.Set();	// unblock frame queue readers/writers
	}
	for (PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {	// for each plugin
		if (!m_Plugin[PlugIdx]->Run(Enable))	// wait for thread to reach desired state
			return(FALSE);
	}
	if (!m_Renderer.Run(Enable))
		return(FALSE);
	if (!Enable) {	// if stopped
		m_FreeQueue.Flush();	// keep empty, else newly created plugins won't stop
		m_RenderQueue.Flush();
	}
	m_StallCount = 0;
	m_IsRunning = Enable;
	return(TRUE);
}

bool CEngine::Pause(bool Enable)
{
	if (Enable == m_IsPaused)
		return(TRUE);
	if (m_IsRunning) {	// if engine running
		int	plugs = GetPluginCount();
		if (Enable) {	// if pausing
			m_Stop.Set();	// unblock frame queue readers/writers
		} else	// continuing
			m_Stop.Reset();
		for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {	// for each plugin
			if (!m_Plugin[PlugIdx]->Pause(Enable))	// wait for thread to reach desired state
				return(FALSE);
		}
		if (!m_Renderer.Pause(Enable))
			return(FALSE);
		m_IsPaused = Enable;
	} else {	// engine stopped
		m_IsPaused = Enable;	// order matters: run checks paused flag
		if (!Enable)	// if continuing, run
			return(Run(TRUE));
	}
	return(TRUE);
}

void CEngine::OnBeginSlotChange()
{
	m_InSlotChange = TRUE;
	m_Plugin.RemoveAll();	// no plugin pointers during slot change
}

void CEngine::OnEndSlotChange()
{
	int	slots = GetSlotCount();	// rebuild plugin pointers
	m_Plugin.SetSize(slots);
	int	plugs = 0;
	m_ThreadCount = 0;
	for (int SlotIdx = 0; SlotIdx < slots; SlotIdx++) {	// for each slot
		if (IsLoaded(SlotIdx)) {	// if slot is loaded
			CPlugin	*plug = m_Slot[SlotIdx];
			plug->SetSlotIdx(SlotIdx);
			plug->SetPluginIdx(plugs);
			m_Plugin[plugs] = plug;
			m_ThreadCount += plug->GetThreadCount();
			plugs++;
		}
	}
	m_Plugin.SetSize(plugs);
	m_InSlotChange = FALSE;
}

void CEngine::OnExtendSlotChange()
{
}

void CEngine::PostSlotChange()
{
}

void CEngine::OnError(int ErrorCode, LPCTSTR Context)
{
}

bool CEngine::RemoveAll()
{
	SLOT_CHANGE(*this);
	m_Slot.RemoveAll();
	m_Plugin.RemoveAll();
	m_Renderer.GetProcessHistory().Flush();
	return(TRUE);
}

bool CEngine::Insert(int SlotIdx, CSlotPtr Slot)
{
	SLOT_CHANGE(*this);
	m_Slot.InsertAt(SlotIdx, Slot);
	for (CInputIter InpIter(m_Slot); !InpIter.End();) {	// for each input
		if (*InpIter >= SlotIdx)	// if input source is at or above insertion
			(*InpIter)++;	// increment input index
	}
	return(TRUE);
}

bool CEngine::Delete(int SlotIdx)
{
	SLOT_CHANGE(*this);
	m_Slot.RemoveAt(SlotIdx);
	for (CInputIter InpIter(m_Slot); !InpIter.End();) {	// for each input
		if (*InpIter > SlotIdx)	// if input source is above deletion
			(*InpIter)--;	// decrement input index
		else if (*InpIter == SlotIdx)	// else if input source was deleted
			(*InpIter) = -1;	// disconnect input
	}
	return(TRUE);
}

bool CEngine::Load(int SlotIdx, CSlotPtr Slot)
{
	SLOT_CHANGE(*this);
	CDWordArray	PrevInput;
	if (IsLoaded(SlotIdx)) {	// if slot is loaded
		// save plugin's input sources
		CPlugin	*plug = GetSlot(SlotIdx);
		int	inputs = plug->GetNumInputs();
		PrevInput.SetSize(inputs);
		for (int InpIdx = 0; InpIdx < inputs; InpIdx++)
			PrevInput[InpIdx] = plug->GetInputSlot(InpIdx);
	}
	m_Slot[SlotIdx] = Slot;
	if (Slot != NULL) {	// if loading
		// restore previous input sources as much as possible
		CPlugin	*plug = GetSlot(SlotIdx);
		if (!plug->IsSource()) {	// if new plugin is an effect
			int	inputs = min(plug->GetNumInputs(), PrevInput.GetSize());
			for (int InpIdx = 0; InpIdx < inputs; InpIdx++)
				plug->SetInputSlot(InpIdx, PrevInput[InpIdx]);
		}
	} else {	// unloading
		for (CInputIter InpIter(m_Slot); !InpIter.End();) {	// for each input
			if (*InpIter == SlotIdx)	// if input source was unloaded
				(*InpIter) = -1;	// disconnect input
		}
	}
	return(TRUE);
}

bool CEngine::Move(int Src, int Dst)
{
	if (Src == Dst)
		return(TRUE);
	SLOT_CHANGE(*this);
	CSlotPtr	temp = m_Slot[Src];
	m_Slot.RemoveAt(Src);
	m_Slot.InsertAt(Dst, temp);
	//	for all inputs in the range Src..Dst
	//		if (input == Src)
	//			shift input by (Dst - Src)
	//		else
	//			if (Src < Dst)
	//				shift input down one
	//			else
	//				shift input up one
	int	Delta = Dst - Src;
	int	Shift, Start, End;
	if (Src < Dst) {
		Start = Src;
		End = Dst;
		Shift = -1;
	} else {	// invert range
		Start = Dst;
		End = Src;
		Shift = 1;
	}
	for (CInputIter InpIter(m_Slot); !InpIter.End();) {
		if (*InpIter >= Start && *InpIter <= End) {
			if (*InpIter == Src)
				*InpIter += Delta;
			else
				*InpIter += Shift;
		}
	}
	return(TRUE);
}

void CEngine::OnStall(CEngineThread *Staller)
{
	if (InterlockedIncrement(&m_StallCount) == 1)	// suppress duplicate notifications
		AfxGetMainWnd()->PostMessage(UWM_ENGINESTALL, WPARAM(Staller));
}

void CEngine::PostCrawl()
{
}

CEngine::CInputIter::CInputIter(const CSlotArray& Slot) :
	m_Slot(Slot)
{
	m_SlotIdx = 0;
	m_InpIdx = 0;
	m_Input = NULL;
}

bool CEngine::CInputIter::End()
{
	while (m_SlotIdx < m_Slot.GetSize()) {
		CPlugin	*pp = m_Slot[m_SlotIdx];
		if (pp != NULL) {
			while (m_InpIdx < pp->GetNumInputs()) {
				pp->GetInputSlot(m_InpIdx, m_Input);
				m_InpIdx++;
				return(FALSE);
			}
			m_InpIdx = 0;
		}
		m_SlotIdx++;
	}
	return(TRUE);
}
