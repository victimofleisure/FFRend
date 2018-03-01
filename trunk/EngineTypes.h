// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      14sep10	move CFrameBufArray here

		parallel plugin engine types
 
*/

#pragma once

#include "ArrayEx.h"
#include "RingBuf.h"

class CFrameQueue;

typedef struct tagFRAME_HEADER {
	long	RefCount;	// reference count
	DWORD	Idx;		// index in frame array
} FRAME_HEADER;

typedef struct tagFRAME : FRAME_HEADER {
	char	Buf[1];		// frame buffer
} FRAME, *PFRAME;

typedef CArrayEx<PFRAME, PFRAME&> CFrameArray;
typedef CArrayEx<PVOID, PVOID> CFrameBufArray;

typedef struct tagSTALL_INFO {
	CFrameQueue	*Queue;	// pointer to queue 
	int		QueueFunc;	// queue function
	int		QueueRetc;	// queue return code
	PFRAME	Frame;		// frame pointer (non-null for writes only)
} STALL_INFO;

typedef struct tagPROCESS_HISTORY_SAMPLE {
	DWORD	Time;			// time in milliseconds
	WORD	Enable;			// non-zero if processing
	WORD	FrameCount;		// time in frames; wraps at 64K
} PROCESS_HISTORY_SAMPLE;

typedef CRingBuf<PROCESS_HISTORY_SAMPLE> CProcessHistory;

// enumerate errors
#define ENGERR_DEF(x) x,
enum {
	#include "EngineErrs.h"
	ENGINE_ERRORS
};
