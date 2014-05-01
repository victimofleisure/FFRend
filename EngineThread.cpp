// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		14sep10	add WaitForEvent
        02      04mar11	make history size unsigned
		03		14may11	add GetRunTime
		04		13apr12	add COM MTA init
		05		31may12	add Output method

		parallel engine thread base class
 
*/

#include "stdafx.h"
#include "EngineThread.h"
#include "Engine.h"

CEngineThread::CEngineThread()
{
	m_Engine = NULL;
	ResetStallInfo();
	m_IsProcessing = FALSE;
	m_IsPausing = FALSE;
	m_FrameCounter = 0;
	m_RunTime = 0;
}

void CEngineThread::ThreadInit()
{
#ifdef _DEBUG
	SetThreadName(DWORD(-1), m_Name);
#endif
#if ENGINE_USES_COM
	if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK) {
		m_Engine->OnError(ENGERR_CANT_INIT_THREAD);
		ExitThread(ENGERR_CANT_INIT_THREAD);
	}
#endif
}

bool CEngineThread::SetHistorySize(UINT Size)
{
	if (Size == m_ProcHist.GetSize())
		return(TRUE);
	STOP_ENGINE(*m_Engine);
	m_ProcHist.Create(Size);
	return(TRUE);
}

bool CEngineThread::OnStall(int QueueRetc)
{
	if (QueueRetc == CFrameQueue::TIMEOUT) {
		Stop();
		m_Engine->OnStall(this);
	}
	if (!WaitForStart())
		return(FALSE);
	return(m_IsPausing);
}

bool CEngineThread::WriteToFreeQueue(PFRAME Frame)
{
	QWRITE(m_Engine->GetFreeQueue(), Frame);
	return(TRUE);
}

void CEngineThread::OnRunError(bool Enable)
{
	int	code = Enable ? ENGERR_CANT_START_THREAD : ENGERR_CANT_STOP_THREAD;
	m_Engine->OnError(code, m_Name);
}

bool CEngineThread::WaitForEvent(WEvent& Event, DWORD Timeout)
{
	while (1) {
		HANDLE	ha[] = {m_Engine->GetStopEvent(), Event};
		DWORD	WaitResult = WaitForMultipleObjects(2, ha, FALSE, Timeout);
		int	retc;
		switch (WaitResult) {
		case WAIT_OBJECT_0:
			retc = CFrameQueue::ESCAPED;
			break;
		case WAIT_OBJECT_0 + 1:
			return(TRUE);
		case WAIT_TIMEOUT:
			retc = CFrameQueue::TIMEOUT;
			break;
		default:
			retc = CFrameQueue::WERROR;
			break;
		}
#ifdef ENGINE_NATTER
		SetStallInfo(NULL, 0, retc, NULL);
#endif
		if (!OnStall(retc))
			return(FALSE);
	}
}

BOOL CEngineThread::GetRunTime(HANDLE hThread, ULONGLONG& RunTime)
{
	union {
		ULONGLONG	ull;
		FILETIME	ft;
	} CreationTime, ExitTime, KernelTime, UserTime;
	if (!GetThreadTimes(hThread, 
		&CreationTime.ft, &ExitTime.ft, &KernelTime.ft, &UserTime.ft))
		return(FALSE);
	RunTime = KernelTime.ull + UserTime.ull;
	return(TRUE);
}

BOOL CEngineThread::GetRunDelta(ULONGLONG& RunDelta)
{
	ULONGLONG	t;
	if (!GetRunTime(t))
		return(FALSE);
	RunDelta = t - m_RunTime;
	m_RunTime = t;
	return(TRUE);
}
