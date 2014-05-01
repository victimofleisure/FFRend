// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		06may10	initial version
		01		24jun10	override OnStall to ignore recorder forced stall
		02		17mar12	remove OnStall override
		03		26mar12 add monitor source
		04		13apr12	remove monitor UID
		05		23may12	make monitor source a slot index
		06		01jun12	refactor monitor bypass and pause

        main frame engine
 
*/

#pragma once

#include "ClipEngine.h"
#include "EngineTap.h"

class CMainFrame;
class CUndoManager;

class CMainEngine : public CClipEngine {
public:
// Construction
	CMainEngine(CRenderer& Renderer);
	bool	Create(CMainFrame *Main);

// Overrides
	virtual	bool	Run(bool Enable);
	virtual	bool	RunInit();
	virtual	bool	Pause(bool Enable);
	virtual	void	OnError(int ErrorCode, LPCTSTR Context = NULL);
	virtual	void	DumpState(FILE *fp);
	virtual	CString	GetQueueName(const CFrameQueue *Queue, bool Scope = TRUE) const;
	virtual	void	SetCurSel(int SlotIdx);
	virtual	void	Bypass(int SlotIdx, bool Enable);

// Attributes
	int		GetMonitorSource() const;
	bool	SetMonitorSource(int SlotIdx);
	bool	MonitoringOutput() const;
	bool	SetMonitorBypass(bool Enable);
	int		MonitorSlotToPlugin(int SlotIdx);
	int		MonitorPluginToSlot(int PlugIdx);

// Operations
	bool	OpenClip(int SlotIdx, LPCTSTR Path);
	bool	Load(int SlotIdx, LPCTSTR Path);
	bool	Load(int SlotIdx, const CFFPlugInfo& Info);

protected:
// Overrides
	virtual	void	OnBeginSlotChange();
	virtual	void	OnExtendSlotChange();
	virtual	void	OnEndSlotChange();
	virtual	void	PostSlotChange();

// Constants
	static const int m_EngineErrorID[FFENGINE_ERRORS];

// Data members
	CMainFrame	*m_Main;		// pointer to main frame
	CUndoManager	*m_UndoMgr;	// pointer to undo manager
	CEngineTap	m_MonitorTap;	// frame tap for output monitoring
	int		m_MonitorSource;	// monitored plugin's slot index, or -1 for default

// Helpers
	bool	OpenClipUndoable(int SlotIdx, LPCTSTR Path);
	bool	InsertClipPlayerUndoable(int SlotIdx, LPCTSTR Path);
	bool	LoadClipPlayerUndoable(int SlotIdx, LPCTSTR Path);
	static	short	USLOT(int SlotIdx);
	void	NotifyUndoableEdit(int SlotIdx, WORD Code, UINT Flags = 0);
	void	CancelUndoableEdit(int SlotIdx, WORD Code);
};

inline int CMainEngine::GetMonitorSource() const
{
	return(m_MonitorSource);
}

inline bool CMainEngine::MonitoringOutput() const
{
	return(m_MonitorSource < 0);
}

inline bool CMainEngine::SetMonitorBypass(bool Enable)
{
	return(m_MonitorTap.Bypass(Enable));
}

inline int CMainEngine::MonitorSlotToPlugin(int SlotIdx)
{
	if (SlotIdx < 0)
		return(-1);
	return(SlotToPlugin(SlotIdx));
}

inline int CMainEngine::MonitorPluginToSlot(int PlugIdx)
{
	if (PlugIdx < 0)
		return(-1);
	return(PluginToSlot(PlugIdx));
}
