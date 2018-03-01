// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		20jun10	in FullScreenMain, add no copy bits flag
		02		24jun10	in RunBatchJobs, start jobs from pause and check links
		03		28aug10	in OnWindowStep, run view timer so auto sliders update
		04		29apr11	in OnAppCancel, call CancelDrag for view and bars
		05		04may11	convert options dialog to property sheet
		06		05may11	in UpdateMidiDevice, show wait cursor
		07		11may11	in OnDropFiles, add DragFinish to avoid leak
        08      18nov11	convert load balance from dialog to bar
        09      19nov11	add playlist dialog
        10      22nov11	disable wait cursor in single-monitor exclusive
		11		24nov11	pass previous slot to OnSelChange
		12		30nov11	add toggle window
		13		01dec11	add run while loading, frame memory limit
		14		15dec11 in OnClose, add playlist save check
		15		16dec11 add get/set tool dialog state
		16		27dec11	in OnWindowStep, update monitor bar
		17		23jan12	add check for updates
		18		26mar12 add monitor source
		19		28mar12 override GetMessageString to support ID ranges
		20		11apr12	in OnTimer, reset frame benchmark ASAP
		21		13apr12	in PromptFile, remove multithreaded file dialog
		22		24apr12	create engine before launching render thread
		23		23may12	make monitor source a slot index
		24		29may12	in OnCreate, set folders after applying options
		25		01jun12	remove undo test hook

        main frame
 
*/

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "FFRend.h"

#include "MainFrm.h"
#include "FFRendDoc.h"
#include "FFRendView.h"

#include "GraphBar.h"
#include "GraphView.h"
#include "HistoryView.h"
#include "SizingDockFrame.h"
#include "Persist.h"
#include "ProgressDlg.h"
#include "MultiFileDlg.h"
#include "FFPluginEx.h"
#include "FFPlugsRow.h"
#include "PathStr.h"
#include "MonitorInfo.h"
#include "MsgBoxDlg.h"
#include "HookMsgBox.h"
#include "VersionInfo.h"
#include "FocusEdit.h"
#include "AviToBmp.h"
#include "MetaplugPropsDlg.h"
#include "Metaproject.h"
#include "UpdateCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

#define RK_MAIN_FRAME		_T("MainFrame")
#define RK_SHOW_OUTPUT		_T("ShowOutput")
#define RK_SHOW_HISTORY		_T("ShowHistory")
#define RK_BAR_DOCK_STYLE	_T("SizingBarDockStyle")
#define	RK_PROJECT_FOLDER	_T("ProjectFolder")
#define	RK_PLUGIN_FOLDER	_T("PluginFolder")
#define	RK_CLIP_FOLDER		_T("ClipFolder")
#define	RK_EXPORT_FOLDER	_T("ExportFolder")
#define	RK_PLAYLIST_FOLDER	_T("PlaylistFolder")
#define RK_SHOW_TOOL_DLG	_T("ShowToolDlg")

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_FRAME_SIZE,
	ID_INDICATOR_TARGET_FRAME_RATE,
	ID_INDICATOR_ACTUAL_FRAME_RATE,
	ID_INDICATOR_PAUSE,
	ID_INDICATOR_RECORD,
};

// this list must match the sizing control bars enum
const CMainFrame::SIZING_BAR_INFO CMainFrame::m_SizingBarInfo[SIZING_BARS] = {
//	BarIdx				TitleID					DockStyle		DockBarID				InitShow	InitFloat
	{CBI_GRAPH,			IDS_CBT_GRAPH,			CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_RIGHT,	FALSE,		{0, 0}},
	{CBI_PATCH,			IDS_CBT_PATCH,			CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_LEFT,	FALSE,		{0, 0}},
	{CBI_QUEUE,			IDS_CBT_QUEUE,			CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_LEFT,	FALSE,		{0, 0}},
	{CBI_HISTORY,		IDS_CBT_HISTORY,		CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
	{CBI_MASTER,		IDS_CBT_MASTER,			CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_TOP,	FALSE,		{0, 0}},
	{CBI_FILE_BROWSER,	IDS_CBT_FILE_BROWSER,	CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_RIGHT,	FALSE,		{0, 0}},
	{CBI_MIDI_SETUP,	IDS_CBT_MIDI_SETUP,		CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
	{CBI_METAPARM,		IDS_CBT_METAPARM,		CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
	{CBI_MONITOR,		IDS_CBT_MONITOR,		CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
	{CBI_LOAD_BALANCE,	IDS_CBT_LOAD_BALANCE,	CBRS_ALIGN_ANY,	AFX_IDW_DOCKBAR_BOTTOM,	FALSE,		{0, 0}},
};

#define MAKEBARID(idx) (AFX_IDW_CONTROLBAR_FIRST + 32 + idx)

// single-monitor exclusive accelerator table edits
const CMainFrame::ACCEL_EDIT CMainFrame::m_SMExclAccelEdit[] = {
	{TRUE,	ID_APP_CANCEL},
	{TRUE,	ID_WINDOW_PAUSE},
	{TRUE,	ID_WINDOW_STEP},
	{TRUE,	ID_WINDOW_FULL_SCREEN},
	{0}	// list terminator
};

const CMainFrame::FOLDER_INFO CMainFrame::m_FolderInfo[FOLDERS] = {
	{RK_PROJECT_FOLDER,		PROJECT_EXT,	IDS_PROJECT_FILTER,		CFileBrowserFF::PANE_PROJECTS},
	{RK_PLUGIN_FOLDER,		PLUGIN_EXT,		IDS_PLUGIN_FILTER,		CFileBrowserFF::PANE_PLUGINS},
	{RK_CLIP_FOLDER,		NULL,			IDS_VIDEO_FILTER,		CFileBrowserFF::PANE_CLIPS},
	{RK_EXPORT_FOLDER,		BITMAP_EXT,		IDS_BITMAP_FILTER,		-1},
	{RK_EXPORT_FOLDER,		AVI_EXT,		IDS_AVI_FILTER,			-1},
	{RK_PLAYLIST_FOLDER,	PLAYLIST_EXT,	IDS_PLAYLIST_FILTER,	-1},
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
	m_Engine(m_Renderer), 
	m_Renderer(&m_Engine),
	m_MetaparmBar(m_Engine.GetMetaplugin().m_Metaparm),
	m_RecentClip(0, _T("Recent Clip"), _T("Clip%d"), MAX_RECENT_CLIPS),
	m_Recorder(&m_Engine),
	m_OptsDlg(IDS_OPTIONS)
{
	m_WasShown = FALSE;
	m_ShowOutput = CPersist::GetInt(REG_SETTINGS, RK_SHOW_OUTPUT, TRUE) != 0;
	m_View = NULL;
	m_PrevFrameCount = 0;
	// these inits must match the sizing control bars enum
	m_SizingBar[SCB_GRAPH]			= &m_GraphBar;
	m_SizingBar[SCB_PATCH]			= &m_PatchBar;
	m_SizingBar[SCB_QUEUE]			= &m_QueueBar;
	m_SizingBar[SCB_HISTORY]		= &m_HistoryBar;
	m_SizingBar[SCB_MASTER]			= &m_MasterBar;
	m_SizingBar[SCB_FILE_BROWSER]	= &m_FilesBar;
	m_SizingBar[SCB_MIDI_SETUP]		= &m_MidiSetupBar;
	m_SizingBar[SCB_METAPARM]		= &m_MetaparmBar;
	m_SizingBar[SCB_MONITOR]		= &m_MonitorBar;
	m_SizingBar[SCB_LOAD_BALANCE]	= &m_LoadBalanceBar;
	// these inits must match the tool dialogs enum
	m_ToolDlg[TD_RECORD_STATUS]		= &m_RecStatDlg;
	m_ToolDlg[TD_JOB_CONTROL]		= &m_JobCtrlDlg;
	m_ToolDlg[TD_PLAYLIST]			= &m_PlaylistDlg;
	m_ShowToolDlg = theApp.RdRegInt(RK_SHOW_TOOL_DLG, 0);
	m_EnableAccels = TRUE;
	for (int FolderIdx = 0; FolderIdx < FOLDERS; FolderIdx++)
		m_Folder[FolderIdx] = theApp.RdRegString(m_FolderInfo[FolderIdx].RegKey);
	m_PrevMainAccel = NULL;
	m_KeybdHook = NULL;
	m_PreExclWndRect.SetRectEmpty();
	m_FullScrnDualMon = TRUE;
	m_MidiDev = CMidiIO::NO_DEVICE;
	m_MidiIO.SetInputCallback(MidiCallback, this);
	m_InputPopupPos = -1;
	m_InputPopups = 0;
	m_RecentClip.ReadList();
	m_ActualFrameRate = 0;
	m_BatchMode = FALSE;
	m_BatchPeek = FALSE;
	m_IsClosing = FALSE;
	m_BatchPrevMenu = NULL;
}

CMainFrame::~CMainFrame()
{
	CPersist::WriteInt(REG_SETTINGS, RK_SHOW_OUTPUT, m_ShowOutput);
	for (int FolderIdx = 0; FolderIdx < FOLDERS; FolderIdx++)
		theApp.WrRegString(m_FolderInfo[FolderIdx].RegKey, m_Folder[FolderIdx]);
	m_RecentClip.WriteList();
	theApp.WrRegInt(RK_SHOW_TOOL_DLG, m_ShowToolDlg);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	theApp.m_pMainWnd = this;	// so view's OnCreate can use GetMain
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	if (!CreateSizingBars())
	{
		TRACE0("Failed to create sizing control bars\n");
		return -1;      // fail to create
	}
	// replacing CBRS_ALIGN_ANY with the following MAGIC prevents horizontally
	// docked bars from taking over the whole frame width; thanks Cristi Posea
	EnableDocking(CBRS_ALIGN_TOP);
	EnableDocking(CBRS_ALIGN_LEFT);
	EnableDocking(CBRS_ALIGN_RIGHT);
	EnableDocking(CBRS_ALIGN_BOTTOM);
	m_ToolBar.EnableDocking(CBRS_ALIGN_ANY);
	// set m_pFloatingFrameClass after ALL EnableDocking calls,
	// because EnableDocking also sets m_pFloatingFrameClass
	m_pFloatingFrameClass = RUNTIME_CLASS(CSizingDockFrame);	// custom dock frame
	DockControlBar(&m_ToolBar);
	DockSizingBars();
	if (VerifyBarState(REG_SETTINGS)) {	// verify bar state before loading it
		for (int i = 0; i < SIZING_BARS; i++)
			m_SizingBar[i]->LoadState(REG_SETTINGS);
		LoadBarState(REG_SETTINGS);
	}
	for (int i = 0; i < SIZING_BARS; i++)
		m_SizingBar[i]->SendMessage(WM_INITDIALOG);	// initialize sizing bars
	m_RecStatDlg.Create(IDD_REC_STAT);
	m_JobCtrlDlg.Create(IDD_JOB_CONTROL);
	m_PlaylistDlg.Create(IDD_PLAYLIST);
	DragAcceptFiles();
	m_UndoMgr.SetRoot(m_View);
	// create render window
	CRect	rc(CPoint(100, 100), CSize(320, 240));	// initial rect
	LPCTSTR RendererClass = AfxRegisterWndClass(
		CS_DBLCLKS,								// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	if (!m_Renderer.CreateEx(0, RendererClass, LDS(IDS_OUTPUT), 
	WS_OVERLAPPEDWINDOW, rc, 0, NULL)) {
		AfxMessageBox(IDS_MF_CANT_CREATE_RENDERER);
		return -1;
	}
	if (m_ShowOutput)
		m_Renderer.ShowWindow(SW_SHOW);
	// create engine
	if (!m_Engine.Create(this)) {
		AfxMessageBox(IDS_MF_CANT_CREATE_ENGINE);
		return -1;
	}
	if (!m_Renderer.LaunchRenderThread())
		return -1;
	// create timers, apply options, and run
	if (!(m_FrameTimer.Create(m_hWnd, FRAME_TIMER_ID, FRAME_TIMER_PERIOD)
	&& m_ViewTimer.Create(m_hWnd, VIEW_TIMER_ID, VIEW_TIMER_PERIOD))) {
		AfxMessageBox(IDS_REND_CANT_CREATE_TIMER);
		return -1;
	}
	ApplyOptions();
	// set files bar folders after options are applied to save time;
	// changing files bar options is faster when file list is empty
	for (int FolderIdx = 0; FolderIdx < FOLDERS; FolderIdx++) {
		FOLDER_INFO	info = m_FolderInfo[FolderIdx];
		if (info.PaneIdx >= 0)
			m_FilesBar.SetFolder(info.PaneIdx, m_Folder[FolderIdx]);
	}
	Run(TRUE);	// start engine

	return 0;
}

bool CMainFrame::CreateSizingBars()
{
	static const int BAR_STYLE = CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC;
	static const int STYLE = WS_CHILD | CBRS_TOP;	// invisible to avoid flicker
	CString	Title;
	for (int i = 0; i < SIZING_BARS; i++) {
		const SIZING_BAR_INFO& info = m_SizingBarInfo[i];
		CSizingControlBar	*bar = m_SizingBar[i];
		Title.LoadString(info.TitleID);
		if (!bar->Create(Title, this, MAKEBARID(info.BarIdx), STYLE))
			return(FALSE);
		bar->SetBarStyle(bar->GetBarStyle() | BAR_STYLE);
	}
	return(TRUE);
}

void CMainFrame::DockSizingBars()
{
	CDWordArray	RegDockStyle;
	RegDockStyle.SetSize(SIZING_BARS);
	DWORD	ValSize = INT64TO32(RegDockStyle.GetSize()) * sizeof(DWORD);
	BOOL	GotRegDockStyle = CPersist::GetBinary(REG_SETTINGS, 
		RK_BAR_DOCK_STYLE, RegDockStyle.GetData(), &ValSize);
	// 27apr10: don't assume current and registry bar count are the same
	DWORD	RegDockBars = ValSize / sizeof(DWORD);	// registry dock bar count
	for (DWORD i = 0; i < SIZING_BARS; i++) {
		const SIZING_BAR_INFO& info = m_SizingBarInfo[i];
		CSizingControlBar	*bar = m_SizingBar[i];
		// use dock style from registry if we have it, else take default value
		DWORD	DockStyle = GotRegDockStyle && i < RegDockBars ? 
			RegDockStyle[i] : info.DockStyle;
		bar->EnableDocking(DockStyle);
		if (DockStyle) {	// if bar is dockable
			DockControlBar(bar, info.DockBarID);
			if (info.InitShow)	// if bar is initially shown
				ShowControlBar(bar, TRUE, 0);	// show it
		} else {	// bar is undockable
			CPoint	pt(info.InitFloat);
			ClientToScreen(&pt);
			FloatControlBar(bar, pt);
		}
	}
}

BOOL CMainFrame::VerifyBarState(LPCTSTR lpszProfileName)
{
	CDockState	state;
	state.LoadState(lpszProfileName);
	return(VerifyDockState(state, this));
}

BOOL CMainFrame::VerifyDockState(const CDockState& state, CFrameWnd *Frm)
{
	// thanks to Cristi Posea at codeproject.com
	for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++) {
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		int nDockedCount = INT64TO32(pInfo->m_arrBarID.GetSize());
		if (nDockedCount > 0) {
			// dockbar
			for (int j = 0; j < nDockedCount; j++)
			{
				UINT	nID = (UINT) pInfo->m_arrBarID[j];
				if (nID == 0)
					continue; // row separator
				if (nID > 0xFFFF)
					nID &= 0xFFFF; // placeholder - get the ID
				if (Frm->GetControlBar(nID) == NULL)
					return FALSE;
			}
		}
		if (!pInfo->m_bFloating) // floating dockbars can be created later
			if (Frm->GetControlBar(pInfo->m_nBarID) == NULL)
				return FALSE; // invalid bar ID
	}
    return TRUE;
}

UINT CMainFrame::GetToolDlgState() const
{
	UINT	State = 0;
	for (int i = 0; i < TOOL_DLGS; i++) {
		if (m_ToolDlg[i]->IsWindowVisible())
			State |= (1 << i);
	}
	return(State);
}

void CMainFrame::SetToolDlgState(UINT State)
{
	for (int i = 0; i < TOOL_DLGS; i++)
		m_ToolDlg[i]->ShowWindow((State & (1 << i)) ? SW_SHOWNA : SW_HIDE);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

HACCEL CMainFrame::GetDefaultAccelerator()
{
	if (!m_EnableAccels)
		return(NULL);
	return(CFrameWnd::GetDefaultAccelerator());
}

void CMainFrame::GetProject(CFFProject& Project)
{
	m_Engine.GetProject(Project);
}

bool CMainFrame::SetProject(const CFFProject& Project)
{
	m_UndoMgr.DiscardAllEdits();
	if (!m_Engine.SetProject(Project))
		return(FALSE);
	m_MasterBar.GetDlg().SetSpeed(Project.m_Speed);
	return(TRUE);
}

UINT CMainFrame::CheckColorDepth()
{
	UINT	DisplayColorDepth = theApp.GetDisplayColorDepth();
	UINT	ColorDepth = m_OptsDlg.GetColorDepth();
	if (ColorDepth != DisplayColorDepth) {
		if (AfxMessageBox(IDS_MF_COLOR_DEPTH_MISMATCH, MB_YESNO) == IDYES) {
			ColorDepth = DisplayColorDepth;
			m_OptsDlg.SetColorDepth(ColorDepth);
		}
	}
	return(ColorDepth);
}

void CMainFrame::ApplyOptions()
{
	m_Engine.SetFrameProps(m_OptsDlg.GetFrameSize(), CheckColorDepth());
	m_Engine.SetFrameRate(m_OptsDlg.GetFrameRate());
	m_Engine.SetFrameTimeout(m_OptsDlg.GetFrameTimeout());
	m_Engine.SetHistorySize(m_OptsDlg.GetHistorySize());
	m_Engine.SetRunWhileLoading(m_OptsDlg.GetRunWhileLoading());
	m_Engine.SetFrameMemoryLimit(m_OptsDlg.GetFrameMemoryLimit());
	m_Renderer.SetLockFrameRate(m_OptsDlg.GetLockFrameRate());
	m_Renderer.SetUseMMTimer(m_OptsDlg.GetUseMMTimer());
	m_ViewTimer.SetPeriod(round(1000.0 / m_OptsDlg.GetViewFreq()));
	m_FilesBar.SetThumbSize(m_OptsDlg.GetThumbSize());
	m_FilesBar.SetCacheThumbs(m_OptsDlg.GetCacheThumbs());
	UpdateMidiDevice();
	bool	RandUseTime;
	UINT	RandSeed = m_OptsDlg.GetRandomSeed(RandUseTime);
	m_Engine.SetRandomSeed(RandSeed, RandUseTime);
	m_MonitorBar.SetQuality(m_OptsDlg.GetMonitorQuality());
	m_UndoMgr.SetLevels(m_OptsDlg.GetUndoLevels());
	bool	SaveChgsWarn = m_OptsDlg.GetSaveChgsWarn();
	if (SaveChgsWarn != m_OptsDlg.GetPrevOpts().m_SaveChgsWarn && !SaveChgsWarn)
		SetModify(FALSE);
	bool	CheckForUpdates = m_OptsDlg.GetCheckForUpdates();
	if (CheckForUpdates != m_OptsDlg.GetPrevOpts().m_CheckForUpdates && CheckForUpdates)
		CUpdateCheck::CheckAsync(m_hWnd);	// launch check updates thread
}

void CMainFrame::UpdateViews()
{
	m_View->UpdateView();
	m_PatchBar.UpdateView();
	m_QueueBar.UpdateView();
	m_HistoryBar.UpdateView();
	if (m_WasShown)	// delay initial graph update until bar is sized
		m_GraphBar.UpdateView();
	m_MidiSetupBar.UpdateView();
	m_MetaparmBar.UpdateView();
	m_LoadBalanceBar.UpdateView();
}

void CMainFrame::RunViews(bool Enable)
{
	m_ViewTimer.Run(Enable);
	m_QueueBar.Run(Enable);
	m_HistoryBar.Run(Enable);
}

bool CMainFrame::Run(bool Enable)
{
	bool	retc = m_Engine.Run(Enable);
	if (!retc)
		AfxMessageBox(Enable ? IDS_MF_CANT_START_ENGINE : IDS_MF_CANT_STOP_ENGINE);
	return(retc);
}

bool CMainFrame::Pause(bool Enable)
{
	bool	retc = m_Engine.Pause(Enable);
	if (!retc)
		AfxMessageBox(Enable ? IDS_MF_CANT_PAUSE_ENGINE : IDS_MF_CANT_RESUME_ENGINE);
	return(retc);
}

void CMainFrame::OnBypass(int SlotIdx, bool Enable)
{
	m_PatchBar.OnBypass(SlotIdx, Enable);
	m_View->OnBypass(SlotIdx, Enable);
}

void CMainFrame::OnSelChange(int PrevSlotIdx, int SlotIdx)
{
	m_PatchBar.OnSelChange(SlotIdx);
	m_View->OnSelChange(PrevSlotIdx, SlotIdx);
}

bool CMainFrame::OpenClip(int SlotIdx, LPCTSTR Path)
{
	if (!m_Engine.OpenClip(SlotIdx, Path))
		return(FALSE);
	m_RecentClip.Add(Path);
	return(TRUE);
}

void CMainFrame::DropFiles(int SlotIdx, const CStringArray& PathList)
{
	if (SlotIdx < 0)	// if no slot was specified
		SlotIdx = m_Engine.GetSlotCount();	// insert after last slot
	UINT	Files = PathList.GetSize();
	for (UINT i = 0; i < Files; i++) {
		LPCTSTR	Path = PathList[i];
		LPCTSTR	Ext = PathFindExtension(Path);
		if (!_tcsicmp(Ext, PLUGIN_EXT)) {	// if plugin
			if (m_View->Insert(SlotIdx, Path)) {
				SlotIdx++;
			}
		} else if (!_tcsicmp(Ext, PROJECT_EXT)) {	// if project
			if (!m_PlaylistDlg.DropProject(Path))	// try playlist first
				theApp.OpenDocumentFile(Path);
			break;
		} else {	// assume clip
			OpenClip(SlotIdx, Path);
		}
	}
}

int CMainFrame::FindSlot(CPoint pt) const
{
	CRect	r;
	m_PatchBar.GetWindowRect(r);	// assume point is in screen coords
	// if patch bar is visible and point is within patch bar
	if (m_PatchBar.FastIsVisible() && r.PtInRect(pt))
		return(m_PatchBar.FindSlot(pt));	// find patch bar slot containing point
	return(m_View->FindSlot(pt));	// delegate to view
}

void CMainFrame::EditAccels(CAccelArray& AccArr, const ACCEL_EDIT *EditList, bool Default)
{
	int	accs = AccArr.GetSize();
	for (int i = 0; i < accs; i++) {
		int	cmd = AccArr[i].cmd;
		bool	enab = Default;
		for (int j = 0; EditList[j].StartID; j++) {
			const ACCEL_EDIT&	ed = EditList[j];
			if (cmd == ed.StartID 
			|| ed.EndID && (cmd >= ed.StartID && cmd <= ed.EndID))
				enab = ed.Enable;
		}
		if (!enab)
			AccArr[i].cmd = 0;	// no-op this command
	}
}

LRESULT CALLBACK CMainFrame::KeybdProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CMainFrame	*frm = theApp.GetMain();
	if (nCode == HC_ACTION) {	// ignore peek
		LPKBDLLHOOKSTRUCT	khs = (LPKBDLLHOOKSTRUCT)lParam;
		switch (khs->vkCode) {	// disable task-switching keys
		case VK_LWIN:	// Windows keys
		case VK_RWIN:
			return(TRUE);	// eat key
		case VK_ESCAPE:
			if ((khs->flags & LLKHF_ALTDOWN)	// Alt+Escape 
			|| (GetAsyncKeyState(VK_CONTROL) & GKS_DOWN))	// Ctrl+Escape
				return(TRUE);	// eat key
			break;
		case VK_TAB:
			if (khs->flags & LLKHF_ALTDOWN)	// Alt+Tab
				return(TRUE);	// eat key
			break;
		case VK_LMENU:	// Alt keys
		case VK_RMENU:
			// if exclusive on a single monitor
			if (frm->IsSingleMonitorExclusive())
				return(TRUE);	// eat key to avoid modal menu state
		}
	}
	return(CallNextHookEx(frm->m_KeybdHook, nCode, wParam, lParam));
}

bool CMainFrame::HookKeybd(bool Enable)
{
	bool	retc = TRUE;
	if (Enable != (m_KeybdHook != NULL)) {	// if not in requested state
		if (Enable) {
			m_KeybdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeybdProc, 
				theApp.m_hInstance, 0);
			retc = m_KeybdHook != NULL;
		} else {
			retc = UnhookWindowsHookEx(m_KeybdHook) != 0;
			m_KeybdHook = NULL;
		}
	}
	return(retc);
}

bool CMainFrame::SetSingleMonitorExclusiveAccels(bool Enable)
{
	if (Enable == (m_PrevMainAccel != NULL))
		return(TRUE);	// nothing to do
	if (Enable) {
		if (m_SMExclAccel == NULL) {
			CAccelArray	AccArr;
			CAccelTable::GetArray(m_hAccelTable, AccArr);
			EditAccels(AccArr, m_SMExclAccelEdit, FALSE);
			m_SMExclAccel.LoadFromArray(AccArr);
			if (m_SMExclAccel == NULL)
				return(FALSE);	// error creating accelerator table
		}
		m_PrevMainAccel = m_hAccelTable;
		m_hAccelTable = m_SMExclAccel;
	} else {
		m_hAccelTable = m_PrevMainAccel;
		m_PrevMainAccel = NULL;
		m_SMExclAccel.Destroy();
	}
	return(TRUE);
}

void CMainFrame::FullScreenMain(bool Enable)
{
	if (Enable == !m_PreExclWndRect.IsRectNull())
		return;	// nothing to do
	CRect	rc;
	const UINT	NCArea = WS_CAPTION | WS_THICKFRAME;
	if (Enable) {	// if entering full-screen
		GetWindowRect(m_PreExclWndRect);
		CMonitorInfo::GetFullScreenRect(m_hWnd, rc);
		ModifyStyle(NCArea, 0);	// remove non-client area
		SetFocus();
	} else {	// exiting full-screen
		ModifyStyle(0, NCArea);	// restore non-client area
		rc = m_PreExclWndRect;
		m_PreExclWndRect.SetRectEmpty();
	}
	SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), 
		SWP_NOZORDER | SWP_NOCOPYBITS);
	// if thick frame style is removed from frame, dragging status bar gripper
	// moves status bar; disabling status bar prevents this, see MS Q177341
	m_StatusBar.EnableWindow(!Enable);
}

bool CMainFrame::FullScreen(bool Enable)
{
	if (Enable == IsFullScreen())
		return(TRUE);
	if (!HookKeybd(Enable) && Enable)	// hook keyboard before changing UI
		AfxMessageBox(IDS_MF_CANT_HOOK_KEYBD);
	if (!m_Renderer.SetExclusive(Enable)) {	// switch cooperative mode
		AfxMessageBox(IDS_MF_CANT_SET_EXCLUSIVE);
		return(FALSE);
	}
	if (!Enable)
		BringWindowToTop();
	FullScreenMain(Enable && IsDualMonitor() && m_FullScrnDualMon);
	if (!SetSingleMonitorExclusiveAccels(Enable && !IsDualMonitor()))
		return(FALSE);
	// disable wait cursor in single-monitor exclusive mode
	theApp.EnableWaitCursor(!(Enable && !IsDualMonitor()));
	return(TRUE);
}

void CMainFrame::UpdateBars()
{
	m_ToolBar.OnUpdateCmdUI(this, TRUE);
	m_StatusBar.OnUpdateCmdUI(this, TRUE);
}

void CMainFrame::UpdateMidiDevice()
{
	CWaitCursor	wc;	// EnableInput can be slow if CPU is saturated
	int	Device = m_OptsDlg.GetMidiDevice();
	if (Device != m_MidiDev) {
		m_MidiIO.EnableInput(FALSE);	// close previous MIDI device if any
		if (Device != CMidiIO::NO_DEVICE) {
			if (!m_MidiIO.EnableInput(TRUE, Device)) {
				CString	s, msg;
				m_MidiIO.GetLastErrorString(s);
				AfxFormatString1(msg, IDS_MF_CANT_CREATE_MIDI_DEV, s); 
				AfxMessageBox(msg);
			}
		}
		m_MidiDev = Device;
	}
}

bool CMainFrame::ExportMetaplugin(LPCTSTR Path)
{
	CMetaproject	Project;
	GetProject(Project);	// get project data
	return(Project.Export(Path));
}

bool CMainFrame::ImportMetaplugin(LPCTSTR Path)
{
	CMetaproject	Project;
	if (!Project.Import(Path))
		return(FALSE);
	Project.CheckLinks();	// try to repair broken links first
	if (!CFFRendDoc::CheckLinks(Project))	// display missing files dialog if needed
		return(FALSE);
	CString	VideoPath(Project.m_VideoPath);	// save video path
	Project.m_VideoPath.Empty();	// disable replacing null plugin with clip player
	if (!SetProject(Project))	// set project data
		return(FALSE);
	Project.m_VideoPath = VideoPath;	// restore video path
	CPathStr	ProjectPath(Path);
	ProjectPath.RenameExtension(PROJECT_EXT);
	m_View->GetDocument()->SetPathName(ProjectPath);
	SetModify(FALSE);
	return(TRUE);
}

void CMainFrame::AddMetaparm(int PageType, int SlotIdx, int ParmIdx, int PropIdx)
{
	CMetaparm::TARGET	Target;
	m_Engine.MakeTarget(PageType, SlotIdx, ParmIdx, PropIdx, Target);
	m_MetaparmBar.Add(Target);
}

void CMainFrame::AddAllMetaparms(int SlotIdx)
{
	ASSERT(m_Engine.IsLoaded(SlotIdx));
	CFFPluginEx	*slot = m_Engine.GetSlot(SlotIdx);
	int	parms = slot->GetParmCount();
	for (int i = 0; i < parms; i++)
		AddMetaparm(MPT_PARAM, SlotIdx, i, MP_PARAM);
}

void CMainFrame::ShowPluginProps(const CFFPlugin& Plugin) const
{
	PlugInfoStruct	pis;
	ZeroMemory(&pis, sizeof(pis));
	Plugin.GetInfo(pis);
	PlugExtendedInfoStruct	peis;
	ZeroMemory(&peis, sizeof(peis));
	Plugin.GetExtendedInfo(peis);
	CString	PluginName, PluginId, msg;
	Plugin.GetPluginName(PluginName);
	Plugin.GetPluginId(PluginId);
	msg.Format(LDS(IDS_FF_PLUG_PROPS),
		PluginName, PluginId,
		pis.APIMinorVersion / 1000.0 + pis.APIMajorVersion,
		peis.PluginMinorVersion / 1000.0 + peis.PluginMajorVersion,
		(pis.pluginType & 1) ? _T("Source") : _T("Effect"));
	CString	s;
	if (peis.Description != NULL) {	// optional description
		s.Format(IDS_FF_PLUG_DESCRIP, CString(peis.Description));
		msg += s;
	}
	if (peis.About != NULL) {	// optional about
		s.Format(IDS_FF_PLUG_ABOUT, CString(peis.About));
		msg += s;
	}
	typedef struct tagCAPINFO {	// capability information 
		int		CapIdx;	// capability's index, from FreeFrame.h
		int		Name;	// capability's name, as a resource ID
		int		States;	// number of optional state names; may be zero
		const	int		*StateName;	// array of resource IDs, one per state name
	} CAPINFO;
	static const int	YesNo[] = {
		IDS_FF_PLUG_CAP_NO,
		IDS_FF_PLUG_CAP_YES
	};
	static const int	CopyPref[] = {
		IDS_FF_PLUG_CAP_PREF_NONE,
		IDS_FF_PLUG_CAP_PREF_INPLACE,
		IDS_FF_PLUG_CAP_PREF_COPY,
		IDS_FF_PLUG_CAP_PREF_BOTH
	};
	static const CAPINFO CapInfo[] = {
		{FF_CAP_16BITVIDEO,			IDS_FF_PLUG_CAP_16BIT,			2,	YesNo},
		{FF_CAP_24BITVIDEO,			IDS_FF_PLUG_CAP_24BIT,			2,	YesNo},
		{FF_CAP_32BITVIDEO,			IDS_FF_PLUG_CAP_32BIT,			2,	YesNo},
		{FF_CAP_PROCESSFRAMECOPY,	IDS_FF_PLUG_CAP_PROCESSCOPY,	2,	YesNo},
		{FF_CAP_MINIMUMINPUTFRAMES,	IDS_FF_PLUG_CAP_MINFRAMES},
		{FF_CAP_MAXIMUMINPUTFRAMES,	IDS_FF_PLUG_CAP_MAXFRAMES},
		{FF_CAP_COPYORINPLACE,		IDS_FF_PLUG_CAP_COPYINPLACE,	4,	CopyPref},
	};
	static const int CAPS = sizeof(CapInfo) / sizeof(CAPINFO);
	for (int i = 0; i < CAPS; i++) {	// for each capability we know about
		const CAPINFO	*pci = &CapInfo[i];
		int	CapVal = Plugin.GetPluginCaps(pci->CapIdx);	// try to get capability's value
		if (CapVal >= 0) {	// if capability is supported
			CString	CapName((LPCTSTR)pci->Name);
			if (pci->States > 0) {	// if capability has state names
				int	StateIdx = CLAMP(CapVal, 0, pci->States - 1);	// use value as index
				s.Format(_T("\n%s:\t%s"), CapName, LDS(pci->StateName[StateIdx]));
			} else	// display value as an integer
				s.Format(_T("\n%s:\t%d"), CapName, CapVal);
			msg += s;	// append line to message
		}
	}
	CString	Path;
	Plugin.GetPluginPath(Path);
	if (CMetaproject::IsMetaplugin(Path)) {	// if plugin is a metaplugin
		CMetaproject	Project;
		if (Project.Import(Path, FALSE)) {	// read without unpacking embedded plugins
			CStringArray	PluginList;
			Project.GetPluginList(PluginList);	// get list of metaplugin's plugins
			int	plugs = PluginList.GetSize();
			if (plugs) {
				// get correct caption for composition type: linked or embedded 
				int	Caption = Project.m_Metaplugin.m_IsEmbedded ?
					IDS_META_PLUGS_EMBEDDED : IDS_META_PLUGS_LINKED;
				msg += LDS(Caption);	// append composition type to message
				for (int i = 0; i < plugs; i++)		// append plugin list to message
					msg += CString("\n\t\t") + PluginList[i];	// one line per plugin
			}
		}
	}
	CMsgBoxDlg::Do(msg, PluginName + " " + LDS(IDS_FF_PROPERTIES), 
		MB_ICONINFORMATION | MB_TOPMOST);
}

bool CMainFrame::ShowPluginProps(LPCTSTR Path) const
{
	CFFPlugin	plug;
	if (!plug.Load(Path)) {
		CString	msg;
		AfxFormatString1(msg, IDS_FF_CANT_LOAD_PLUGIN, Path);
		AfxMessageBox(msg);
		return(FALSE);
	}
	ShowPluginProps(plug);
	return(TRUE);
}

bool CMainFrame::GetClipProps(LPCTSTR Path, CLIP_PROPS& Props)
{
	bool	retc = FALSE;
	ZeroMemory(&Props, sizeof(CLIP_PROPS));
	LPCTSTR	Ext = PathFindExtension(Path);
	if (!_tcsicmp(Ext, AVI_EXT) || !_tcsicmp(Ext, AVS_EXT)) {
		CAviToBmp	avi;
		if (avi.Open(Path)) {
			AVISTREAMINFO	si;
			if (avi.GetStreamInfo(si)) {
				CRect	FrameRect(si.rcFrame);
				Props.FrameSize = FrameRect.Size();
				Props.FrameCount = avi.GetFrameCount();
				Props.FrameRate = float(si.dwRate) / si.dwScale;
				Props.BitCount = avi.GetBitCount();
				Props.FCCHandler = si.fccHandler;
				retc = TRUE;
			}
		}
	} else {	// not a video; assume picture
		CPicture	pic;
		if (pic.Open(Path)) {
			HBITMAP hBmp = pic.GetHandle();
			if (hBmp != NULL) {
				BITMAP	bmp;
				if (GetObject(hBmp, sizeof(bmp), &bmp)) {
					Props.FrameSize = CSize(bmp.bmWidth, bmp.bmHeight);
					Props.FrameCount = 1;
					Props.BitCount = bmp.bmBitsPixel;
					retc = TRUE;
				}
			}
		}
	}
	return(retc);
}

bool CMainFrame::ShowClipProps(LPCTSTR Path) const
{
	CLIP_PROPS	cp;
	if (!GetClipProps(Path, cp)) {
		AfxMessageBox(IDS_MF_CANT_GET_PROPS);
		return(FALSE);
	}
	LPCTSTR	Name = PathFindFileName(Path);
	float	PlayLength = cp.FrameRate ? cp.FrameCount / cp.FrameRate : 0;
	char	HandlerBuf[sizeof(DWORD) + 1];
	memcpy(HandlerBuf, &cp.FCCHandler, sizeof(DWORD));
	HandlerBuf[sizeof(DWORD)] = 0;
	CString	Handler(HandlerBuf);
	Handler.MakeUpper();
	CString	msg;
	msg.Format(LDS(IDS_MF_CLIP_PROPS), Name, cp.FrameSize.cx, cp.FrameSize.cy, 
		PlayLength, cp.FrameCount, cp.FrameRate, cp.BitCount, Handler);
	CMsgBoxDlg::Do(msg, CString(Name) + " " + LDS(IDS_FF_PROPERTIES), 
		MB_ICONINFORMATION | MB_TOPMOST);
	return(TRUE);
}

bool CMainFrame::PromptFile(int FolderIdx, bool OpenFile, LPCTSTR DefName, int TitleID, CString& Path)
{
	ASSERT(FolderIdx >= 0 && FolderIdx < FOLDERS);
	const FOLDER_INFO&	info = m_FolderInfo[FolderIdx];
	CString	Filter((LPCTSTR)info.Filter);
	CString	Title((LPCTSTR)TitleID);
	UINT	flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CMultiFileDlg	fd(OpenFile, info.DefExt, DefName, flags, Filter,
		NULL, Title, &m_Folder[FolderIdx]);
	if (fd.DoModal() != IDOK)
		return(FALSE);
	Path = fd.GetPathName();
	if (info.PaneIdx >= 0)
		PostMessage(UWM_FOLDERCHANGE, info.PaneIdx, FolderIdx);
	return(TRUE);
}

bool CMainFrame::Record(bool Enable)
{
	if (Enable == IsRecording())
		return(TRUE);
	if (Enable) {	// if starting
		CString	path;
		if (!PromptFile(DIR_RECORD, FALSE, NULL, IDS_MF_RECORD_AS, path))
			return(FALSE);
		if (m_RecordDlg.DoModal() != IDOK)
			return(FALSE);
		CRecordInfo	RecInfo;
		m_RecordDlg.GetInfo(RecInfo);
		if (m_RecordDlg.QueueJob()) {	// if queueing to job control
			CString	TempFilePath;
			theApp.GetTempFileName(TempFilePath);
			if (m_Recorder.Open(TempFilePath, RecInfo)) {
				CJobInfo	Info;
				GetJobInfo(Info);
				Info.m_RecordPath = path;
				Info.m_Source = PathFindFileName(m_View->GetDocument()->GetPathName());
				Info.m_Dest = PathFindFileName(path);
				m_Recorder.GetComprState(Info.m_ComprState);
				m_JobCtrlDlg.Add(Info);
				m_Recorder.Close();
			}
			DeleteFile(TempFilePath);
		} else {	// record now
			if (!m_Recorder.Start(path, RecInfo))
				return(FALSE);
			m_RecStatDlg.Start();
			m_RecStatDlg.ShowWindow(SW_SHOW);	// show record status
		}
	} else {	// stopping
		if (!m_Recorder.IsDone()) {	// if recording aborted
			if (!m_BatchMode) {	// if not batch processing
				UINT	style = MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2;
				if (AfxMessageBox(IDS_MF_STOP_RECORD, style) != IDYES)
					return(FALSE);
			}
		}
		m_RecStatDlg.Stop();
		if (!m_Recorder.Stop())
			return(FALSE);
		// in batch mode we didn't show record status, so don't hide it either
		if (!m_BatchMode)
			m_RecStatDlg.ShowWindow(SW_HIDE);
	}
	return(TRUE);
}

bool CMainFrame::ShowWarning(int TemplateID, int TimeoutSecs)
{
	CProgressDlg	dlg(TemplateID);
	if (!dlg.Create())
		return(FALSE);
 	dlg.SendDlgItemMessage(IDC_WARNING_ICON, STM_SETICON,	// set warning icon
		(WPARAM)AfxGetApp()->LoadStandardIcon(IDI_EXCLAMATION), 0);
	dlg.SetWindowPos(&CWnd::wndTopMost,	// make topmost so user sees warning
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	clock_t start = clock();
	clock_t	end = start + TimeoutSecs * CLOCKS_PER_SEC;
	dlg.SetRange(start, end);
	clock_t	now;
	do {
		if (dlg.Canceled())
			return(FALSE);
		now = clock();
		dlg.SetPos(now);
		Sleep(100);
	} while (now < end);
	return(TRUE);
}

bool CMainFrame::ShutdownWarn()
{
	static const int WARN_SECS = 10;
	if (!ShowWarning(IDD_SHUTDOWN_WARN, WARN_SECS))
		return(FALSE);
	if (!theApp.SetShutdown(TRUE, EWX_SHUTDOWN | EWX_POWEROFF)) {
		CString	msg;
		msg.Format(IDS_MF_CANT_SHUTDOWN, GetLastError());
		AfxMessageBox(msg);
		return(FALSE);
	}
	SetModify(FALSE);	// prevent save changes prompt
	PostMessage(WM_CLOSE);	// exit application gracefully
	return(TRUE);	// CFFRendApp::ExitInstance does actual shutdown
}

void CMainFrame::GetJobInfo(CJobInfo& Info)
{
	GetProject(Info.m_Project);	// get project data
	m_Recorder.GetComprState(Info.m_ComprState);	// get compression state
	m_OptsDlg.GetEngineOptions(Info.m_EngineOpts);	// get engine options info
	m_RecordDlg.GetInfo(Info.m_RecInfo);	// get record info
}

void CMainFrame::SetJobInfo(const CJobInfo& Info)
{
	m_Engine.RemoveAll();	// avoids needless work if ApplyOptions changes frame size 
	m_OptsDlg.SetEngineOptions(Info.m_EngineOpts);	// set engine options info
	m_RecordDlg.SetInfo(Info.m_RecInfo);	// set record info
	ApplyOptions();	// apply new options; order matters
	SetProject(Info.m_Project);	// set project data
}

void CMainFrame::SetBatchMode(bool Enable)
{
	if (Enable == m_BatchMode)
		return;	// nothing to do
	theApp.EnableChildWindows(*this, !Enable);
	CMenu	*pMenu;
	if (Enable) {
		m_BatchPrevMenu = GetMenu();
		m_BatchMenu.LoadMenu(IDR_MAIN_JOB);
		pMenu = &m_BatchMenu;
	} else {
		pMenu = m_BatchPrevMenu;
		m_BatchPrevMenu = NULL;
		m_BatchMenu.DestroyMenu();
	}
	SetMenu(pMenu);
	m_BatchMode = Enable;
}

bool CMainFrame::MsgBoxCallback(PVOID Cookie, LPCTSTR Caption, LPCTSTR MsgText)
{
	CMainFrame	*frm = (CMainFrame *)Cookie;
	if (frm->m_BatchPeek)	// if we're in peek message loop
		return(TRUE);	// assume message box isn't an error; let it stay open
	frm->m_JobCtrlDlg.FailJob(MsgText);	// use message box text as error string
	return(FALSE);	// close the message box
}

bool CMainFrame::RunBatchJobs()
{
	if (!m_JobCtrlDlg.GetJobCount())
		return(FALSE);	// nothing to do
	if (!Record(FALSE))	// can't run batch jobs while recording
		return(FALSE);
	bool	ModFlag = IsModified();	// save document modified flag
	SetModify(FALSE);	// prevent save warning
	CJobInfo	PreJobInfo;
	GetJobInfo(PreJobInfo);	// save app state
	CEngine::CPause	pause(m_Engine);	// start jobs from pause
	SetBatchMode(TRUE);	// enable batch mode
	CHookMsgBox	HookMsg;
	HookMsg.Install(MsgBoxCallback, this);	// intercept all message boxes
	int	JobIdx;
	while (m_BatchMode && (JobIdx = m_JobCtrlDlg.FindWaiting()) >= 0) {
		m_JobCtrlDlg.StartJob(JobIdx);	// do first; SetJobInfo may report an error
		CJobInfo	info;
		m_JobCtrlDlg.GetInfo(JobIdx, info);
		if (!CFFRendDoc::CheckLinks(info.m_Project, CFFRendDoc::CLA_MSGBOX))
			continue;	// job has missing file(s), skip it
		SetJobInfo(info);	// restore app state associated with this job
		if (m_JobCtrlDlg.FindRunning() < 0)	// make sure job is still running
			continue;
		CEngine::CPause	pause(m_Engine, FALSE);	// unpause engine
		if (IsRunning() && m_Recorder.Start(info.m_RecordPath, info.m_RecInfo, &info.m_ComprState)) {
			m_RecStatDlg.Start();
			UpdateBars();	// peek loop doesn't receive update UI notifications
			m_BatchPeek = TRUE;
			MSG msg;
			while (m_BatchMode && IsRecording() && GetMessage(&msg, NULL, 0, 0)) {
				DispatchMessage(&msg);
			}
			m_BatchPeek = FALSE;
			// messages may have been processed during PeekMessage loop above;
			// job could be aborted, deleted, or moved to a new list position
			m_JobCtrlDlg.FinishJob();	// don't assume JobIdx is still valid
		} else	// record failed, so job list hasn't changed
			m_JobCtrlDlg.SetStatus(JobIdx, CJobControlDlg::JOB_FAILED);
	}
	HookMsg.Remove();	// stop intercepting message boxes
	bool	Aborted = !GetBatchMode();	// if batch mode already false, assume abort
	SetBatchMode(FALSE);	// disable batch mode
	SetJobInfo(PreJobInfo);	// restore app state prior to running batch jobs
	m_JobCtrlDlg.UpdateUI();
	SetModify(ModFlag);	// restore document modified flag
	if (m_IsClosing)	// if OnClose aborted batch processing
		PostMessage(WM_CLOSE);	// close the app
	else {
		if (!Aborted && m_JobCtrlDlg.GetShutdown())	// jobs done and shutdown enabled?
			ShutdownWarn();	// shutdown computer after showing a warning dialog
	}
	return(TRUE);
}

bool CMainFrame::IsModified() const
{
	return(m_View->GetDocument()->IsModified() != 0);
}

void CMainFrame::SetModify(bool Enable)
{
	GetView()->GetDocument()->SetModifiedFlag(Enable);
}

void CMainFrame::CMyUndoManager::OnModify(bool Modified)
{
	CMainFrame	*frm = theApp.GetMain();
	if (frm->GetOptions().GetSaveChgsWarn())
		frm->SetModify(Modified);
}

void CMainFrame::ToggleWindow(CWnd& Wnd)
{
	Wnd.ShowWindow(Wnd.IsIconic() ? SW_RESTORE : 
		Wnd.IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::SetMonitorSource(int SlotIdx)
{
	m_Engine.SetMonitorSource(SlotIdx);
	m_View->OnMonitorSourceChange(SlotIdx);
}

void CMainFrame::GetMessageString(UINT nID, CString& rMessage) const
{
	if (nID >= CFFRendView::ID_PLUGIN_INPUT_FIRST 
	&& nID <= CFFRendView::ID_PLUGIN_INPUT_LAST) {
		rMessage.LoadString(IDS_MF_HINT_PLUGIN_INPUT);
		return;
	}
	if (nID >= CFFRendView::ID_MONITOR_SOURCE_FIRST 
	&& nID <= CFFRendView::ID_MONITOR_SOURCE_LAST) {
		rMessage.LoadString(IDS_MF_HINT_MONITOR_SOURCE);
		return;
	}
	CFrameWnd::GetMessageString(nID, rMessage);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message map

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_APP_CANCEL, OnAppCancel)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_EDIT_OPTIONS, OnEditOptions)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_JOB_CONTROL, OnFileJobControl)
	ON_COMMAND(ID_FILE_META_EXPORT, OnFileMetaExport)
	ON_COMMAND(ID_FILE_META_IMPORT, OnFileMetaImport)
	ON_COMMAND(ID_FILE_META_PROPS, OnFileMetaProps)
	ON_COMMAND(ID_FILE_RECORD, OnFileRecord)
	ON_COMMAND(ID_FILE_VIDEO_OPEN, OnFileVideoOpen)
	ON_WM_INITMENUPOPUP()
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT, OnUpdateFileExport)
	ON_UPDATE_COMMAND_UI(ID_FILE_JOB_CONTROL, OnUpdateFileJobControl)
	ON_UPDATE_COMMAND_UI(ID_FILE_RECORD, OnUpdateFileRecord)
	ON_UPDATE_COMMAND_UI(ID_FILE_VIDEO_MRU_FILE1, OnUpdateFileVideoMruFile)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILES, OnUpdateViewFilesBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRAPH, OnUpdateViewGraph)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HISTORY, OnUpdateViewHistory)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MASTER, OnUpdateViewMaster)
	ON_UPDATE_COMMAND_UI(ID_VIEW_METAPARM, OnUpdateViewMetaparm)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MIDI_SETUP, OnUpdateViewMidi)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MONITOR, OnUpdateViewMonitor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, OnUpdateViewOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PATCH, OnUpdateViewPatch)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QUEUES, OnUpdateViewQueues)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RECORD_STATUS, OnUpdateViewRecordStatus)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_FULL_SCREEN, OnUpdateWindowFullScreen)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PAUSE, OnUpdateWindowPause)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_STEP, OnUpdateWindowStep)
	ON_COMMAND(ID_VIEW_FILES, OnViewFilesBar)
	ON_COMMAND(ID_VIEW_GRAPH, OnViewGraph)
	ON_COMMAND(ID_VIEW_HISTORY, OnViewHistory)
	ON_COMMAND(ID_VIEW_MASTER, OnViewMaster)
	ON_COMMAND(ID_VIEW_METAPARM, OnViewMetaparm)
	ON_COMMAND(ID_VIEW_MIDI_SETUP, OnViewMidiSetup)
	ON_COMMAND(ID_VIEW_MONITOR, OnViewMonitor)
	ON_COMMAND(ID_VIEW_OUTPUT, OnViewOutput)
	ON_COMMAND(ID_VIEW_PATCH, OnViewPatch)
	ON_COMMAND(ID_VIEW_QUEUES, OnViewQueues)
	ON_COMMAND(ID_VIEW_RECORD_STATUS, OnViewRecordStatus)
	ON_COMMAND(ID_WINDOW_FULL_SCREEN, OnWindowFullScreen)
	ON_COMMAND(ID_WINDOW_PAUSE, OnWindowPause)
	ON_COMMAND(ID_WINDOW_STEP, OnWindowStep)
	ON_COMMAND(ID_VIEW_LOAD_BALANCE, OnViewLoadBalance)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOAD_BALANCE, OnUpdateViewLoadBalance)
	ON_COMMAND(ID_EDIT_PLAYLIST, OnEditPlaylist)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PLAYLIST, OnUpdateEditPlaylist)
	ON_COMMAND(ID_HELP_CHECK_FOR_UPDATES, OnHelpCheckForUpdates)
	//}}AFX_MSG_MAP
	ON_WM_ACTIVATEAPP()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_MESSAGE(UWM_HANDLEDLGKEY, OnHandleDlgKey)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FRAME_SIZE, OnUpdateIndicatorFrameSize)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TARGET_FRAME_RATE, OnUpdateIndicatorTargetFrameRate)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_PAUSE, OnUpdateIndicatorPause)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RECORD, OnUpdateIndicatorRecord)
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(UWM_ENGINESTALL, OnEngineStall)
	ON_MESSAGE(UWM_GRAPHDONE, OnGraphDone)
	ON_NOTIFY(FBLCN_OPENITEM, IDC_FB_LIST, OnFBOpenItem)
	ON_NOTIFY(FBLCN_RENAMEITEM, IDC_FB_LIST, OnFBRenameItem)
	ON_NOTIFY(FBLCN_DRAGEND, IDC_FB_LIST, OnFBDragEnd)
	ON_COMMAND(ID_FB_FILE_OPEN, OnFBFileOpen)
	ON_COMMAND(ID_FB_FILE_PROPS, OnFBFileProps)
	ON_MESSAGE(UWM_MIDIIN, OnMidiIn)
	ON_MESSAGE(UWM_FOLDERCHANGE, OnFolderChange)
	ON_COMMAND_RANGE(ID_FILE_VIDEO_MRU_FILE1, ID_FILE_VIDEO_MRU_FILE4, OnFileVideoMruFile)
	ON_MESSAGE(UWM_ENDRECORD, OnEndRecord)
	ON_MESSAGE(UWM_APPUPDATEINFO, OnAppUpdateInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnDestroy() 
{
	CPersist::SaveWnd(REG_SETTINGS, this, RK_MAIN_FRAME);
	CFrameWnd::OnDestroy();
}

BOOL CMainFrame::DestroyWindow() 
{
	m_ShowOutput = m_Renderer.IsWindowVisible() != 0;
	m_Renderer.DestroyWindow();	// not in OnDestroy, more reliable here
	m_Engine.Destroy();
	CDWordArray	RegDockStyle;
	RegDockStyle.SetSize(SIZING_BARS);
	for (int i = 0; i < SIZING_BARS; i++) {
		CSizingControlBar	*bar = m_SizingBar[i];
		bar->SaveState(REG_SETTINGS);
		RegDockStyle[i] = bar->m_dwDockStyle;
	}
	SaveBarState(REG_SETTINGS);
	CPersist::WriteBinary(REG_SETTINGS, RK_BAR_DOCK_STYLE, 
		RegDockStyle.GetData(), INT64TO32(RegDockStyle.GetSize()) * sizeof(DWORD));
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnClose() 
{
	FullScreen(FALSE);	// exit full screen
	if (GetBatchMode()) {
		if (AfxMessageBox(IDS_MF_ABORT_JOBS, MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return;
		m_JobCtrlDlg.Abort();	// abort batch processing
		// assume we were called from RunBatchJobs peek message loop;
		// closing main window would crash app, so set flag instead
		m_IsClosing = TRUE;	// RunBatchJobs checks this flag and closes app
		return;	
	}
	if (!Record(FALSE))
		return;
	if (!m_PlaylistDlg.SaveCheck())
		return;
	m_IsClosing = TRUE;
	if (!IsIconic())	// if iconic, OnSysCommand already saved states
		m_ShowToolDlg = GetToolDlgState();
	CFrameWnd::OnClose();
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	if (bShow && !m_WasShown && !IsWindowVisible()) {
		m_WasShown = TRUE;
		CPersist::LoadWnd(REG_SETTINGS, this, RK_MAIN_FRAME, CPersist::NO_MINIMIZE);
		SetToolDlgState(m_ShowToolDlg);
	}
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	// if minimizing, grab tool dialog state before OnSysCommand changes it
	if ((nID & 0xFFF0) == SC_MINIMIZE)	// mask off unreliable low nibble
		m_ShowToolDlg = GetToolDlgState();
	CFrameWnd::OnSysCommand(nID, lParam);
}

void CMainFrame::OnActivateApp(BOOL bActive, ACTIVATEAPPTASK hTask) 
{
	CFrameWnd::OnActivateApp(bActive, hTask);
	if (!bActive)
		FullScreen(FALSE);
}

LRESULT CMainFrame::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	if (IsFullScreen())
		FullScreen(FALSE);
	else
		m_Renderer.CreateSurfaces();
	m_Engine.SetColorDepth(CheckColorDepth());
	return(0);
}

LRESULT	CMainFrame::OnHandleDlgKey(WPARAM wParam, LPARAM lParam)
{
	return(theApp.HandleDlgKeyMsg((MSG *)wParam));
}

void CMainFrame::OnUpdateIndicatorPause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPaused());
}

void CMainFrame::OnUpdateIndicatorRecord(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsRecording());
}

void CMainFrame::OnUpdateIndicatorTargetFrameRate(CCmdUI *pCmdUI)
{
	CString	s;
	s.Format(_T("%.2f"), m_OptsDlg.GetFrameRate());
	pCmdUI->SetText(s);
}

void CMainFrame::OnUpdateIndicatorFrameSize(CCmdUI *pCmdUI)
{
	CString	s;
	CSize	FrameSize = m_Engine.GetFrameSize();
	s.Format(_T("%d x %d"), FrameSize.cx, FrameSize.cy);
	pCmdUI->SetText(s);
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case 0:	// menu separator
		lParam = LPARAM(_T(""));	// empty message
		break;
	case AFX_IDS_IDLEMESSAGE:	// display our status message for these IDs
	case IDC_PATCH_VIEW:
	case IDC_FB_LIST:
	case IDC_FB_TAB:
	case IDC_MONITOR_WND:
	case IDC_METAPARM_VIEW:
	case IDC_GRAPH_VIEW:
	case IDC_HISTORY_VIEW:
	case IDC_QUEUE_VIEW:
		wParam = 0;
		lParam = (LPARAM)m_StatusMsg.GetBuffer(0);
		break;
	case AFX_IDW_PANE_FIRST:	// 0xE900 from control bar; ignore it
		return FALSE;
	}
	return CFrameWnd::OnSetMessageString(wParam, lParam);
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	if (nIndex == MENU_PLUGIN) {	// if showing plugin menu
		if (m_InputPopupPos < 0) {		// if input popup not situated
			// search for input popup's placeholder
			m_InputPopupPos = theApp.FindMenuItem(pPopupMenu, ID_PLUGIN_INPUT);
			if (m_InputPopupPos >= 0)	// if placeholder was found
				m_InputPopups = 1;		// count it for removal below
		}
		if (m_InputPopupPos >= 0) {		// if valid input popup position
			// remove any previous input popups, and insert fresh ones
			for (int i = 0; i < m_InputPopups; i++)
				pPopupMenu->RemoveMenu(m_InputPopupPos, MF_BYPOSITION);
			CPoint	pt;
			GetCursorPos(&pt);
			int	SlotIdx = FindSlot(pt);
			m_InputPopups = m_View->InsertInputPopups(pPopupMenu, 
				m_InputPopupPos, SlotIdx);
		}
	}
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

LRESULT CMainFrame::OnEngineStall(WPARAM wParam, LPARAM lParam)
{
#ifdef ENGINE_NATTER
	CEngineThread	*Staller = (CEngineThread *)wParam;
	STALL_INFO	StallInfo;
	Staller->GetStallInfo(StallInfo);
	m_Engine.DumpStallInfo(stdout, _T("stall"), StallInfo);
#endif
	Run(FALSE);	// stop engine before displaying message
	AfxMessageBox(IDS_MF_ENGINE_STALLED);
	Pause(TRUE);	// enter pause; unpause restarts engine
	return(0);
}

LRESULT CMainFrame::OnGraphDone(WPARAM wParam, LPARAM lParam)
{
	m_GraphBar.OnGraphDone((LPTSTR)wParam);
	return(0);
}

void CMainFrame::OnTimer(W64UINT nIDEvent) 
{
	switch (nIDEvent) {
	case FRAME_TIMER_ID:
		{
			double	t = m_FrameBench.Elapsed();
			UINT	count = m_Renderer.GetFrameCounter();
			if (count >= m_PrevFrameCount) {
				m_FrameBench.Reset();	// reset timer ASAP
				m_ActualFrameRate = float((count - m_PrevFrameCount) / t);
				CString	s;
				s.Format(_T("%.2f"), m_ActualFrameRate);
				m_StatusBar.SetPaneText(SBP_ACTUAL_FRAME_RATE, s);
			}
			m_PrevFrameCount = count;
			CString	s;
			CRecordDlg::FrameToTime(count, m_OptsDlg.GetFrameRate(), s);
			m_StatusMsg.Format(_T("%s (%d)"), s, count);
			if (!IsTracking())	// if we're not in a menu
				m_StatusBar.SetPaneText(0, m_StatusMsg);	// update status bar
			if (IsRecording() && m_Recorder.IsRunning()) {
				m_RecStatDlg.TimerHook();
				if (m_BatchMode) {
					// calculate progress in frames
					const CRecordInfo& info = m_Recorder.GetInfo();
					int	pct = info.m_FrameCount ? round(
						m_Recorder.GetFrameCounter() * 100.0 / info.m_FrameCount) : 0;
					m_JobCtrlDlg.SetProgressPos(pct);
				}
			}
			if (m_LoadBalanceBar.FastIsVisible())
				m_LoadBalanceBar.TimerHook();
		}
		break;
	case VIEW_TIMER_ID:
		{
			if (m_MonitorBar.FastIsVisible())
				m_MonitorBar.TimerHook();
			if (m_QueueBar.FastIsVisible())
				m_QueueBar.TimerHook();
			if (m_HistoryBar.FastIsVisible())
				m_HistoryBar.TimerHook();
			int	CurSel = m_Engine.GetCurSel();
			if (CurSel >= 0 && m_Engine.IsLoaded(CurSel)) {
				CFFPluginEx	*plug = m_Engine.GetSlot(CurSel);
				int	parms = plug->GetParmCount();
				for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
					if (plug->IsModulating(ParmIdx)) {
						float	val = plug->GetParmVal(ParmIdx);
						m_View->GetRow(ParmIdx)->SetVal(val);
					}
				}
			}
		}
		break;
	default:
		CFrameWnd::OnTimer(nIDEvent);
	}
}

void CMainFrame::OnFBOpenItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMFBOPENITEM pnmoi = (LPNMFBOPENITEM)pNMHDR;
	LPCTSTR	ItemPath = pnmoi->pszPath;
	if (pnmoi->bIsDir) {
		int	sel = m_FilesBar.GetCurPane();
		if (sel >= 0 && sel < FOLDERS)
			m_Folder[sel] = ItemPath;
	} else {
		CStringArray	PathList;
		PathList.Add(ItemPath);
		DropFiles(m_Engine.GetCurSel(), PathList);
	};
}

void CMainFrame::OnFBRenameItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	// rename not implemented
}

void CMainFrame::OnFBDragEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW	pnmlv = (LPNMLISTVIEW)pNMHDR;
	CPoint	pt(pnmlv->ptAction);
	CRect	r;
	m_FilesBar.GetWindowRect(r);
	if (!r.PtInRect(pt)) {	// if we're not over file browser
		CStringArray	PathList;
		if (m_FilesBar.GetCurPane() >= 0) {
			m_FilesBar.GetCurList()->GetSelectedItems(PathList);
			int	SlotIdx = FindSlot(pt);
			DropFiles(SlotIdx, PathList);
		}
	}
}

void CMainFrame::OnFBFileOpen()
{
	if (m_FilesBar.GetCurPane() >= 0)
		m_FilesBar.GetCurList()->OpenSelectedItems();
}

void CMainFrame::OnFBFileProps()
{
	CFileBrowserListCtrl	*lp = m_FilesBar.GetCurList();
	CDirItemArray	DirItem;
	lp->GetSelectedItems(DirItem);
	LPCTSTR	Folder = lp->GetFolder();
	int	items = DirItem.GetSize();
	for (int i = 0; i < items; i++) {
		if (!DirItem[i].IsDir()) {	// ignore folders
			CString	Path = DirItem[i].GetPath(Folder);
			switch (m_FilesBar.GetCurPane()) {
			case CFileBrowserFF::PANE_PLUGINS:
				ShowPluginProps(Path);
				break;
			case CFileBrowserFF::PANE_CLIPS:
				ShowClipProps(Path);
				break;
			}
		}
	}
}

LRESULT CMainFrame::OnFolderChange(WPARAM wParam, LPARAM lParam)
{
	ASSERT(wParam < CFileBrowserFF::PANES);
	m_FilesBar.SetFolder(wParam, GetFolder(lParam));
	return(0);
}

void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	UINT	Files = DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);
	TCHAR	Path[MAX_PATH];
	CStringArray	PathList;
	for (UINT i = 0; i < Files; i++) {
		DragQueryFile(hDropInfo, i, Path, MAX_PATH);
		PathList.Add(Path);
	};
	DragFinish(hDropInfo);	// release memory
	CPoint	pt;
	GetCursorPos(&pt);	// screen coords
	int	SlotIdx = FindSlot(pt);
	DropFiles(SlotIdx, PathList);
	SetForegroundWindow();
}

LRESULT CMainFrame::OnMidiIn(WPARAM wParam, LPARAM lParam)
{
	// pass to setup dialog first, so learned assignments take effect immediately
	if (m_MidiSetupBar.FastIsVisible())	// only if bar is visible
		m_MidiSetupBar.GetDlg().SendMessage(UWM_MIDIIN, wParam);
	MIDI_MSG	msg;
	msg.dw = wParam;
	switch (msg.s.cmd & 0xf0) {
	case MC_NOTE_ON:
		if (!msg.s.p2)	// ignore zero velocity note off
			return(TRUE);
		m_Engine.SetMidiProperty(MET_NOTE, msg);
		break;
	case MC_KEY_AFT:
		break;
	case MC_CTRL_CHG:
		m_Engine.SetMidiProperty(MET_CTRL, msg);
		break;
	case MC_PROG_CHG:
		break;
	case MC_CHAN_AFT:
		break;
	case MC_PITCH_BEND:
		m_Engine.SetMidiProperty(MET_PITCH, msg);
		break;
	}
	return(TRUE);
}

void CALLBACK CMainFrame::MidiCallback(HMIDIIN handle, UINT uMsg, DWORD dwInstance,
								  DWORD dwParam1, DWORD dwParam2)
{
	// This function runs in a system thread, so beware of data corruption;
	// also avoid calling any system functions except for those listed in the
	// SDK under MidiInProc, and keep processing time to the absolute minimum.
	if (uMsg != MIM_DATA)
		return;
	if ((dwParam1 & 0xff) < 0xf0) // if channel message
		::PostMessage(theApp.GetMain()->m_hWnd, UWM_MIDIIN, dwParam1, 0);
}

LRESULT CMainFrame::OnEndRecord(WPARAM wParam, LPARAM lParam)
{
	Record(FALSE);
	if (m_BatchMode)
		m_JobCtrlDlg.SetProgressPos(100);
	if (m_RecStatDlg.Shutdown())
		ShutdownWarn();
	return(0);
}

LRESULT CMainFrame::OnAppUpdateInfo(WPARAM wParam, LPARAM lParam)
{
	CUpdateCheck::OnAppUpdateInfo(wParam, lParam);
	return(0);
}

void CMainFrame::OnAppCancel() 
{
	if (IsSingleMonitorExclusive())
		FullScreen(FALSE);
	else {
		m_View->CancelDrag();
		m_PatchBar.CancelDrag();
		m_MetaparmBar.CancelDrag();
		if (m_FilesBar.GetCurPane() >= 0)
			m_FilesBar.GetCurList()->CancelDrag();
	}
}

void CMainFrame::OnFileVideoOpen() 
{
	CString	path;
	if (PromptFile(DIR_CLIPS, TRUE, NULL, IDS_MF_OPEN_VIDEO, path))
		OpenClip(-1, path);
}

void CMainFrame::OnFileVideoMruFile(UINT nID) 
{
	int	MruIdx = nID - ID_FILE_VIDEO_MRU_FILE1;
	ASSERT(MruIdx >= 0 && MruIdx < MAX_RECENT_CLIPS);
	if (!m_Engine.OpenClip(-1, m_RecentClip[MruIdx]))
		m_RecentClip.Remove(MruIdx);
}

void CMainFrame::OnUpdateFileVideoMruFile(CCmdUI* pCmdUI) 
{
	m_RecentClip.UpdateMenu(pCmdUI);
}

void CMainFrame::OnFileExport() 
{
	CString	path;
	if (PromptFile(DIR_EXPORT, FALSE, NULL, IDS_MF_EXPORT, path)) {
		CFileException	e;
		if (!m_Renderer.ExportBitmap(path, 72, &e))
			e.ReportError();
	}
}

void CMainFrame::OnUpdateFileExport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Engine.GetPluginCount());
}

void CMainFrame::OnFileRecord() 
{
	Record(!IsRecording());
}

void CMainFrame::OnUpdateFileRecord(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsRecording());
}

void CMainFrame::OnFileJobControl() 
{
	ToggleWindow(m_JobCtrlDlg);
}

void CMainFrame::OnUpdateFileJobControl(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_JobCtrlDlg.FastIsVisible());
}

void CMainFrame::OnFileMetaExport() 
{
	CString	path(m_View->GetDocument()->GetPathName());
	CPathStr	DefName;
	if (!path.IsEmpty()) {	// set default file name
		DefName = PathFindFileName(path);	// same as project file name
		DefName.RenameExtension(PLUGIN_EXT);	// but with plugin extension
	}
	if (PromptFile(DIR_PLUGINS, FALSE, DefName, IDS_META_EXPORT, path)) {
		CMetaplugPropsDlg	dlg(m_Engine.GetMetaplugin(), PathFindFileName(path));
		if (dlg.DoModal() == IDOK) {
			if (dlg.GetModified())
				SetModify();
			if (ExportMetaplugin(path))
				// files bar pane was already refreshed (too early) during
				// metaplugin properties dialog's DoModal; refresh it again
				m_FilesBar.GetList(CFileBrowserFF::PANE_PLUGINS).Refresh();
		}
	}
}

void CMainFrame::OnFileMetaImport() 
{
	CString	path;
	if (PromptFile(DIR_PLUGINS, TRUE, NULL, IDS_META_IMPORT, path)) {
		SendMessage(WM_COMMAND, ID_FILE_NEW);
		if (!IsModified())
			ImportMetaplugin(path);
	}
}

void CMainFrame::OnFileMetaProps() 
{
	CString	DocPath(m_View->GetDocument()->GetPathName());
	CMetaplugPropsDlg	dlg(m_Engine.GetMetaplugin(), PathFindFileName(DocPath));
	if (dlg.DoModal() == IDOK) {
		if (dlg.GetModified())
			SetModify();
	}
}

void CMainFrame::OnEditUndo() 
{
	m_UndoMgr.Undo();
}

void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	CString	Text;
	Text.Format(LDS(IDS_EDIT_UNDO_FMT), m_UndoMgr.GetUndoTitle());
	pCmdUI->SetText(Text);
	pCmdUI->Enable(m_UndoMgr.CanUndo());
}

void CMainFrame::OnEditRedo() 
{
	m_UndoMgr.Redo();
}

void CMainFrame::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	CString	Text;
	Text.Format(LDS(IDS_EDIT_REDO_FMT), m_UndoMgr.GetRedoTitle());
	pCmdUI->SetText(Text);
	pCmdUI->Enable(m_UndoMgr.CanRedo());
}

void CMainFrame::OnEditOptions() 
{
	if (m_OptsDlg.DoModal() == IDOK) {
		UpdateWindow();	// clean up dialog in case ApplyOptions is slow
		ApplyOptions();
	}
}

void CMainFrame::OnEditPlaylist() 
{
	ToggleWindow(m_PlaylistDlg);
}

void CMainFrame::OnUpdateEditPlaylist(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PlaylistDlg.FastIsVisible());
}

void CMainFrame::OnViewOutput() 
{
	m_Renderer.ShowWindow(m_Renderer.IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

void CMainFrame::OnUpdateViewOutput(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Renderer.IsWindowVisible());
}

void CMainFrame::OnViewMaster() 
{
	ShowControlBar(&m_MasterBar, !m_MasterBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewMaster(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MasterBar.FastIsVisible());
}

void CMainFrame::OnViewPatch() 
{
	ShowControlBar(&m_PatchBar, !m_PatchBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewPatch(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_PatchBar.FastIsVisible());
}

void CMainFrame::OnViewFilesBar() 
{
	ShowControlBar(&m_FilesBar, !m_FilesBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewFilesBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_FilesBar.FastIsVisible());
}

void CMainFrame::OnViewMonitor() 
{
	ShowControlBar(&m_MonitorBar, !m_MonitorBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewMonitor(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MonitorBar.FastIsVisible());
}

void CMainFrame::OnViewMidiSetup() 
{
	ShowControlBar(&m_MidiSetupBar, !m_MidiSetupBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewMidi(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MidiSetupBar.FastIsVisible());
}

void CMainFrame::OnViewMetaparm() 
{
	ShowControlBar(&m_MetaparmBar, !m_MetaparmBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewMetaparm(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_MetaparmBar.FastIsVisible());
}

void CMainFrame::OnViewRecordStatus() 
{
	ToggleWindow(m_RecStatDlg);
}

void CMainFrame::OnUpdateViewRecordStatus(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_RecStatDlg.FastIsVisible());
}

void CMainFrame::OnViewGraph() 
{
	ShowControlBar(&m_GraphBar, !m_GraphBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewGraph(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_GraphBar.FastIsVisible());
}

void CMainFrame::OnViewHistory() 
{
	ShowControlBar(&m_HistoryBar, !m_HistoryBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewHistory(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_HistoryBar.FastIsVisible());
}

void CMainFrame::OnViewQueues() 
{
	ShowControlBar(&m_QueueBar, !m_QueueBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewQueues(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_QueueBar.FastIsVisible());
}

void CMainFrame::OnViewLoadBalance() 
{
	ShowControlBar(&m_LoadBalanceBar, !m_LoadBalanceBar.FastIsVisible(), 0);
}

void CMainFrame::OnUpdateViewLoadBalance(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_LoadBalanceBar.FastIsVisible());
}

void CMainFrame::OnWindowFullScreen() 
{
	FullScreen(!IsFullScreen());
}

void CMainFrame::OnUpdateWindowFullScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsFullScreen());
}

void CMainFrame::OnWindowPause() 
{
	Pause(!IsPaused());
}

void CMainFrame::OnUpdateWindowPause(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsPaused());
}

void CMainFrame::OnWindowStep() 
{
	m_Renderer.SingleStep();
	if (m_MonitorBar.FastIsVisible())
		m_MonitorBar.UpdateView();
	OnTimer(FRAME_TIMER_ID);
	OnTimer(VIEW_TIMER_ID);
}

void CMainFrame::OnUpdateWindowStep(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsPaused());
}

void CMainFrame::OnHelpCheckForUpdates() 
{
	CUpdateCheck::Check();
}
