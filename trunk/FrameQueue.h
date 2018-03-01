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
		03		05mar03		use critical section wrapper object
		04		15aug04		ck: wrap events, threads, and timers
		05		31may08		ck: add Create and Destroy
		06		14mar10		specialize for frames
		07		03may11		add SetSize
		08		24apr12		store escape event as handle

		thread-safe frame queue class

*/

#pragma once

#include "Event.h"
#include "CritSec.h"
#include "EngineTypes.h"

class CFrameQueue : public WObject {
public:
// Constants
	enum {	// read/write return codes
		SUCCESS,
		TIMEOUT,
		ESCAPED,
		WERROR,
		RETURN_CODES
	};

// Construction
	CFrameQueue();
	~CFrameQueue();
	bool	Create(DWORD Size, WEvent& EscapeEvent, DWORD Timeout);
	void	Destroy();

// Attributes
	DWORD	GetSize() const;
	bool	SetSize(DWORD Size);
	volatile DWORD	GetCount() const;
	DWORD	GetTimeout() const;
	void	SetTimeout(DWORD Timeout);

// Operations
	int		Write(PFRAME Frame);
	int		Read(PFRAME& Frame);
	void	Flush();
	WEvent&	WriteEvent();
	WEvent&	ReadEvent();
	void	DumpState(FILE *fp);
	void	DumpState(CFrameArray& Frame);

protected:
// Member data
	PFRAME	*m_Start;			// points to first slot
	PFRAME	*m_End;				// points to last slot
	PFRAME	*m_Head;			// the next slot to write to
	PFRAME	*m_Tail;			// the next slot to read from
	DWORD	m_Size;				// number of slots
	DWORD	m_Items;			// number of slots in use
	WEvent	m_WriteEvent;		// writers block here on full queue
	WEvent	m_ReadEvent;		// readers block here on empty queue
	WCritSec	m_CritSec;		// gates access to queue
	WHANDLE	m_EscapeEvent;		// set to escape blocked read/write
	DWORD	m_Timeout;			// read/write timeout in milliseconds

// Helpers
	void	Reset();
};

inline WEvent& CFrameQueue::WriteEvent()
{
	return(m_WriteEvent);
}

inline WEvent& CFrameQueue::ReadEvent()
{
	return(m_ReadEvent);
}

inline DWORD CFrameQueue::GetSize() const
{
	return(m_Size);
}

inline volatile DWORD CFrameQueue::GetCount() const
{
	return(m_Items);
}

inline DWORD CFrameQueue::GetTimeout() const
{
	return(m_Timeout);
}

inline void CFrameQueue::SetTimeout(DWORD Timeout)
{
	m_Timeout = Timeout;
}
