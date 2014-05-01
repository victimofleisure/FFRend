// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26mar12	initial version
		01		13apr12	standardize thread function
		02		13apr12	add OnEndSlotChange
		03		23may12	add dtor, output queue pointer
		04		31may12	remove worker thread
		05		01jun12	refactor bypass and pause

		parallel engine plugin output tap
 
*/

#pragma once

#include "FrameQueue.h"

class CFFEngine;
class CFFPluginEx;

class CEngineTap : public WObject {
public:
// Construction
	CEngineTap();

// Attributes
	bool	IsAttached() const;
	bool	IsBypassed() const;
	CFFPluginEx	*GetPlugin();
	const CFFPluginEx	*GetPlugin() const;
	void	SetPlugin(CFFPluginEx *Plugin);
	CFrameQueue	*GetOutputQueue();
	void	SetOutputQueue(CFrameQueue *Queue);
	LPCTSTR	GetPluginName() const;

// Operations
	bool	Attach(CFFPluginEx& Plugin);
	bool	Detach();
	bool	Bypass(bool Enable);
	void	RunInit();
	bool	Pause(bool Enable);
	int		OnEndSlotChange();

protected:
// Data members
	CFrameQueue	*m_OutputQueue;		// pointer to output frame queue
	CFFPluginEx	*m_Plugin;			// pointer to attached plugin
	CFFPluginEx	*m_PausePlugin;		// pointer to connected plugin at start of pause
	UINT	m_PluginUID;			// tapped plugin's unique ID
	WEvent	m_OutputEvent;			// event for diverting helper output token ring
	bool	m_IsBypassed;			// true if bypassed
	bool	m_IsPaused;				// true if paused

// Helpers
	bool	Work();
	bool	Connect(CFFPluginEx& Plugin);
	bool	Disconnect(CFFPluginEx& Plugin);
	bool	SuspendOutput(CFFPluginEx& Plugin, bool Enable);
	static	UINT	ThreadFunc(LPVOID Arg);
};

inline bool CEngineTap::IsAttached() const
{
	return(m_Plugin != NULL);
}

inline bool CEngineTap::IsBypassed() const
{
	return(m_IsBypassed);
}

inline CFFPluginEx *CEngineTap::GetPlugin()
{
	return(m_Plugin);
}

inline const CFFPluginEx *CEngineTap::GetPlugin() const
{
	return(m_Plugin);
}

inline CFrameQueue *CEngineTap::GetOutputQueue()
{
	return(m_OutputQueue);
}

inline void CEngineTap::SetOutputQueue(CFrameQueue *Queue)
{
	m_OutputQueue = Queue;
}
