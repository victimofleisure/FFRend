// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      12sep10	remove stop event

		worker thread with run/stop support
 
*/

#include "stdafx.h"
#include "WorkerThread.h"

CWorkerThread::CWorkerThread()
{
	m_Thread = NULL;
	m_Timeout = 0;
	m_StopFlag = FALSE;
	m_KillFlag = FALSE;
	m_IsRunning = FALSE;
}

CWorkerThread::~CWorkerThread()
{
	Destroy();
}

bool CWorkerThread::Create(PTHREADFUNC ThreadFunc, LPVOID ThreadArg, int Priority, UINT StackSize, UINT Timeout)
{
	m_Timeout = Timeout;
	if (!(m_Start.Create(NULL, TRUE, FALSE, NULL)	// all manual
	&& m_Started.Create(NULL, TRUE, FALSE, NULL)
	&& m_Stopped.Create(NULL, TRUE, FALSE, NULL))) {
		return(FALSE);
	}
	m_StopFlag = TRUE;
	m_KillFlag = FALSE;
	m_Thread = AfxBeginThread(ThreadFunc, ThreadArg, Priority, StackSize, CREATE_SUSPENDED, NULL);
	if (m_Thread == NULL)
		return(FALSE);
	m_Thread->m_bAutoDelete = FALSE;
	m_Thread->ResumeThread();
	if (WaitForSingleObject(m_Stopped, Timeout) != WAIT_OBJECT_0)
		return(FALSE);
	return(TRUE);
}

bool CWorkerThread::Destroy()
{
	if (m_Thread == NULL)
		return(TRUE);
	Run(FALSE);
	m_KillFlag = TRUE;
	Run(TRUE);
	if (WaitForSingleObject(m_Thread->m_hThread, m_Timeout) != WAIT_OBJECT_0)
		return(FALSE);
	delete m_Thread;
	m_Thread = NULL;
	m_IsRunning = FALSE;
	return(TRUE);
}

bool CWorkerThread::Run(bool Enable)
{
	if (Enable == m_IsRunning)
		return(TRUE);	// nothing to do
	if (Enable) {
		m_StopFlag = FALSE;
		m_Start.Set();
		if (WaitForSingleObject(m_Started, m_Timeout) != WAIT_OBJECT_0)
			return(FALSE);
		m_Start.Reset();
	} else {
		m_StopFlag = TRUE;
		if (WaitForSingleObject(m_Stopped, m_Timeout) != WAIT_OBJECT_0)
			return(FALSE);
	}
	m_IsRunning = Enable;
	return(TRUE);
}

bool CWorkerThread::WaitForStart()
{
	m_Started.Reset();
	m_Stopped.Set();
	WaitForSingleObject(m_Start, INFINITE);
	m_Stopped.Reset();
	m_Started.Set();
	return(!m_KillFlag);
}

#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD	dwType;		// Must be 0x1000.
	LPCSTR	szName;		// Pointer to name (in user addr space).
	DWORD	dwThreadID; // Thread ID (-1=caller thread).
	DWORD	dwFlags;	// Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, LPCTSTR threadName)
{
#ifdef UNICODE
	USES_CONVERSION;
	LPCSTR	ANSIName = W2A(threadName);	// convert name from Unicode to ANSI
#else
	LPCSTR	ANSIName = threadName;
#endif
	Sleep(10);
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = ANSIName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG*), (ULONG*)&info);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
