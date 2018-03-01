// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		26apr10	in Run, clear m_OutFrame AFTER thread stops
        02      14sep10	add helpers
		03		11jan11	add priming accessors
        04      04mar11	make history size unsigned
		05		16apr11	set m_UseProcessCopy in ResetQueues instead of Run
		06		03may11	create input queues in SetNumInputs
		07		13apr12	standardize thread function
		08		27apr12	invalidate output frame in ResetQueues instead of Run
		09		01jun12	add DisconnectOutput

		parallel plugin
 
*/

#include "stdafx.h"
#include "Plugin.h"
#include "Engine.h"

CPlugin::CPlugin()
{
	m_OutFrame = NULL;
	m_SlotIdx = 0;
	m_PlugIdx = 0;
	m_Bypass = FALSE;
	m_IsSource = FALSE;
	m_ProcessCopy = FALSE;
	m_UseProcessCopy = FALSE;
	m_CopyPref = CEngine::CPF_NONE;
	m_NumInputs = 0;
	m_CanRender = FALSE;
	m_ProcessDelay = 0;
	m_ThreadCount = 1;
	m_NextHelper = 0;
}

CPlugin::~CPlugin()
{
	Destroy();
}

CPlugin::CInput::CInput()
{
	m_Frame = NULL;
	m_SlotIdx = -1;
	m_HasOutput = FALSE;
	m_PrimeFrames = 0;
	m_QueueSize = CEngine::PLUGIN_QUEUE_SIZE;
}

void CPlugin::DumpState(FILE *fp)
{
	m_Engine->DumpStallInfo(fp, m_Name, m_StallInfo);
	for (int InpIdx = 0; InpIdx < m_NumInputs; InpIdx++) {
		CInput	inp = m_Input[InpIdx];
		_ftprintf(fp, _T("  In%d %s Out=%c: "), InpIdx, 
			m_Engine->GetInputName(inp.m_SlotIdx), inp.m_HasOutput ? 'Y' : 'N');
		PFRAME InFrame = GetInputFrame(InpIdx);
		if (InFrame != NULL)
			_ftprintf(fp, _T("[%d(%d)] "), InFrame->Idx, InFrame->RefCount);
		m_InputQueue[InpIdx].DumpState(fp);
	}
	_fputts(_T("  Out: "), fp);
	PFRAME OutFrame = GetOutputFrame();
	if (OutFrame != NULL)
		_ftprintf(fp, _T("[%d(%d)] "), OutFrame->Idx, OutFrame->RefCount);
	for (int OutIdx = 0; OutIdx < m_OutputQueue.GetSize(); OutIdx++) {
		if (OutIdx)
			_fputts(_T(", "), fp);
		_ftprintf(fp, _T("%s"), m_Engine->GetQueueName(m_OutputQueue[OutIdx]));
	}
	_fputtc('\n', fp);
	int	helpers = GetHelperCount();
	for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++)
		m_Helper[HelpIdx].DumpState(fp);
}

bool CPlugin::Create(CEngine& Engine, int NumInputs)
{
	m_Engine = &Engine;
	m_ProcHist.Create(Engine.GetHistorySize());
	if (!SetNumInputs(NumInputs))
		return(FALSE);
	if (!CEngineThread::Create(ThreadFunc, this, Engine.GetPriority(), 0, Engine.GetFrameTimeout())) {
		Engine.OnError(ENGERR_CANT_CREATE_THREAD);
		return(FALSE);
	}
	return(TRUE);
}

bool CPlugin::Destroy()
{
	if (GetHelperCount()) {
		if (!DestroyHelpers())
			return(FALSE);
	}
	return(CEngineThread::Destroy());
}

void CPlugin::ResetQueues()
{
	m_OutputQueue.RemoveAll();
	for (int InpIdx = 0; InpIdx < m_NumInputs; InpIdx++) {
		m_InputQueue[InpIdx].Flush();
		CInput&	inp = m_Input[InpIdx];
		inp.m_HasOutput = FALSE;
		inp.m_PrimeFrames = 0;
		inp.m_Frame = NULL;	// invalidate input frame
		inp.m_QueueSize = CEngine::PLUGIN_QUEUE_SIZE;
	}
	m_OutFrame = NULL;	// invalidate output frame
	m_CanRender = FALSE;
	int	helpers = GetHelperCount();
	if (helpers) {
		for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++)
			m_Helper[HelpIdx].ResetState();
		m_NextHelper = 0;	// first helper is up next
		m_Helper[0].GetOutputEvent().Set();	// grant output access to first helper
	}
	// was in Run, moved here so CEngine::RunInit can test UsingProcessCopy
	if (m_ProcessCopy)
		m_UseProcessCopy = m_CopyPref != CEngine::CPF_INPLACE || m_NumInputs > 1;
	else
		m_UseProcessCopy = FALSE;
}

void CPlugin::ConnectInput(int InpIdx, CPlugin& Plugin)
{
	if (!m_Input[InpIdx].m_PrimeFrames)
		m_Input[InpIdx].m_HasOutput = TRUE;
	Plugin.m_OutputQueue.Add(&m_InputQueue[InpIdx]);
}

bool CPlugin::DisconnectOutput(CFrameQueue& Queue)
{
	int	OutIdx = FindOutput(Queue);
	if (OutIdx < 0)	// if queue not found
		return(FALSE);
	m_OutputQueue.RemoveAt(OutIdx);
	return(TRUE);
}

int CPlugin::FindOutput(const CFrameQueue& Queue) const
{
	int	outs = m_OutputQueue.GetSize();
	for (int OutIdx = 0; OutIdx < outs; OutIdx++) {
		if (m_OutputQueue[OutIdx] == &Queue)
			return(OutIdx);
	}
	return(-1);
}

void CPlugin::SetBypass(bool Enable)
{
	m_Bypass = Enable;
}

bool CPlugin::SetSource(bool Enable)
{
	if (Enable == m_IsSource)
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	m_IsSource = Enable;
	return(TRUE);
}

bool CPlugin::SetProcessCopy(bool Enable)
{
	if (Enable == m_ProcessCopy)
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	m_ProcessCopy = Enable;
	return(TRUE);
}

bool CPlugin::SetCopyPref(int Pref)
{
	if (Pref == m_CopyPref)
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	m_CopyPref = Pref;
	return(TRUE);
}

bool CPlugin::SetNumInputs(int Inputs)
{
	if (Inputs == m_NumInputs)
		return(TRUE);
	SLOT_CHANGE(*m_Engine);
	m_Input.SetSize(Inputs);
	m_InputQueue.SetSize(Inputs);
	m_InFrameBuf.SetSize(Inputs);
	m_NumInputs = Inputs;
	int	helpers = GetHelperCount();
	for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++) {
		if (!m_Helper[HelpIdx].SetNumInputs(Inputs))
			return(FALSE);
	}
	for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {
		if (!m_InputQueue[InpIdx].Create(m_Input[InpIdx].m_QueueSize, 
			m_Engine->GetStopEvent(), m_Engine->GetFrameTimeout())) {
			m_Engine->OnError(ENGERR_CANT_CREATE_QUEUE);
			return(FALSE);
		}
	}
	return(TRUE);
}

bool CPlugin::SetInputSlot(int InpIdx, int SlotIdx)
{
	if (SlotIdx == m_Input[InpIdx].m_SlotIdx)
		return(TRUE);
	SLOT_CHANGE(*m_Engine);
	m_Input[InpIdx].m_SlotIdx = SlotIdx;
	return(TRUE);
}

int CPlugin::GetInputPlugin(int InpIdx) const
{
	int	SlotIdx = GetInputSlot(InpIdx);
	return(SlotIdx >= 0 ? m_Engine->GetSlot(SlotIdx)->GetPluginIdx() : -1);
}

bool CPlugin::SetInputPlugin(int InpIdx, int PlugIdx)
{
	int	SlotIdx = PlugIdx >= 0 ? m_Engine->GetPlugin(PlugIdx).GetSlotIdx() : -1;
	return(SetInputSlot(InpIdx, SlotIdx));
}

CPlugin *CPlugin::GetInputSource(int InpIdx)
{
	int	InputSlot = GetInputSlot(InpIdx);
	if (InputSlot >= 0)
		return(m_Engine->GetSlot(InputSlot));
	if (m_PlugIdx > 0)
		return(&m_Engine->GetPlugin(m_PlugIdx - 1));
	return(NULL);
}

bool CPlugin::SetInputQueueSize(int InpIdx, int Size)
{
	if (Size == m_Input[InpIdx].m_QueueSize)
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	m_Input[InpIdx].m_QueueSize = Size;
	return(TRUE);
}

bool CPlugin::SetInputPriming(int InpIdx, int Frames)
{
	if (Frames == m_Input[InpIdx].m_PrimeFrames)
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	m_Input[InpIdx].m_PrimeFrames = Frames;
	return(TRUE);
}

void CPlugin::SetTimeout(DWORD Timeout)
{
	CEngineThread::SetTimeout(Timeout);
	for (int InpIdx = 0; InpIdx < m_NumInputs; InpIdx++)
		m_InputQueue[InpIdx].SetTimeout(Timeout);
}

bool CPlugin::SetHistorySize(UINT Size)
{
	if (Size == m_ProcHist.GetSize())
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	if (!CEngineThread::SetHistorySize(Size))
		return(FALSE);
	int	helpers = GetHelperCount();
	for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++) {
		if (!m_Helper[HelpIdx].SetHistorySize(Size))
			return(FALSE);
	}
	return(TRUE);
}

bool CPlugin::SetThreadCount(int Threads)
{
	if (Threads == m_ThreadCount)
		return(TRUE);
	SLOT_CHANGE(*m_Engine);
	m_ThreadCount = Threads;
	int	helpers = Threads > 1 ? Threads : 0;
	return(CreateHelpers(helpers));
}

bool CPlugin::CreateHelpers(int Helpers)
{
	if (GetHelperCount()) {	// destroy existing helpers if any
		if (!DestroyHelpers())
			return(FALSE);
	}
	m_Helper.SetSize(Helpers);
	if (Helpers) {
		for (int HelpIdx = 0; HelpIdx < Helpers; HelpIdx++) {
			if (!m_Helper[HelpIdx].Create((CFFPluginEx *)this, HelpIdx))
				return(FALSE);
		}
		m_NextHelper = 0;
	}
	return(TRUE);
}

bool CPlugin::DestroyHelpers()
{
	int	helpers = GetHelperCount();
	for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++) {
		if (!m_Helper[HelpIdx].Destroy())
			return(FALSE);
	}
	m_Helper.RemoveAll();
	return(TRUE);
}

bool CPlugin::Delegate()
{
	int	helpers = GetHelperCount();
	CPluginHelper&	helper = m_Helper[m_NextHelper];
	m_NextHelper++;
	if (m_NextHelper >= helpers)
		m_NextHelper = 0;
	if (!WaitForEvent(helper.GetIdleEvent(), m_Timeout))
		return(FALSE);
	helper.SetFrameCounter(m_FrameCounter);
	for (int InpIdx = 0; InpIdx < m_NumInputs; InpIdx++)
		helper.SetInputFrame(InpIdx, m_Input[InpIdx].m_Frame);
	helper.UpdateParms();
	helper.GetInputEvent().Set();
	return(TRUE);
}

bool CPlugin::Run(bool Enable)
{
	if (Enable) {	// if starting thread
		if (!m_CanRender) {	// if not connected to output
#ifdef ENGINE_NATTER
			ResetStallInfo();	// normally done by base class; avoids stale frame pointers
#endif
			return(TRUE);	// succeed without starting thread
		}
		if (!CEngineThread::Run(TRUE))
			return(FALSE);
	} else {	// stopping thread
		if (!CEngineThread::Run(FALSE))
			return(FALSE);
	}
	int	helpers = GetHelperCount();
	for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++) {
		if (!m_Helper[HelpIdx].Run(Enable))	// start/stop helper thread
			return(FALSE);
	}
	return(TRUE);
}

bool CPlugin::Pause(bool Enable)
{
	if (!Enable && !m_CanRender)	// if continuing and not connected to output
		return(TRUE);	// don't start thread
	if (!CEngineThread::Pause(Enable))
		return(FALSE);
	int	helpers = GetHelperCount();
	for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++) {
		if (!m_Helper[HelpIdx].Pause(Enable))	// pause helper thread
			return(FALSE);
	}
	return(TRUE);
}
