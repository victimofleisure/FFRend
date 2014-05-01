// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun10	initial version
        01      06may12	add monitor source
        02      01jun12	add events; use own timer

		automated undo test
 
*/

#pragma once

#include "ProgressDlg.h"

class CMainFrame;
class CFFRendView;
class CClipEngine;
class CUndoManager;

class CUndoTest : public WObject {
public:
// Construction
	CUndoTest(bool Start);
	~CUndoTest();

// Attributes
	bool	IsRunning() const;

// Operations
	bool	Run(bool InitRunning);

protected:
// Types
	typedef struct tagEDIT_INFO {
		int		UndoCode;		// undo code; see UndoCodes.h
		float	Probability;	// relative probability
	} EDIT_INFO;

// Constants
	enum {	// command returns
		SUCCESS,
		DISABLED,
		ABORT,
		EVENT,
	};
	enum {	// states
		STOP,
		EDIT,
		UNDO,
		REDO,
		STATES
	};
	enum {
		TIMER_PERIOD = 80
	};
	static const LPCTSTR	m_TestPlug[];
	static const int		m_TestPlugCount;
	static const LPCTSTR	m_TestClip[];
	static const int		m_TestClipCount;
	static const EDIT_INFO	m_EditInfo[];
	static const int		m_EditInfoCount;
	static const LPCTSTR	m_StateName[STATES];

// Member data
	bool	m_InitRunning;		// true if initally running
	FILE	*m_LogFile;			// log file for test results
	CMainFrame	*m_Main;		// pointer to main frame
	CFFRendView	*m_View;		// pointer to view
	CMainEngine	*m_Engine;		// pointer to engine
	CUndoManager	*m_UndoMgr;	// pointer to undo manager
	CMetaparmBar	*m_Metaparm;	// pointer to metaparameter bar
	int		m_State;			// current state
	int		m_Passes;			// number of passes to do
	int		m_PassEdits;		// number of edits per pass
	int		m_PassUndos;		// number of undos per pass
	int		m_MaxEdits;			// maximum number of edits
	int		m_MaxPlugins;		// maximum number of plugins
	int		m_PassesDone;		// number of passes completed
	int		m_EditsDone;		// number of edits completed
	int		m_UndosDone;		// number of undos completed
	int		m_UndosToDo;		// number of undos to do
	int		m_StepsDone;		// number of steps completed
	int		m_RandSeed;			// random number generator seed
	int		m_Clock;			// number of timer hook calls
	CDWordArray	m_UndoCode;		// array of undo codes
	CProgressDlg	m_ProgressDlg;	// progress dialog

// Helpers
	void	Init();
	bool	Create();
	void	Destroy();
	int		LogPrintf(LPCTSTR Format, ...);
	static	int		Random(int Vals);
	int		ApplyEdit(int UndoCode);
	int		GetRandEdit() const;
	int		GetRandSlot() const;
	int		GetRandInsertSlot() const;
	int		GetRandLoadedSlot() const;
	int		GetRandEffectSlot() const;
	bool	GetRandMonitorSlot(int& SlotIdx) const;
	LPCTSTR	GetRandPlugin() const;
	LPCTSTR	GetRandClip() const;
	int		GetRandParm(int SlotIdx) const;
	float	GetRandNormVal() const;
	int		GetRandMetaparm() const;
	int		GetRandInsertMetaparm() const;
	int		GetRandAssignedMetaparm() const;
	bool	GetRandMetaparmTarget(CMetaparm::TARGET& Target) const;
	void	ShowBar(CMySizingControlBar& Bar, int Show = -1);
	void	InitBars();
	bool	LastPass() const;
	void	SetState(int State);
	void	OnTimer();
	bool	DoRandEvent(int SeqNum);
	static	VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
};

inline bool CUndoTest::IsRunning() const
{
	return(m_State != STOP);
}

inline bool CUndoTest::LastPass() const
{
	return(m_PassesDone >= m_Passes - 1);
}
