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
		04		13apr12	add init/cleanup macros
		05		31may12	add Output method

		parallel engine thread base class
 
*/

#pragma once

#include "RefPtr.h"
#include "EngineTypes.h"
#include "WorkerThread.h"
#include "FrameQueue.h"

class CEngine;

class CEngineThread : public CRefObj, public CWorkerThread {
public:
// Construction
	CEngineThread();

// Constants
	enum {
		QF_READ,
		QF_WRITE,
		QUEUE_FUNCTIONS
	};

// Types
	class CThreadState : public WObject {
	public:
		~CThreadState();
	};

// Attributes
	LPCTSTR	GetName() const;
	void	SetName(LPCTSTR Name);
	void	GetStallInfo(STALL_INFO& Info) const;
	const STALL_INFO&	GetStallInfo() const;
	UINT	GetFrameCounter() const;
	void	SetFrameCounter(UINT Count);
	bool	SetHistorySize(UINT Size);
	CProcessHistory&	GetProcessHistory();
	bool	IsProcessing() const;
	void	SetProcessing(bool Enable);
	BOOL	GetRunTime(ULONGLONG& RunTime) const;
	BOOL	GetRunDelta(ULONGLONG& RunDelta);

// Operations
	bool	Run(bool Enable);
	bool	Pause(bool Enable);
	bool	Read(CFrameQueue *Queue, PFRAME& Frame);
	bool	Write(CFrameQueue *Queue, PFRAME Frame);
	bool	Output(CFrameQueue *Queue, PFRAME Frame);
	bool	WriteToFreeQueue(PFRAME Frame);
	void	AddProcessHistorySample(bool Enable);
	bool	WaitForEvent(WEvent& Event, DWORD Timeout);
	static	BOOL	GetRunTime(HANDLE hThread, ULONGLONG& RunTime);

protected:
// Member data
	CEngine	*m_Engine;			// pointer to engine
	CString	m_Name;				// thread name
	STALL_INFO	m_StallInfo;	// most recent stall state
	CProcessHistory	m_ProcHist;	// processing history ring buffer
	bool	m_IsProcessing;		// true if processing a frame
	bool	m_IsPausing;		// true if thread is pausing
	UINT	m_FrameCounter;		// position in frame sequence
	ULONGLONG	m_RunTime;		// execution time, in 100-nanosecond units

// Helpers
	void	ThreadInit();
	void	ThreadCleanup();
	bool	OnStall(int QueueRetc);
	void	SetStallInfo(CFrameQueue *Queue, int QueueFunc, int QueueRetc, PFRAME Frame);
	void	ResetStallInfo();
	bool	RunWorker(bool Enable);
	void	OnRunError(bool Enable);
	void	EngineThreadFunc();
};

inline void CEngineThread::ThreadCleanup()
{
#if ENGINE_USES_COM
	CoUninitialize();
#endif
}

inline LPCTSTR CEngineThread::GetName() const
{
	return(m_Name);
}

inline void CEngineThread::SetName(LPCTSTR Name)
{
	m_Name = Name;
}

inline void CEngineThread::GetStallInfo(STALL_INFO& Info) const
{
	Info = m_StallInfo;
}

inline const STALL_INFO& CEngineThread::GetStallInfo() const
{
	return(m_StallInfo);
}

inline void CEngineThread::SetStallInfo(CFrameQueue *Queue, int QueueFunc, int QueueRetc, PFRAME Frame)
{
	m_StallInfo.Queue = Queue;
	m_StallInfo.QueueFunc = QueueFunc;
	m_StallInfo.QueueRetc = QueueRetc;
	m_StallInfo.Frame = Frame;
}

inline void CEngineThread::ResetStallInfo()
{
	ZeroMemory(&m_StallInfo, sizeof(STALL_INFO));
}

inline UINT CEngineThread::GetFrameCounter() const
{
	return(m_FrameCounter);
}

inline void CEngineThread::SetFrameCounter(UINT Count)
{
	m_FrameCounter = Count;
}

inline CProcessHistory& CEngineThread::GetProcessHistory()
{
	return(m_ProcHist);
}

inline void CEngineThread::AddProcessHistorySample(bool Enable)
{
	PROCESS_HISTORY_SAMPLE	samp;
	samp.Time = GetTickCount();
	samp.FrameCount = static_cast<WORD>(m_FrameCounter);
	samp.Enable = Enable;
	m_ProcHist.PushOver(samp);
}

inline void CEngineThread::SetProcessing(bool Enable)
{
	m_IsProcessing = Enable;
}

inline bool CEngineThread::IsProcessing() const
{
	return(m_IsProcessing);
}

inline BOOL CEngineThread::GetRunTime(ULONGLONG& RunTime) const
{
	return(GetRunTime(m_Thread->m_hThread, RunTime));
}

inline bool CEngineThread::RunWorker(bool Enable)
{
#ifdef ENGINE_NATTER
	if (Enable)
		ResetStallInfo();
#endif
	if (!CWorkerThread::Run(Enable)) {
		OnRunError(Enable);
		return(FALSE);
	}
	return(TRUE);
}

inline bool CEngineThread::Run(bool Enable)
{
	m_IsPausing = FALSE;
	return(RunWorker(Enable));
}

inline bool CEngineThread::Pause(bool Enable)
{
	m_IsPausing = TRUE;
	return(RunWorker(!Enable));
}

inline bool CEngineThread::Read(CFrameQueue *Queue, PFRAME& Frame)
{
	while (1) {
		int	retc = Queue->Read(Frame);
		if (retc == CFrameQueue::SUCCESS)
			return(TRUE);
#ifdef ENGINE_NATTER
		SetStallInfo(Queue, QF_READ, retc, Frame);
#endif
		if (!OnStall(retc))
			return(FALSE);
	}
}

inline bool CEngineThread::Write(CFrameQueue *Queue, PFRAME Frame)
{
	while (1) {
		int	retc = Queue->Write(Frame);
		if (retc == CFrameQueue::SUCCESS)
			return(TRUE);
#ifdef ENGINE_NATTER
		SetStallInfo(Queue, QF_WRITE, retc, Frame);
#endif
		if (!OnStall(retc))
			return(FALSE);
	}
}

inline bool CEngineThread::Output(CFrameQueue *Queue, PFRAME Frame)
{
	while (1) {
		int	retc = Queue->Write(Frame);
		if (retc == CFrameQueue::SUCCESS)
			return(TRUE);
		// if frame write timed out and destination queue has a zero timeout
		if (retc == CFrameQueue::TIMEOUT && !Queue->GetTimeout()) {
			// skipping output: subtract a reference from frame to compensate
			if (!InterlockedDecrement(&Frame->RefCount)) {	// if no more refs
				if (!WriteToFreeQueue(Frame))	// return frame to free queue
					return(FALSE);
			}
			return(TRUE);
		}
#ifdef ENGINE_NATTER
		SetStallInfo(Queue, QF_WRITE, retc, Frame);
#endif
		if (!OnStall(retc))
			return(FALSE);
	}
}

#define QREAD(queue, frame)	{	\
	if (!Read(queue, frame))	\
		return(!m_KillFlag);	\
}

#define QWRITE(queue, frame) {	\
	if (!Write(queue, frame))	\
		return(!m_KillFlag);	\
}

#define QOUTPUT(queue, frame) {	\
	if (!Output(queue, frame))	\
		return(!m_KillFlag);	\
}

#define ENGINE_THREAD_INIT(T)	\
	T	*pThread = (T *)Arg;	\
	pThread->ThreadInit();

#define ENGINE_THREAD_EXIT()	\
	pThread->ThreadCleanup();	\
	return(0);
