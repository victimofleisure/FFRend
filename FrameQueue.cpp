// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		21feb00		initial version
		01		31may00		begin revision history
		02		08may01		allow derived classes
		03		19nov02		write event must be initially signalled
		04		05mar03		use critical section wrapper object
		05		15aug04		ck: wrap events, threads, and timers
		06		18oct04		ck: set critsec debug name in ctor
		07		31may08		ck: add Create and Destroy
		08		14mar10		specialize for frames
		09		26apr10		add frame pointer assertions
		10		03may11		add SetSize
		11		24apr12		store escape event as handle

		thread-safe frame queue class

*/

#include "StdAfx.h"
#include "FrameQueue.h"

CFrameQueue::CFrameQueue()
{
	Reset();
}

CFrameQueue::~CFrameQueue()
{
	Destroy();
}

void CFrameQueue::Reset()
{
	m_Start = NULL;
	m_End = NULL;
	m_Head = NULL;
	m_Tail = NULL;
	m_Size = 0;
	m_Items = 0;
	m_EscapeEvent = NULL;
	m_Timeout = INFINITE;
}

bool CFrameQueue::Create(DWORD Size, WEvent& EscapeEvent, DWORD Timeout)
{
	ASSERT(EscapeEvent != NULL);
	Destroy();	// in case instance is being reused
	m_WriteEvent.Create(NULL, FALSE, TRUE, NULL); // auto reset
	m_ReadEvent.Create(NULL, TRUE, FALSE, NULL);	// manual reset
	if (m_WriteEvent == NULL || m_ReadEvent == NULL)
		return(FALSE);
	m_EscapeEvent = EscapeEvent;
	m_Timeout = Timeout;
	return(SetSize(Size));
}

bool CFrameQueue::SetSize(DWORD Size)
{
	if (Size != m_Size) {	// if size changed
		delete [] m_Start;
		m_Start = new PFRAME[Size];
		if (m_Start == NULL)
			return(FALSE);
		m_Size = Size;
	}
	m_End = m_Start + Size;
	m_Head = m_Tail = m_Start;
	m_Items = 0;
	return(TRUE);		
}

void CFrameQueue::Destroy()
{
	delete [] m_Start;
	Reset();
}

int CFrameQueue::Write(PFRAME Frame)
{
	ASSERT(Frame != NULL);
//
// If the queue is full, block until there's a free slot or the timeout expires.
//
	while (1) {
		m_CritSec.Enter();		// enter critical section
		if (m_Items < m_Size)	// if there's a free slot, go write to it
			break;
		m_CritSec.Leave();		// leave critical section before blocking
		HANDLE	ha[2] = {m_EscapeEvent, m_WriteEvent};	// escape has priority
		DWORD	retc = WaitForMultipleObjects(2, ha, FALSE, m_Timeout);
		switch (retc) {
		case WAIT_OBJECT_0:		// escape event
			return(ESCAPED);
		case WAIT_OBJECT_0 + 1:	// write event, go to top of loop
			break;
		case WAIT_TIMEOUT:		// timeout
			return(TIMEOUT);
		default:				// wait error
			return(WERROR);
		}
	}
//
// Write the entry to the queue, and wake any blocked readers.
//
	*m_Head++ = Frame;
	if (m_Head == m_End)
		m_Head = m_Start;
	m_Items++;
	m_ReadEvent.Set();	// wake any blocked readers
	m_CritSec.Leave();	// leave critical section
	return(SUCCESS);
}

int CFrameQueue::Read(PFRAME& Frame)
{
//
// If the queue is empty, block until there's an entry or the timeout expires.
//
	while (1) {
		m_CritSec.Enter();		// enter critical section
		if (m_Items)			// if there's an item, go read it
			break;
		m_CritSec.Leave();		// leave critical section before blocking
		HANDLE	ha[2] = {m_EscapeEvent, m_ReadEvent};	// escape has priority
		DWORD	retc = WaitForMultipleObjects(2, ha, FALSE, m_Timeout);
		switch (retc) {
		case WAIT_OBJECT_0:		// escape event
			Frame = NULL;
			return(ESCAPED);
		case WAIT_OBJECT_0 + 1:	// read event, go to top of loop
			break;
		case WAIT_TIMEOUT:		// timeout
			Frame = NULL;
			return(TIMEOUT);
		default:				// wait error
			Frame = NULL;
			return(WERROR);
		}
	}
//
// Read the entry from the queue, and wake any blocked writers.
//
	Frame = *m_Tail++;
	ASSERT(Frame != NULL);
	if (m_Tail == m_End)
		m_Tail = m_Start;
	m_Items--;
	if (!m_Items)	// if no more items
		m_ReadEvent.Reset();	// reset read event
	m_WriteEvent.Set();	// wake any blocked writers
	m_CritSec.Leave();	// leave critical section
	return(SUCCESS);
}

void CFrameQueue::Flush()
{
	WCritSec::Lock	lk(m_CritSec);
	m_Items = 0;
	m_Head = m_Tail = m_Start;
	m_ReadEvent.Reset();
	m_WriteEvent.Set();
}

void CFrameQueue::DumpState(CFrameArray& Frame)
{
	WCritSec::Lock	lk(m_CritSec);
	Frame.SetSize(m_Items);
	PFRAME	*pFrame = m_Tail;
	for (UINT i = 0; i < m_Items; i++) {
		Frame[i] = *pFrame;
		pFrame++;
		if (pFrame == m_End)
			pFrame = m_Start;
	}
}

void CFrameQueue::DumpState(FILE *fp)
{
	ASSERT(fp != NULL);
	WCritSec::Lock	lk(m_CritSec);
	if (m_Items) {
		const PFRAME *frame = m_Tail;
		for (UINT i = 0; i < m_Items; i++) {
			if (i)
				_fputts(_T(", "), fp);
			_ftprintf(fp, _T("%d(%d)"), (*frame)->Idx, (*frame)->RefCount);
			frame++;
			if (frame == m_End)
				frame = m_Start;
		}
	}
	_fputtc('\n', fp);
}
