// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      18nov11	convert load balance from dialog to bar
        02      19nov11	add playlist dialog
		03		24nov11	pass previous slot to OnSelChange
		04		30nov11	add toggle window
		05		16dec11 add get/set tool dialog state
		06		13jan12	add NotifyEdit wrapper
		07		23jan12	add check for updates
		08		26mar12 add monitor source
		09		28mar12 override GetMessageString to support ID ranges
		10		06may12	change GetEngine to return most-derived engine
		11		23may12	make monitor source a slot index

        main frame
 
*/

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__389C08EE_EC90_479E_939D_392697DA83FF__INCLUDED_)
#define AFX_MAINFRM_H__389C08EE_EC90_479E_939D_392697DA83FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MainEngine.h"
#include "Renderer.h"
#include "Benchmark.h"
#include "WndTimer.h"
#include "HistoryBar.h"
#include "PatchBar.h"
#include "GraphBar.h"
#include "QueueBar.h"
#include "MasterBar.h"
#include "FileBrowserFF.h"
#include "MonitorBar.h"
#include "MidiSetupBar.h"
#include "MetaparmBar.h"
#include "OptionsDlg.h"
#include "AccelTable.h"
#include "MidiIO.h"
#include "Recorder.h"
#include "RecordDlg.h"
#include "RecStatDlg.h"
#include "JobControlDlg.h"
#include "UndoManager.h"
#include "LoadBalanceBar.h"
#include "PlaylistDlg.h"

class CFFRendView;
class CFFPlugin;

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Constants
	enum {	// timer IDs
		FRAME_TIMER_ID	= 256,
		VIEW_TIMER_ID	= 257,
	};
	enum {
		DIR_PROJECTS,
		DIR_PLUGINS,
		DIR_CLIPS,
		DIR_EXPORT,
		DIR_RECORD,
		DIR_PLAYLIST,
		FOLDERS
	};

// Attributes
public:
	void	SetView(CFFRendView *View);
	CFFRendView	*GetView();
	CMainEngine&	GetEngine();
	CRenderer&	GetRenderer();
	COptionsDlg&	GetOptions();
	CStatusBar&		GetStatusBar();
	CMasterBar&	GetMasterBar();
	CPatchBar&	GetPatchBar();
	CFileBrowserFF&	GetFilesBar();
	CMonitorBar&	GetMonitorBar();
	CMidiSetupBar&	GetMidiSetupBar();
	CMetaparmBar&	GetMetaparmBar();
	CGraphBar&	GetGraphBar();
	CHistoryBar&	GetHistoryBar();
	CQueueBar&	GetQueueBar();
	CRecorder&	GetRecorder();
	CRecordDlg&	GetRecordDlg();
	CRecStatDlg&	GetRecordStatusDlg();
	CJobControlDlg&	GetJobControlDlg();
	CPlaylistDlg&	GetPlaylistDlg();
	CUndoManager&	GetUndoManager();
	CLoadBalanceBar&	GetLoadBalanceBar();
	bool	IsRunning() const;
	bool	IsPaused() const;
	bool	IsRecording() const;
	void	GetProject(CFFProject& Project);
	bool	SetProject(const CFFProject& Project);
	HACCEL	GetAccelTable() const;
	bool	IsFullScreen() const;
	bool	IsDualMonitor() const;
	bool	IsSingleMonitorExclusive() const;
	bool	PromptFile(int FolderIdx, bool OpenFile, LPCTSTR DefName, int TitleID, CString& Path);
	LPCTSTR	GetFolder(int FolderIdx);
	void	SetFolder(int FolderIdx, LPCTSTR Path);
	float	GetActualFrameRate() const;
	void	GetJobInfo(CJobInfo& Info);
	void	SetJobInfo(const CJobInfo& Info);
	bool	GetBatchMode() const;
	void	SetBatchMode(bool Enable);
	void	SetModify(bool Enable = TRUE);
	bool	IsModified() const;
	int		GetMonitorSource() const;
	void	SetMonitorSource(int SlotIdx);
	bool	MonitoringOutput() const;

// Operations
public:
	void	EnableAccels(bool Enable);
	void	ApplyOptions();
	bool	Run(bool Enable);
	bool	Pause(bool Enable);
	void	UpdateViews();
	void	RunViews(bool Enable);
	void	DropFiles(int SlotIdx, const CStringArray& PathList);
	int		FindSlot(CPoint pt) const;
	bool	FullScreen(bool Enable);
	void	UpdateBars();
	void	UpdateMidiDevice();
	void	AddMetaparm(int PageType, int SlotIdx, int ParmIdx, int PropIdx);
	void	AddAllMetaparms(int SlotIdx);
	void	ShowPluginProps(const CFFPlugin& Plugin) const;
	bool	ShowPluginProps(LPCTSTR Path) const;
	bool	ShowClipProps(LPCTSTR Path) const;
	bool	OpenClip(int SlotIdx, LPCTSTR Path);
	bool	Record(bool Enable);
	bool	RunBatchJobs();
	UINT	CheckColorDepth();
	void	NotifyEdit(WORD CtrlID, WORD Code, UINT Flags = 0);

// Handlers
	void	OnBypass(int SlotIdx, bool Enable);
	void	OnSelChange(int PrevSlotIdx, int SlotIdx);
	void	OnModify(bool Modified);

// Nested classes
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual HACCEL GetDefaultAccelerator();
	virtual BOOL DestroyWindow();
	virtual void GetMessageString(UINT nID, CString& rMessage) const;
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_StatusBar;
	CToolBar    m_ToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAppCancel();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEditOptions();
	afx_msg void OnEditRedo();
	afx_msg void OnEditUndo();
	afx_msg void OnFileExport();
	afx_msg void OnFileJobControl();
	afx_msg void OnFileMetaExport();
	afx_msg void OnFileMetaImport();
	afx_msg void OnFileMetaProps();
	afx_msg void OnFileRecord();
	afx_msg void OnFileVideoOpen();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileExport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileJobControl(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileVideoMruFile(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFilesBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewGraph(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHistory(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMaster(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMetaparm(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMidi(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMonitor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewOutput(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPatch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewQueues(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewRecordStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowFullScreen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowPause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowStep(CCmdUI* pCmdUI);
	afx_msg void OnViewFilesBar();
	afx_msg void OnViewGraph();
	afx_msg void OnViewHistory();
	afx_msg void OnViewMaster();
	afx_msg void OnViewMetaparm();
	afx_msg void OnViewMidiSetup();
	afx_msg void OnViewMonitor();
	afx_msg void OnViewOutput();
	afx_msg void OnViewPatch();
	afx_msg void OnViewQueues();
	afx_msg void OnViewRecordStatus();
	afx_msg void OnWindowFullScreen();
	afx_msg void OnWindowPause();
	afx_msg void OnWindowStep();
	afx_msg void OnViewLoadBalance();
	afx_msg void OnUpdateViewLoadBalance(CCmdUI* pCmdUI);
	afx_msg void OnEditPlaylist();
	afx_msg void OnUpdateEditPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnHelpCheckForUpdates();
	//}}AFX_MSG
	afx_msg void OnActivateApp(BOOL bActive, ACTIVATEAPPTASK hTask);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnHandleDlgKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateIndicatorFrameSize(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorTargetFrameRate(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorPause(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorRecord(CCmdUI *pCmdUI);
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg	LRESULT OnEngineStall(WPARAM wParam, LPARAM lParam);
	afx_msg	LRESULT OnGraphDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFBOpenItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBRenameItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBDragEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBFileOpen();
	afx_msg void OnFBFileProps();
	afx_msg LRESULT OnMidiIn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFolderChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFileVideoMruFile(UINT nID);
	afx_msg	LRESULT OnEndRecord(WPARAM wParam, LPARAM lParam);
	afx_msg	LRESULT OnAppUpdateInfo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagSIZING_BAR_INFO {
		int		BarIdx;		// control bar index; see enum below
		int		TitleID;	// string resource ID of bar's title
		int		DockStyle;	// docking style flags
		int		DockBarID;	// dock bar ID for initial docking
		BOOL	InitShow;	// true if bar should show on initial run
		POINT	InitFloat;	// initial floating position in client coords
	} SIZING_BAR_INFO;
	typedef struct tagACCEL_EDIT {	// accelerator table edit
		bool	Enable;		// if true, enable accelerator, else disable it
		int		StartID;	// target command ID, or start of command ID range
		int		EndID;		// if non-zero, end of command ID range
	} ACCEL_EDIT;
	typedef struct tagFOLDER_INFO {	// persistent folder information
		LPCTSTR	RegKey;		// registry key
		LPCTSTR	DefExt;		// default extension if any
		int		Filter;		// resource ID of file filter string
		int		PaneIdx;	// files bar pane index, or -1 if none
	} FOLDER_INFO;
	typedef struct tagCLIP_PROPS {
		CSize	FrameSize;	// frame size
		DWORD	FrameCount;	// frame count
		float	FrameRate;	// frame rate, in frames per second
		DWORD	BitCount;	// color depth, in bits per pixel
		DWORD	FCCHandler;	// compressor code if applicable
	} CLIP_PROPS;
	class CMyUndoManager : public CUndoManager {
	public:
		void	OnModify(bool Modified);
	};

// Constants
	enum {	// control bar indices
		// APPEND ONLY: this list generates unique IDs for the bars
		CBI_GRAPH,
		CBI_PATCH,
		CBI_QUEUE,
		CBI_HISTORY,
		CBI_MASTER,
		CBI_FILE_BROWSER,
		CBI_MIDI_SETUP,
		CBI_METAPARM,
		CBI_MONITOR,
		CBI_LOAD_BALANCE,
		CONTROL_BARS
	};
	enum {	// sizing control bars; must match m_SizingBar and m_SizingBarInfo
		SCB_GRAPH,
		SCB_PATCH,
		SCB_QUEUE,
		SCB_HISTORY,
		SCB_MASTER,
		SCB_FILE_BROWSER,
		SCB_MIDI_SETUP,
		SCB_METAPARM,
		SCB_MONITOR,
		SCB_LOAD_BALANCE,
		SIZING_BARS
	};
	enum {	// tool dialogs
		TD_RECORD_STATUS,
		TD_JOB_CONTROL,
		TD_PLAYLIST,
		TOOL_DLGS
	};
	enum {	// status panes
		SBP_MESSAGE,
		SBP_FRAME_SIZE,
		SBP_TARGET_FRAME_RATE,
		SBP_ACTUAL_FRAME_RATE,
		SBP_PAUSE,
		STATUS_BAR_PANES
	};
	enum {	// main menus
		MENU_FILE,
		MENU_EDIT,
		MENU_PLUGIN,
		MENU_VIEW,
		MENU_WINDOW,
		MENU_HELP,
		MAIN_MENUS
	};
	enum {
		FRAME_TIMER_PERIOD = 1000,
		VIEW_TIMER_PERIOD = 40,
		MAX_RECENT_CLIPS = 4,
	};
	static const SIZING_BAR_INFO m_SizingBarInfo[SIZING_BARS];
	static const ACCEL_EDIT	m_SMExclAccelEdit[];	// edits in single-monitor exclusive
	static const FOLDER_INFO m_FolderInfo[FOLDERS];

// Member data
	bool	m_WasShown;
	bool	m_ShowOutput;
	CFFRendView	*m_View;	// pointer to view
	CMainEngine	m_Engine;		// plugin engine
	CRenderer	m_Renderer;	// rendering window
	CWndTimer	m_FrameTimer;	// frame timer
	CBenchmark	m_FrameBench;	// frame benchmark
	UINT	m_PrevFrameCount;	// previous frame counter
	CMySizingControlBar	*m_SizingBar[SIZING_BARS];	// list of sizable control bars
	CToolDlg	*m_ToolDlg[TOOL_DLGS];	// list of tool modeless dialogs
	CMasterBar	m_MasterBar;	// master control bar
	CPatchBar	m_PatchBar;		// patch control bar
	CFileBrowserFF	m_FilesBar;	// file browser control bar
	CMonitorBar	m_MonitorBar;	// monitor control bar
	CMidiSetupBar	m_MidiSetupBar;	// MIDI setup control bar
	CMetaparmBar	m_MetaparmBar;	// metaparameter control bar
	CGraphBar	m_GraphBar;		// graph control bar
	CHistoryBar	m_HistoryBar;	// history control bar
	CQueueBar	m_QueueBar;		// queue control bar
	COptionsDlg	m_OptsDlg;		// options dialog
	CLoadBalanceBar	m_LoadBalanceBar;	// load balance bar
	UINT	m_ShowToolDlg;		// tool dialog visibility bitmask
	bool	m_EnableAccels;		// true if accelerators are enabled
	CWndTimer	m_ViewTimer;	// view timer
	CString	m_Folder[FOLDERS];	// persistent folders
	HACCEL	m_PrevMainAccel;	// backup of main accelerators
	CAccelTable	m_SMExclAccel;	// safe accelerators in single-monitor exclusive
	HHOOK	m_KeybdHook;		// if non-zero, handle to keyboard hook procedure
	CRect	m_PreExclWndRect;	// window rect restore when exclusive ends
	bool	m_FullScrnDualMon;	// if true, go full-screen in dual-monitor exclusive
	int		m_MidiDev;			// currently selected MIDI device
	CMidiIO	m_MidiIO;			// MIDI interface
	int		m_InputPopupPos;	// position of first input popup in plugin menu
	int		m_InputPopups;		// number of input popups in plugin menu
	CString	m_StatusMsg;		// text to display in status bar message
	CRecentFileList	m_RecentClip;	// recently used clips
	CRecorder	m_Recorder;		// recording thread
	CRecordDlg	m_RecordDlg;	// record dialog
	CRecStatDlg	m_RecStatDlg;	// record status dialog
	float	m_ActualFrameRate;	// actual frame rate in frames per second
	bool	m_BatchMode;		// true if we're running queued jobs
	bool	m_BatchPeek;		// true if we're in RunBatchJobs peek message loop
	bool	m_IsClosing;		// true if we've received a WM_CLOSE message
	CMenu	m_BatchMenu;		// special menu in batch mode
	CMenu	*m_BatchPrevMenu;	// backup of normal menus in batch mode
	CJobControlDlg	m_JobCtrlDlg;	// job control dialog
	CPlaylistDlg	m_PlaylistDlg;	// playlist dialog
	CMyUndoManager	m_UndoMgr;	// undo manager

// Helpers
	BOOL	VerifyBarState(LPCTSTR lpszProfileName);
	static	BOOL	VerifyDockState(const CDockState& state, CFrameWnd *Frm);
	bool	CreateSizingBars();
	void	DockSizingBars();
	UINT	GetToolDlgState() const;
	void	SetToolDlgState(UINT State);
	bool	SetSingleMonitorExclusiveAccels(bool Enable);
	static	void	EditAccels(CAccelArray& AccArr, const ACCEL_EDIT *EditList, bool Default);
	static	LRESULT CALLBACK KeybdProc(int nCode, WPARAM wParam, LPARAM lParam);
	bool	HookKeybd(bool Enable);
	void	FullScreenMain(bool Enable);
	bool	ExportMetaplugin(LPCTSTR Path);
	bool	ImportMetaplugin(LPCTSTR Path);
	static	void	CALLBACK MidiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	static	bool	ShowWarning(int TemplateID, int TimeoutSecs);
	bool	ShutdownWarn();
	static	bool	MsgBoxCallback(PVOID Cookie, LPCTSTR Caption, LPCTSTR MsgText);
	static	bool	GetClipProps(LPCTSTR Path, CLIP_PROPS& Props);
	void	ToggleWindow(CWnd& Wnd);
};

inline CFFRendView *CMainFrame::GetView()
{
	return(m_View);
}

inline void CMainFrame::SetView(CFFRendView *View)
{
	m_View = View;
}

inline CMainEngine& CMainFrame::GetEngine()
{
	return(m_Engine);
}

inline CRenderer& CMainFrame::GetRenderer()
{
	return(m_Renderer);
}

inline CStatusBar& CMainFrame::GetStatusBar()
{
	return(m_StatusBar);
}

inline COptionsDlg& CMainFrame::GetOptions()
{
	return(m_OptsDlg);
}

inline CMasterBar& CMainFrame::GetMasterBar()
{
	return(m_MasterBar);
}

inline CPatchBar& CMainFrame::GetPatchBar()
{
	return(m_PatchBar);
}

inline CFileBrowserFF& CMainFrame::GetFilesBar()
{
	return(m_FilesBar);
}

inline CMonitorBar& CMainFrame::GetMonitorBar()
{
	return(m_MonitorBar);
}

inline CMidiSetupBar& CMainFrame::GetMidiSetupBar()
{
	return(m_MidiSetupBar);
}

inline CMetaparmBar& CMainFrame::GetMetaparmBar()
{
	return(m_MetaparmBar);
}

inline CGraphBar& CMainFrame::GetGraphBar()
{
	return(m_GraphBar);
}

inline CHistoryBar& CMainFrame::GetHistoryBar()
{
	return(m_HistoryBar);
}

inline CQueueBar& CMainFrame::GetQueueBar()
{
	return(m_QueueBar);
}

inline CRecorder& CMainFrame::GetRecorder()
{
	return(m_Recorder);
}

inline CRecordDlg& CMainFrame::GetRecordDlg()
{
	return(m_RecordDlg);
}

inline CRecStatDlg& CMainFrame::GetRecordStatusDlg()
{
	return(m_RecStatDlg);
}

inline CJobControlDlg& CMainFrame::GetJobControlDlg()
{
	return(m_JobCtrlDlg);
}

inline CPlaylistDlg& CMainFrame::GetPlaylistDlg()
{
	return(m_PlaylistDlg);
}

inline CUndoManager& CMainFrame::GetUndoManager()
{
	return(m_UndoMgr);
}

inline CLoadBalanceBar& CMainFrame::GetLoadBalanceBar()
{
	return(m_LoadBalanceBar);
}

inline void CMainFrame::EnableAccels(bool Enable)
{
	m_EnableAccels = Enable;
}

inline bool CMainFrame::IsRunning() const
{
	return(m_Engine.IsRunning());
}

inline bool CMainFrame::IsPaused() const
{
	return(m_Engine.IsPaused());
}

inline bool CMainFrame::IsRecording() const
{
	return(m_Recorder.IsRecording());
}

inline HACCEL CMainFrame::GetAccelTable() const
{
	return(m_hAccelTable);
}

inline bool CMainFrame::IsFullScreen() const
{
	return(m_Renderer.IsExclusive());
}

inline bool CMainFrame::IsDualMonitor() const
{
	return(m_Renderer.IsDualMonitor());
}

inline bool CMainFrame::IsSingleMonitorExclusive() const
{
	return(IsFullScreen() && !IsDualMonitor());
}

inline LPCTSTR CMainFrame::GetFolder(int FolderIdx)
{
	ASSERT(FolderIdx >= 0 && FolderIdx < FOLDERS);
	return(m_Folder[FolderIdx]);
}

inline void CMainFrame::SetFolder(int FolderIdx, LPCTSTR Path)
{
	ASSERT(FolderIdx >= 0 && FolderIdx < FOLDERS);
	m_Folder[FolderIdx] = Path;
}

inline float CMainFrame::GetActualFrameRate() const
{
	return(m_ActualFrameRate);
}

inline bool CMainFrame::GetBatchMode() const
{
	return(m_BatchMode);
}

inline void CMainFrame::NotifyEdit(WORD CtrlID, WORD Code, UINT Flags)
{
	m_UndoMgr.NotifyEdit(CtrlID, Code, Flags);
}

inline int CMainFrame::GetMonitorSource() const
{
	return(m_Engine.GetMonitorSource());
}

inline bool CMainFrame::MonitoringOutput() const
{
	return(m_Engine.MonitoringOutput());
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__389C08EE_EC90_479E_939D_392697DA83FF__INCLUDED_)
