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
		02		17mar12	remove IsForcingStall, add current frame
		03		13apr12	standardize thread function

        engine with recording support
 
*/

#pragma once

#include "EngineThread.h"
#include "FrameQueue.h"
#include "BmpToAvi.h"
#include "Dib.h"
#include <ddraw.h>
#include "RecordInfo.h"

class CFFEngine;

class CRecorder : public CEngineThread {
public:
// Construction
	CRecorder(CFFEngine *Engine);
	void	Destroy();

// Attributes
	CFFEngine	*GetEngine();
	bool	IsRecording() const;
	CFrameQueue *GetInputQueue();
	const CRecordInfo&	GetInfo() const;
	bool	IsDone() const;
	void	GetComprState(CVideoComprState& ComprState) const;

// Operations
	bool	RunInit();
	bool	Run(bool Enable);
	bool	Pause(bool Enable);
	bool	Start(LPCTSTR Path, const CRecordInfo& Info, const CVideoComprState *ComprState = NULL);
	bool	Stop();
	bool	Open(LPCTSTR Path, const CRecordInfo& Info, const CVideoComprState *ComprState = NULL);
	void	Close();
	void	DumpState(FILE *fp);

// Implementation
protected:
// Constants
	enum {
		NO_PLUGINS_PAUSE = 100
	};

// Data members
	bool	m_IsRecording;		// true if recording
	bool	m_IsHooking;		// true if hooking output
	bool	m_IsEnding;			// true if end record message was posted
	CFrameQueue	m_InputQueue;	// record input frame pointer queue
	CBmpToAvi	m_RecAvi;		// records sequential bitmaps to AVI file
	CDib	m_OutDib;			// output frame as a device-independent bitmap
	CDC		m_OutDC;			// device context for blitting to output frame
	HGDIOBJ	m_PrevBmp;			// device context's previous bitmap
	IDirectDrawSurface7	*m_FrameSurf;	// off-screen surface for input frame
	DDSURFACEDESC2	m_SurfDesc;	// description of input frame surface
	CRecordInfo	m_RecInfo;		// record information
	CVideoComprState	m_ComprState;	// video compressor state
	PFRAME	m_Frame;			// pointer to input frame in process, if any

// Helpers
	bool	HookOutput();
	bool	UnhookOutput();
	bool	Work();
	bool	ReplaceOutput(CFrameQueue& Target, CFrameQueue& Substitute);
	static	UINT	ThreadFunc(LPVOID Arg);
};

inline CFFEngine *CRecorder::GetEngine()
{
	return((CFFEngine *)m_Engine);
}

inline bool CRecorder::IsRecording() const
{
	return(m_IsRecording);
}

inline CFrameQueue *CRecorder::GetInputQueue()
{
	return(&m_InputQueue);
}

inline const CRecordInfo& CRecorder::GetInfo() const
{
	return(m_RecInfo);
}

inline bool CRecorder::IsDone() const
{
	return(!m_RecInfo.m_Unlimited && m_FrameCounter >= UINT(m_RecInfo.m_FrameCount));
}

inline void CRecorder::GetComprState(CVideoComprState& ComprState) const
{
	ComprState = m_ComprState;
}
