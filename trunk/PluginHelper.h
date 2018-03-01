// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13sep10	initial version
		01		03apr11	add dtor
		02		16apr11	add GetOutputFrame
		03		26mar12	replace next helper with next output event
		04		13apr12	standardize thread function
		05		06may12	add AttachHelper and DetachHelper

		plugin helper thread
 
*/

#pragma once

#include "ArrayEx.h"
#include "EngineThread.h"
#include "FFInstance.h"

class CFFPluginEx;

class CPluginHelper : public CEngineThread {
public:
// Construction
	CPluginHelper();
	~CPluginHelper();
	bool	Create(CFFPluginEx *Plugin, int HelperIdx);

// Attributes
	WEvent&	GetIdleEvent();
	WEvent&	GetInputEvent();
	WEvent&	GetOutputEvent();
	int		GetNumInputs() const;
	bool	SetNumInputs(int Inputs);
	volatile PFRAME	GetInputFrame(int InpIdx) const;
	void	SetInputFrame(int InpIdx, PFRAME Frame);
	volatile PFRAME	GetOutputFrame() const;
	WEvent	*GetNextOutputEvent() const;
	void	SetNextOutputEvent(WEvent *Event);
	CFFInstance&	GetFFInstance();

// Operations
	void	ResetState();
	void	DumpState(FILE *fp);
	void	UpdateParms();

protected:
// Types

// Member data
	CFFPluginEx	*m_Plugin;			// pointer to parent plugin
	int		m_HelperIdx;			// index in plugin's array of helpers
	CFrameArray	m_InputFrame;		// array of input frames
	CFrameBufArray	m_InFrameBuf;	// array of pointers to input frame buffers
	PFRAME	m_OutFrame;				// output frame pointer
	WEvent	m_IdleEvent;			// set if we're idle
	WEvent	m_InputEvent;			// set if input is available
	WEvent	m_OutputEvent;			// set if it's our turn to output
	WEvent	*m_NextOutputEvent;		// pointer to next helper's output event
	CFFInstance	m_FFInst;			// freeframe instance
	CArrayEx<float, float&>	m_ParmTarget;	// target value for each parameter
	CArrayEx<float, float&>	m_ParmShadow;	// shadow value for each parameter

// Overridables

// Helpers
	bool	Work();
	static	UINT	ThreadFunc(LPVOID Arg);
};

typedef CArrayEx<CPluginHelper, CPluginHelper&> CPluginHelperArray;

inline WEvent& CPluginHelper::GetIdleEvent()
{
	return(m_IdleEvent);
}

inline WEvent& CPluginHelper::GetInputEvent()
{
	return(m_InputEvent);
}

inline WEvent& CPluginHelper::GetOutputEvent()
{
	return(m_OutputEvent);
}

inline int CPluginHelper::GetNumInputs() const
{
	return(m_InputFrame.GetSize());
}

inline volatile PFRAME CPluginHelper::GetInputFrame(int InpIdx) const
{
	return(m_InputFrame[InpIdx]);
}

inline void CPluginHelper::SetInputFrame(int InpIdx, PFRAME Frame)
{
	m_InputFrame[InpIdx] = Frame;
}

inline volatile PFRAME CPluginHelper::GetOutputFrame() const
{
	return(m_OutFrame);
}

inline WEvent *CPluginHelper::GetNextOutputEvent() const
{
	return(m_NextOutputEvent);
}

inline void CPluginHelper::SetNextOutputEvent(WEvent *Event)
{
	m_NextOutputEvent = Event;
}

inline CFFInstance& CPluginHelper::GetFFInstance()
{
	return(m_FFInst);
}
