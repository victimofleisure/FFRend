// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		13may10	initial version
		01		24jun10	add IsForcingStall
		02		17mar12	redesign hot unhook to use engine pause
		03		13apr12	standardize thread function

        engine with recording support
 
*/

#include "stdafx.h"
#include "FFRend.h"
#include "Recorder.h"
#include "RecordInfo.h"
#include "FFEngine.h"
#include "FFPluginEx.h"
#include "Renderer.h"
#include "PathStr.h"
#include "VideoComprState.h"

CRecorder::CRecorder(CFFEngine *Engine)
{
	m_Engine = Engine;
	m_Name = _T("Recorder");
	m_IsRecording = FALSE;
	m_IsHooking = FALSE;
	m_IsEnding = FALSE;
	m_PrevBmp = NULL;
	m_FrameSurf = NULL;
	m_Frame = NULL;
}

void CRecorder::DumpState(FILE *fp)
{
	m_Engine->DumpStallInfo(fp, m_Name, m_StallInfo);
	_fputts(_T("  In: "), fp);
	if (m_Frame != NULL)
		_ftprintf(fp, _T("[%d(%d)] "), m_Frame->Idx, m_Frame->RefCount);
	m_InputQueue.DumpState(fp);
}

bool CRecorder::ReplaceOutput(CFrameQueue& Target, CFrameQueue& Substitute)
{
	int	plugs = m_Engine->GetPluginCount();
	if (!plugs)
		return(TRUE);	// nothing to do
	CPlugin&	plug = m_Engine->GetPlugin(plugs - 1);
	int	OutIdx = plug.FindOutput(Target);
	if (OutIdx < 0)
		return(FALSE);	// target not found
	plug.SetOutputQueue(OutIdx, &Substitute);
	return(TRUE);
}

bool CRecorder::HookOutput()
{
	if (m_IsHooking)
		return(TRUE);
	// connect last plugin to our input queue instead of render queue; no need
	// to stop plugin's thread, because setting its output queue pointer is an
	// atomic write and we don't care exactly when frame stream diverts to us
	if (!ReplaceOutput(*m_Engine->GetRenderQueue(), m_InputQueue)) {
		AfxMessageBox(IDS_REC_CANT_HOOK_OUTPUT);
		return(FALSE);
	}
	m_IsHooking = TRUE;
	return(TRUE);
}

bool CRecorder::UnhookOutput()
{
	if (!m_IsHooking)
		return(TRUE);
	// if engine is running and unpaused, do a hot unhook
	if (m_Engine->IsRunning() && !GetEngine()->IsPaused()) {
		// pause engine
		if (!m_Engine->Pause(TRUE))
			return(FALSE);
		// reset engine stop event, so frame queues behave normally
		m_Engine->GetStopEvent().Reset();
		// unpause renderer, so we can flush our frames to it without stalling
		if (!m_Engine->GetRenderer().Pause(FALSE))
			return(FALSE);
		// our worker clears m_Frame after writing it, so if m_Frame isn't empty,
		// assume worker stopped in queue write, leaving current frame unwritten
		if (m_Frame != NULL) {	// if there's a current frame
			m_Engine->GetRenderQueue()->Write(m_Frame);	// flush frame to render queue
			m_Frame = NULL;
		}
		// if unread frames remain in our input queue, flush them to render queue
		m_InputQueue.SetTimeout(0);	// avoids stall; order matters
		PFRAME	Frame;
		while (m_InputQueue.Read(Frame) == CFrameQueue::SUCCESS) {	// if read succeeds
			m_Engine->GetRenderQueue()->Write(Frame);	// flush frame to render queue
		}
		// reconnect last plugin to render queue
		if (!ReplaceOutput(m_InputQueue, *m_Engine->GetRenderQueue())) {
			AfxMessageBox(IDS_REC_CANT_UNHOOK_OUTPUT);
			return(FALSE);
		}
		// if last plugin was blocked trying to write to our input queue when
		// we paused it, it will write to our input queue when we unpause it;
		// reconnect doesn't alter destination of a write already in progress
		int	plugs = m_Engine->GetPluginCount();
		if (plugs) {
			m_InputQueue.SetTimeout(10);	// give last plugin time to write
			m_Engine->GetPlugin(plugs - 1).Pause(FALSE);	// unpause last plugin
			if (m_InputQueue.Read(Frame) == CFrameQueue::SUCCESS) {	// if read succeeds
				m_Engine->GetRenderQueue()->Write(Frame);	// flush frame to render queue
			}
		}
		m_IsRecording = FALSE;	// prevent engine from unpausing our worker
		// unpause engine
		if (!m_Engine->Pause(FALSE))
			return(FALSE);
	} else {	// engine stopped or paused: do a cold unhook
		STOP_ENGINE(*m_Engine);
		// reconnect last plugin to render queue
		if (!ReplaceOutput(m_InputQueue, *m_Engine->GetRenderQueue())) {
			AfxMessageBox(IDS_REC_CANT_UNHOOK_OUTPUT);
			return(FALSE);
		}
	}
	m_IsHooking = FALSE;
	return(TRUE);
}

bool CRecorder::RunInit()
{
	ASSERT(m_IsRecording);
	// assume CEngine::RunInit unhooked us by recreating routes
	m_IsHooking = FALSE;
	return(HookOutput());
}

bool CRecorder::Run(bool Enable)
{
	ASSERT(m_IsRecording);
	if (m_Engine->IsPaused())	// if paused
		return(TRUE);	// succeed without actually starting, as engine does
	if (!CEngineThread::Run(Enable))
		return(FALSE);
	if (!Enable)	// if stopping
		m_InputQueue.Flush();	// flush our input queue
	return(TRUE);
}

bool CRecorder::Pause(bool Enable)
{
	ASSERT(m_IsRecording);
	if (m_Engine->IsRunning())
		return(CEngineThread::Pause(Enable));
	if (!Enable)	// engine stopped: if continuing, run
		return(Run(TRUE));
	return(TRUE);
}

bool CRecorder::Open(LPCTSTR Path, const CRecordInfo& Info, const CVideoComprState *ComprState)
{
	static const LPCTSTR COMPR_STATE_FILE_NAME = _T("ComprState.dat");
	if (m_IsRecording)
		return(FALSE);
	m_RecInfo = Info;
	m_FrameCounter = 0;
	BMPTOAVI_PARMS	parms = {
		m_RecInfo.m_OutFrameSize.cx,
		m_RecInfo.m_OutFrameSize.cy,
		m_RecInfo.m_BitCount,
		m_RecInfo.m_OutFrameRate
	};
	bool	GotComprArg = ComprState != NULL;
	CPathStr	StateFolder, StatePath;
	if (GotComprArg)	// if caller specified a compressor state
		m_RecAvi.SetComprState(*ComprState);	// use it
	else {	// read previous compressor state from file
		theApp.GetAppDataFolder(StateFolder);
		StatePath = StateFolder;
		StatePath.Append(COMPR_STATE_FILE_NAME);
		if (PathFileExists(StatePath) && m_ComprState.Read(StatePath))
			m_RecAvi.SetComprState(m_ComprState);	// restore compressor state
	}
	// only show compression dialog if caller didn't pass compressor state
	if (!m_RecAvi.Open(parms, Path, !GotComprArg)) {	// open output AVI file
		if (m_RecAvi.GetLastError()) {	// if user didn't cancel
			CString	msg, Err, DSErr;
			m_RecAvi.GetLastErrorString(Err, DSErr);
			AfxFormatString2(msg, IDS_REC_CANT_CREATE_FILTER, Err, DSErr);
			AfxMessageBox(msg);
		}
		return(FALSE);
	}
	if (!GotComprArg) {
		m_RecAvi.GetComprState(m_ComprState);
		if (!PathFileExists(StatePath))	// test using shlwapi, it's faster
			theApp.CreateFolder(StateFolder);
		m_ComprState.Write(StatePath);	// save compressor state
	}
	return(TRUE);
}

void CRecorder::Close()
{
	m_RecAvi.Close();
}

bool CRecorder::Start(LPCTSTR Path, const CRecordInfo& Info, const CVideoComprState *ComprState)
{
	if (!Open(Path, Info, ComprState))
		return(FALSE);
	// create frame surface and surface description
	if (!m_Engine->GetRenderer().CreateSurface(m_SurfDesc, m_FrameSurf))
		return(FALSE);
	// create output device context and bitmap
	CClientDC	dc(theApp.GetMain());
	if (!m_OutDC.CreateCompatibleDC(&dc)) {
		AfxMessageBox(IDS_REC_CANT_CREATE_DC);
		return(FALSE);
	}
	WORD	BitCount = static_cast<WORD>(m_RecInfo.m_BitCount);
	CSize	OutFrameSize = m_RecInfo.m_OutFrameSize;
	if (!m_OutDib.Create(OutFrameSize.cx, OutFrameSize.cy, BitCount)) {
		AfxMessageBox(IDS_REC_CANT_CREATE_BITMAP);
		return(FALSE);
	}
	m_PrevBmp = m_OutDC.SelectObject(m_OutDib);
	m_OutDC.SetStretchBltMode(HALFTONE);
	// create input queue and thread
	DWORD	Timeout = m_Engine->GetFrameTimeout();
	if (!m_InputQueue.Create(CEngine::PLUGIN_QUEUE_SIZE, m_Engine->GetStopEvent(), Timeout)) {
		AfxMessageBox(IDS_REC_CANT_CREATE_QUEUE);
		return(FALSE);
	}
	m_Frame = NULL;
	if (!CEngineThread::Create(ThreadFunc, this, m_Engine->GetPriority(), 0, Timeout)) {
		AfxMessageBox(IDS_REC_CANT_CREATE_THREAD);
		return(FALSE);
	}
	m_IsEnding = FALSE;
	if (m_Engine->IsRunning()) {	// if engine running
		if (!HookOutput())	// divert last plugin's frames to our input queue
			return(FALSE);
		if (!m_Engine->IsPaused()) {
			if (!CEngineThread::Run(TRUE)) {	// start our thread
				AfxMessageBox(IDS_REC_CANT_START_THREAD);
				return(FALSE);
			}
		}
	}	// otherwise RunInit hooks output and Run starts our thread
	m_IsRecording = TRUE;
	return(TRUE);
}

bool CRecorder::Stop()
{
	if (!m_IsRecording)
		return(FALSE);
	if (!UnhookOutput())
		return(FALSE);
	Destroy();
	m_IsRecording = FALSE;
	return(TRUE);
}

void CRecorder::Destroy()
{
	CEngineThread::Destroy();
	m_InputQueue.Destroy();
	m_OutDC.SelectObject(m_PrevBmp);
	m_PrevBmp = NULL;
	m_OutDib.Destroy();
	m_OutDC.DeleteDC();
	if (m_FrameSurf != NULL) {
		// reset surface to default before destroying it; prevents a major leak
		m_FrameSurf->SetSurfaceDesc(m_Engine->GetRenderer().GetDefaultSurface(), 0);
		m_FrameSurf->Release();
		m_FrameSurf = NULL;
	}
	m_Frame = NULL;
	m_RecAvi.Close();
}

UINT CRecorder::ThreadFunc(LPVOID Arg)
{
	ENGINE_THREAD_INIT(CRecorder);
	while (pThread->Work());
	ENGINE_THREAD_EXIT();
}

bool CRecorder::Work()
{
	while (1) {
		if (GetStopFlag()) {	// top of loop stop check
			return(WaitForStart());
		} else {
			if (m_Engine->GetPluginCount()) {
				QREAD(&m_InputQueue, m_Frame);	// read input frame
				if (!IsDone()) {
					m_SurfDesc.lpSurface = m_Frame->Buf;
					if (FAILED(m_FrameSurf->SetSurfaceDesc(&m_SurfDesc, 0))) {
						AfxMessageBox(IDS_REC_CANT_SET_SURFACE);
						return(FALSE);
					}
					HDC	sdc;
					if (m_FrameSurf->GetDC(&sdc) != DD_OK) {
						AfxMessageBox(IDS_REC_CANT_GET_SURFACE);
						return(FALSE);
					}
					CSize	FrameSize = GetEngine()->GetFrameSize();
					if (FrameSize != m_RecInfo.m_OutFrameSize) {
						StretchBlt(m_OutDC, 0, 0, 
							m_RecInfo.m_OutFrameSize.cx, m_RecInfo.m_OutFrameSize.cy,
							sdc, 0, 0, FrameSize.cx, FrameSize.cy, SRCCOPY);
					} else {	// no stretching
						BitBlt(m_OutDC, 0, 0, 
							m_RecInfo.m_OutFrameSize.cx, m_RecInfo.m_OutFrameSize.cy,
							sdc, 0, 0, SRCCOPY);	//  ordinary blit performs better
					}
					m_FrameSurf->ReleaseDC(sdc);
					m_RecAvi.AddFrame(m_OutDib);
					m_FrameCounter++;
				} else {	// reached end of recording
					if (!m_RecInfo.m_Unlimited && !m_IsEnding) {
						theApp.GetMain()->PostMessage(UWM_ENDRECORD);
						m_IsEnding = TRUE;	// only one notification
					}
				}
				QWRITE(m_Engine->GetRenderQueue(), m_Frame);	// write frame to render queue
				m_Frame = NULL;	// so unhook knows current frame was written
			} else {	// no plugins, nothing to record
				Sleep(NO_PLUGINS_PAUSE);	// yield CPU
			}
		}
	}
}
