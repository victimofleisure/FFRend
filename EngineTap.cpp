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

#include "stdafx.h"
#include "FFRend.h"
#include "EngineTap.h"

CEngineTap::CEngineTap()
{
	m_OutputQueue = NULL;
	m_Plugin = NULL;
	m_PausePlugin = NULL;
	m_PluginUID = 0;
	m_IsBypassed = FALSE;
	m_IsPaused = FALSE;
}

void CEngineTap::SetPlugin(CFFPluginEx *Plugin)
{
	m_Plugin = Plugin;
	if (Plugin != NULL)
		m_PluginUID = Plugin->GetUID();
	else
		m_PluginUID = 0;
}

LPCTSTR CEngineTap::GetPluginName() const
{
	if (IsAttached())
		return(m_Plugin->GetName());
	return(_T(""));
}

void CEngineTap::RunInit()
{
	m_IsPaused = FALSE;	// engine restart resets paused state
	if (IsAttached() && !m_IsBypassed)	// if attached and not bypassed
		m_Plugin->ConnectOutput(*m_OutputQueue);	// connect to plugin's output
}

bool CEngineTap::Pause(bool Enable)
{
	if (Enable == m_IsPaused)	// if already in requested state
		return(TRUE);	// nothing to do
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::Pause %d\n"), Enable);
#endif
	m_IsPaused = Enable;	// update state first to fail safe
	if (Enable) {	// if pausing
		if (!m_IsBypassed)	// if not bypassed
			m_PausePlugin = m_Plugin;	// save connected plugin if any
		else	// bypassed
			m_PausePlugin = NULL;	// always disconnected while bypassed
	} else {	// resuming
		// if attachment state changed during pause, or we're bypassed
		if (m_PausePlugin != m_Plugin || m_IsBypassed) {
			// connect and disconnect are deferred during pause
			if (m_PausePlugin != NULL) {	// if connected plugin at start of pause
				if (!Disconnect(*m_PausePlugin))	// do deferred disconnect
					return(FALSE);
			}
			if (IsAttached() && !m_IsBypassed) {	// if attached and not bypassed
				if (!Connect(*m_Plugin))	// do deferred connect
					return(FALSE);
			}
		}
	}
	return(TRUE);
}

bool CEngineTap::Bypass(bool Enable)
{
	if (Enable == m_IsBypassed)	// if already in requested state
		return(TRUE);	// nothing to do
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::Bypass %d\n"), Enable);
#endif
	m_IsBypassed = Enable;	// update state first to fail safe
	if (IsAttached()) {	// if attached to plugin
		if (Enable) {	// if entering bypass
			if (!Disconnect(*m_Plugin))	// disconnect from plugin
				return(FALSE);
		} else {	// exiting bypass
			if (!Connect(*m_Plugin))	// connect to plugin
				return(FALSE);
		}
	}
	return(TRUE);
}

bool CEngineTap::SuspendOutput(CFFPluginEx& Plugin, bool Enable)
{
	int	helpers = Plugin.GetHelperCount();
	if (helpers) {	// if plugin has helpers
		if (Enable) {	// if suspending output
			if (!m_OutputEvent.Create(NULL, FALSE, FALSE, NULL)) {
				AfxMessageBox(ENGERR_CANT_CREATE_EVENT);
				return(FALSE);
			}
			// interrupt helper output token ring and divert output signal to us,
			// by unlinking helper 0's next output event pointer from helper 1's
			// output event and linking it to our own event instead; possession
			// of token guarantees no helper is in or can enter an output loop
			Plugin.GetHelper(0).SetNextOutputEvent(&m_OutputEvent);
			// timeout must allow for worst case: if helper 0 just set helper 1's
			// output event, signal traverses entire ring before diverting to us
			UINT	Timeout = Plugin.GetTimeout() * helpers;
			if (WaitForSingleObject(m_OutputEvent, Timeout) != WAIT_OBJECT_0)
				return(FALSE);
		} else {	// resuming output
			// repair damage done to helper output token ring, by relinking
			// helper 0's output event pointer to helper 1's output event
			WEvent	*NextOutputEvent = &Plugin.GetHelper(1).GetOutputEvent();
			Plugin.GetHelper(0).SetNextOutputEvent(NextOutputEvent);
			NextOutputEvent->Set();	// kick-start to get token ring going again
			m_OutputEvent.Close();
		}
	} else {	// plugin doesn't have helpers
		if (Enable) {	// if suspending output
			if (!Plugin.StopWork())	// stop plugin's worker
				return(FALSE);
		} else {	// resuming output
			CEngineThread	*pThr = &Plugin;
			if (!pThr->Run(TRUE))	// restart plugin's worker
				return(FALSE);
		}
	}
	return(TRUE);
}

bool CEngineTap::Connect(CFFPluginEx& Plugin)
{
	if (!Plugin.IsRunning())	// if plugin is stopped (can't render)
		return(TRUE);	// can't connect, but it doesn't matter
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::Connect to %s\n"), Plugin.GetName());
#endif
	if (!SuspendOutput(Plugin, TRUE))	// suspend plugin's output
		return(FALSE);
	Plugin.ConnectOutput(*m_OutputQueue);	// connect to plugin
	if (!SuspendOutput(Plugin, FALSE))	// resume plugin's output
		return(FALSE);
	return(TRUE);
}

bool CEngineTap::Disconnect(CFFPluginEx& Plugin)
{
	if (!Plugin.IsRunning())	// if plugin is stopped (can't render)
		return(TRUE);	// can't disconnect, but it doesn't matter
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::Disconnect from %s\n"), Plugin.GetName());
#endif
	if (!SuspendOutput(Plugin, TRUE))	// suspend plugin's output
		return(FALSE);
	if (!Plugin.DisconnectOutput(*m_OutputQueue))	// disconnect from plugin
		return(FALSE);
	if (!SuspendOutput(Plugin, FALSE))	// resume plugin's output
		return(FALSE);
	theApp.GetEngine().FlushQueue(*m_OutputQueue);	// flush output queue
	return(TRUE);
}

bool CEngineTap::Attach(CFFPluginEx& Plugin)
{
	if (&Plugin == m_Plugin)	// if already in requested state
		return(TRUE);	// nothing to do
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::Attach to %s\n"), Plugin.GetName());
#endif
	CFFEngine&	Engine = theApp.GetEngine();
	if (!(m_IsBypassed || Engine.IsPaused())) {	// if not bypassed or paused
		if (IsAttached()) {	// if already attached to a different plugin
			if (!Detach())	// detach from that plugin first
				return(FALSE);
		}
		if (!Connect(Plugin))	// connect to plugin
			return(FALSE);
	}
	// we're attached
	m_Plugin = &Plugin;
	m_PluginUID = Plugin.GetUID();
	return(TRUE);
}

bool CEngineTap::Detach()
{
	if (!IsAttached())	// if already in requested state
		return(TRUE);	// nothing to do
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::Detach from %s\n"), m_Plugin->GetName());
#endif
	CFFEngine&	Engine = theApp.GetEngine();
	if (!(m_IsBypassed || Engine.IsPaused())) {	// if not bypassed or paused
		if (!Disconnect(*m_Plugin))	// disconnect from plugin
			return(FALSE);
	}
	// we're detached
	m_Plugin = NULL;
	m_PluginUID = 0;
	return(TRUE);
}

int CEngineTap::OnEndSlotChange()
{
	CFFEngine&	Engine = theApp.GetEngine();
	ASSERT(!Engine.IsRunning());	// engine must be stopped
	ASSERT(!Engine.InSlotChange());	// must be called after slot change
	if (!IsAttached())
		return(-1);
	int	PlugIdx = Engine.FindPluginByUID(m_PluginUID);
#ifdef ENGINE_TAP_NATTER
	_tprintf(_T("CEngineTap::OnEndSlotChange PlugIdx=%d\n"), PlugIdx);
#endif
	if (PlugIdx < 0) {	// if tapped plugin not found
		// plugin was presumably deleted, making m_Plugin an invalid pointer
		m_Plugin = NULL;	// force detached state without accessing m_Plugin
		m_PluginUID = 0;
		return(-1);
	}
	int	SlotIdx = Engine.PluginToSlot(PlugIdx);	// convert to slot index
	return(SlotIdx);	// index of slot in which tapped plugin was found
}
