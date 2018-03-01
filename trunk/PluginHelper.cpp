// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13sep10	initial version
		01		03apr11	attach helper[0] to plugin's freeframe instance
		02		16apr11	in Work, test UsingProcessCopy, not ProcessCopy
		03		17may11	process copy can set output refs w/o interlock
		04		24may11	in-place case can also set refs w/o interlock
		05		01jun11	in Create, copy parameters from plugin
		06		26mar12	replace next helper with next output event
		07		13apr12	standardize thread function
		08		27apr12	in ResetState, invalidate input and output frames
		09		06may12	add AttachHelper and DetachHelper
		10		31may12	in Work, use QOUTPUT to allow monitoring

		plugin helper thread
 
*/

#include "stdafx.h"
#include "ParaPET.h"
#include "PluginHelper.h"
#include "FFEngine.h"
#include "FFPluginEx.h"

CPluginHelper::CPluginHelper()
{
	m_Plugin = NULL;
	m_HelperIdx = 0;
	m_OutFrame = NULL;
	m_NextOutputEvent = NULL;
}

CPluginHelper::~CPluginHelper()
{
	if (!m_HelperIdx && m_Plugin != NULL)	// if first helper
		m_Plugin->DetachHelper();	// detach from plugin's freeframe instance
}

bool CPluginHelper::Create(CFFPluginEx *Plugin, int HelperIdx)
{
	CFFEngine&	Engine = theApp.GetEngine();
	m_Engine = &Engine;
	m_Plugin = Plugin;
	LPCTSTR	Path = Plugin->GetPath();
	VideoInfoStruct	vis;
	vis.frameWidth = Engine.GetFrameSize().cx;
	vis.frameHeight = Engine.GetFrameSize().cy;
	vis.bitDepth = Engine.GetFFColorDepth();
	vis.orientation = FF_ORIGIN_TOP_LEFT;
	if (!HelperIdx) {	// if first helper
		// attach to plugin's freeframe instance to save memory; thread-safe
		// because plugin delegates all freeframe operations to its helpers
		Plugin->AttachHelper();
	} else {	// other helpers must create their own freeframe instance
		if (!m_FFInst.Create(&Plugin->GetFFPlugin(), vis)) {
			m_Engine->OnError(FFERR_CANT_INSTANTIATE, Path);
			return(FALSE);
		}
	}
	m_HelperIdx = HelperIdx;
	int	helpers = Plugin->GetHelperCount();
	int	next = HelperIdx < helpers - 1 ? HelperIdx + 1 : 0;
	m_NextOutputEvent = &Plugin->GetHelper(next).GetOutputEvent();
	if (!SetNumInputs(Plugin->GetNumInputs()))
		return(FALSE);
	CString	s;
	s.Format(_T("[%d]"), HelperIdx);
	m_Name = Plugin->GetName() + s;
	if (!(m_IdleEvent.Create(NULL, FALSE, FALSE, NULL) &&
		m_InputEvent.Create(NULL, FALSE, FALSE, NULL) &&
		m_OutputEvent.Create(NULL, FALSE, FALSE, NULL))) {
		m_Engine->OnError(ENGERR_CANT_CREATE_EVENT);
		return(FALSE);
	}
	if (!CEngineThread::Create(ThreadFunc, this, 
		m_Engine->GetPriority(), 0, m_Engine->GetFrameTimeout())) {
		m_Engine->OnError(ENGERR_CANT_CREATE_THREAD);
		return(FALSE);
	}
	int	parms = m_Plugin->GetParmCount();
	m_ParmTarget.SetSize(parms);
	m_ParmShadow.SetSize(parms);
	// copy parameters from the plugin's freeframe instance to our shadow and
	// target values; also pass them into our freeframe instance unless we're
	// sharing the plugin's freeframe instance, in which case there's no need
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		float	val = Plugin->GetParmVal(ParmIdx);	// get parameter
		m_ParmShadow[ParmIdx] = val;	// set shadow
		m_ParmTarget[ParmIdx] = val;	// and target
		if (HelperIdx)	// if we're not sharing plugin's freeframe instance
			m_FFInst.SetParam(ParmIdx, val);	// pass parameter to instance
	}
	m_ProcHist.Create(m_Engine->GetHistorySize());
	return(TRUE);
}

void CPluginHelper::UpdateParms()
{
	// copy plugin's current parameters to our targets; worker thread compares
	// our targets to its shadow values and updates freeframe plugin as needed
	int	parms = m_Plugin->GetParmCount();
	for (int i = 0; i < parms; i++)
		m_ParmTarget[i] = m_Plugin->GetParmVal(i);
}

void CPluginHelper::ResetState()
{
	m_IdleEvent.Reset();
	m_InputEvent.Reset();
	m_OutputEvent.Reset();
	// invalidate input and output frames
	ZeroMemory(m_InputFrame.GetData(), m_InputFrame.GetSize() * sizeof(PFRAME));
	m_OutFrame = NULL;
}

bool CPluginHelper::SetNumInputs(int Inputs)
{
	m_InputFrame.SetSize(Inputs);
	m_InFrameBuf.SetSize(Inputs);
	return(TRUE);
}

void CPluginHelper::DumpState(FILE *fp)
{
	_ftprintf(fp, _T("  "));
	m_Engine->DumpStallInfo(fp, m_Name, m_StallInfo);
	_fputts(_T("    In: "), fp);
	int	NumInputs = GetNumInputs();
	for (int InpIdx = 0; InpIdx < NumInputs; InpIdx++) {
		PFRAME InFrame = m_InputFrame[InpIdx];
		if (InFrame != NULL)
			_ftprintf(fp, _T("[%d(%d)] "), InFrame->Idx, InFrame->RefCount);
	}
	_fputtc('\n', fp);
}

UINT CPluginHelper::ThreadFunc(LPVOID Arg)
{
	ENGINE_THREAD_INIT(CPluginHelper);
	while (pThread->Work());
	ENGINE_THREAD_EXIT();
}

bool CPluginHelper::Work()
{
	DWORD	Timeout = m_Engine->GetFrameTimeout() * m_Plugin->GetHelperCount();
	int	FrameLength = m_Engine->GetFrameLength();
	while (1) {
		m_IdleEvent.Set();	// signal idleness
		if (!WaitForEvent(m_InputEvent, Timeout))	// wait for input
			return(!m_KillFlag);
		int	parms = m_Plugin->GetParmCount();
		for (int i = 0; i < parms; i++) {	// for each plugin parameter
			if (m_ParmTarget[i] != m_ParmShadow[i]) {	// if target differs from shadow
				m_FFInst.SetParam(i, m_ParmTarget[i]);	// update freeframe plugin
				m_ParmShadow[i] = m_ParmTarget[i];	// update shadow
			}
		}
		ASSERT(m_InputFrame[0] != NULL);	// sanity check
		int	NumInputs = GetNumInputs();
		if (m_Plugin->UsingProcessCopy()) {	// if using process copy
			QREAD(m_Engine->GetFreeQueue(), m_OutFrame);	// get output frame
			if (!m_Plugin->GetBypass()) {	// if not bypassed
				for (int InpIdx = 0; InpIdx < NumInputs; InpIdx++)
					m_InFrameBuf[InpIdx] = m_InputFrame[InpIdx]->Buf;
				ProcessFrameCopyStruct	pfcs;
				pfcs.InputFrames = m_InFrameBuf.GetData();
				pfcs.numInputFrames = NumInputs;
				pfcs.OutputFrame = m_OutFrame->Buf;
				AddProcessHistorySample(TRUE);
				m_FFInst.ProcessFrameCopy(pfcs);	// process frame
				AddProcessHistorySample(FALSE);
			} else	// bypassed; copy first input to output
				memcpy(m_OutFrame->Buf, m_InputFrame[0], FrameLength);
			if (!WaitForEvent(m_OutputEvent, Timeout))	// wait for output token
				return(!m_KillFlag);
			// write output frame to output queues
			int	outs = m_Plugin->GetOutputCount();
			ASSERT(outs > 0);	// if no outputs, we shouldn't be running
			m_OutFrame->RefCount = outs;	// set output refs
			for (int OutIdx = 0; OutIdx < outs; OutIdx++) {
				QOUTPUT(m_Plugin->GetOutputQueue(OutIdx), m_OutFrame);
			}
			m_OutFrame = NULL;
			// free input frames
			for (int InpIdx = 0; InpIdx < NumInputs; InpIdx++) {
				if (!InterlockedDecrement(&m_InputFrame[InpIdx]->RefCount)) {
					QWRITE(m_Engine->GetFreeQueue(), m_InputFrame[InpIdx]);
				}
				m_InputFrame[InpIdx] = NULL;	// for monitoring
			}
		} else {	// in place
			if (!m_Plugin->GetBypass()) {	// if not bypassed
				AddProcessHistorySample(TRUE);
				m_FFInst.ProcessFrame(m_InputFrame[0]->Buf);	// process frame
				AddProcessHistorySample(FALSE);
			}
			if (!WaitForEvent(m_OutputEvent, Timeout))	// wait for output token
				return(!m_KillFlag);
			// write frame to output queues
			int	outs = m_Plugin->GetOutputCount();
			ASSERT(outs > 0);	// if no outputs, we shouldn't be running
			m_InputFrame[0]->RefCount = outs;	// set output refs
			for (int OutIdx = 0; OutIdx < outs; OutIdx++) {
				QOUTPUT(m_Plugin->GetOutputQueue(OutIdx), m_InputFrame[0]);
			}
			m_InputFrame[0] = NULL;	// for monitoring
		}
		m_NextOutputEvent->Set();	// pass output token
	}
}
