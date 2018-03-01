// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		29apr11	in HandleDlgKeyMsg, add escape key
		02		19jul11	in InitInstance, call FixMFCDotBitmap
        03      21nov11	optionally open playlist
        04      22nov11	override DoWaitCursor
        05      23nov11	add full screen command line flag
		06		27mar12	add ProcessMessageFilter to fix edit box/menu pauses
		07		12apr12	in HandleDlgKeyMsg, use GetKeyState, not async
		08		13apr12	remove COM MTA init

		FFRend application
 
*/

// FFRend.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "FFRend.h"
#include "AboutDlg.h"

#include "MainFrm.h"
#include "FFRendDoc.h"
#include "FFRendView.h"
#include "Win32Console.h"
#include "FocusEdit.h"
#include "htmlhelp.h"	// needed for HTML Help API
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const LPCTSTR CFFRendApp::CMyCmdLineInfo::m_FlagName[CL_FLAGS] = {
	_T("fullscreen"),	// CLF_FULL_SCREEN
};

/////////////////////////////////////////////////////////////////////////////
// CFFRendApp

BEGIN_MESSAGE_MAP(CFFRendApp, CWinAppEx)
	//{{AFX_MSG_MAP(CFFRendApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_HOME_PAGE, OnAppHomePage)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFRendApp construction

CFFRendApp::CFFRendApp()
{
	// Place all significant initialization in InitInstance
	m_bExitWindows = FALSE;
	m_uExitFlags = 0;
	m_HelpInit = FALSE;
	HCURSOR	hWaitCursor = LoadStandardCursor(IDC_WAIT);
	m_hWaitCursor = hWaitCursor;
	m_hWaitCursorBackup = hWaitCursor;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFFRendApp object

CFFRendApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFFRendApp initialization

CFFRendApp::CMyCmdLineInfo::CMyCmdLineInfo()
{
	m_PrevFlag = -1;
	m_FullScreen = FALSE;
}

void CFFRendApp::CMyCmdLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	CCommandLineInfo::ParseParam(lpszParam, bFlag, bLast);
	if (bFlag) {
		int	i;
		for (i = 0; i < CL_FLAGS; i++) {
			if (!_tcsicmp(lpszParam, m_FlagName[i]))
				break;
		}
		if (i < CL_FLAGS) {	// if flag name was found
			switch (i) {
			case CLF_FULL_SCREEN:
				m_FullScreen = TRUE;
				break;
			}
			m_PrevFlag = i;
		} else {	// unknown flag
			m_PrevFlag = -1;
		}
	} else {
		m_PrevFlag = -1;
	}
}

BOOL CFFRendApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

#ifdef _DEBUG
	Win32Console::Create();	// create console window
#endif

	// override document manager so we can specialize file dialog
	m_pDocManager = new CMyDocManager();

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Anal Software"));
	// Change profile name to avoid conflict with version 1 registry settings.
	free((void *)m_pszProfileName);	// Free the profile string allocated by MFC
	m_pszProfileName = _tcsdup(_T("FFRend2"));	// CWinApp dtor will free this

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	FixMFCDotBitmap();	// replace wimpy SetRadio dot bitmap with a bigger one

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CFFRendDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CFFRendView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CFFRendApp::CMyCmdLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// if opening a playlist, set flag and force shell command to new
	bool	OpeningPlaylist;
	CString	ext(PathFindExtension(cmdInfo.m_strFileName));
	if (!ext.CompareNoCase(PLAYLIST_EXT)) {
		OpeningPlaylist = TRUE;
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
	} else
		OpeningPlaylist = FALSE;

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// if opening a playlist, open and start playing it
	if (OpeningPlaylist) {
		CPlaylistDlg&	pd = GetMain()->GetPlaylistDlg();
		pd.ShowWindow(SW_SHOWNORMAL);	// show playlist dialog to avoid confusion
		if (pd.Open(cmdInfo.m_strFileName))	// if playlist opens
			pd.SetPlay(TRUE);	// start playing it
	}
	if (cmdInfo.m_FullScreen)	// if full-screen flag was specified
		GetMain()->FullScreen(TRUE);	// go full-screen
	return TRUE;
}

int CFFRendApp::ExitInstance() 
{
	// if HTML help was initialized, close all topics
	if (m_HelpInit)
		::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
	if (m_bExitWindows)
		ExitWindowsEx(m_uExitFlags, 0);	// asynchronous shutdown
	return CWinAppEx::ExitInstance();
}

void CFFRendApp::UpdateMenu(CWnd *pWnd, CMenu *pMenu)
{
	CCmdUI	cui;
	cui.m_pMenu = pMenu;
	cui.m_nIndexMax = pMenu->GetMenuItemCount();
	for (UINT i = 0; i < cui.m_nIndexMax; i++) {
		cui.m_nID = pMenu->GetMenuItemID(i);
		if (!cui.m_nID)	// separator
			continue;
		if (cui.m_nID == -1) {	// popup submenu
			CMenu	*pSubMenu = pMenu->GetSubMenu(i);
			if (pSubMenu != NULL)
				UpdateMenu(pWnd, pSubMenu);	// recursive call
		}
		cui.m_nIndex = i;
		cui.m_pMenu = pMenu;
		cui.DoUpdate(pWnd, FALSE);
	}
}

int	CFFRendApp::FindMenuItem(const CMenu *Menu, UINT ItemID)
{
	int	items = Menu->GetMenuItemCount();
	for (int i = 0; i < items; i++) {
		if (Menu->GetMenuItemID(i) == ItemID)
			return(i);	// return item's position
	}
	return(-1);
}

DWORD CFFRendApp::GetDisplayColorDepth()
{
	DEVMODE	dm;
	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
		return(dm.dmBitsPerPel);
	return(0);
}

void CFFRendApp::WinHelp(DWORD dwData, UINT nCmd) 
{
	static const LPCTSTR	HELP_FILE_NAME = _T("FFRend.chm");
	// if HTML help hasn't been initialized yet, initialize it
	HWND	hMainWnd = m_pMainWnd->m_hWnd;
	HWND	retc = ::HtmlHelp(hMainWnd, HELP_FILE_NAME, HH_DISPLAY_TOC, 0);
	if (!retc) {	// not found, try appending help file name to app path
		CPathStr	HelpPath(GetAppFolder());
		HelpPath.Append(HELP_FILE_NAME);
		retc = ::HtmlHelp(hMainWnd, HelpPath, HH_DISPLAY_TOC, 0);	// try again
		if (!retc) {	// not found, give up
			CString	s;
			AfxFormatString1(s, IDS_MF_HELP_FILE_MISSING, HELP_FILE_NAME);
			AfxMessageBox(s);
			return;
		}
	}
	m_HelpInit = TRUE;
}

// By default, CWinApp::OnIdle is called after WM_TIMER messages.  This isn't
// normally a problem, but if the application uses a short timer, OnIdle will
// be called frequently, seriously degrading performance.  Performance can be
// improved by overriding IsIdleMessage to return FALSE for WM_TIMER messages,
// which prevents them from triggering OnIdle.  This technique can be applied
// to any idle-triggering message that repeats frequently, e.g. WM_MOUSEMOVE.
//
BOOL CFFRendApp::IsIdleMessage(MSG* pMsg)
{
	if (CWinApp::IsIdleMessage(pMsg)) {
		switch (pMsg->message) {	// don't call OnIdle after these messages
		case WM_TIMER:
			switch (pMsg->wParam) {
			case CMainFrame::VIEW_TIMER_ID:
			case CMainFrame::FRAME_TIMER_ID:
				return(FALSE);
			}
			return(TRUE);
		default:
			return(TRUE);
		}
	} else
		return(FALSE);
}

bool CFFRendApp::HandleDlgKeyMsg(MSG* pMsg)
{
	static const LPCSTR	EditBoxCtrlKeys = "ACHVX";	// Z reserved for app undo
	CMainFrame	*Main = theApp.GetMain();
	ASSERT(Main != NULL);	// main frame must exist
	switch (pMsg->message) {
	case WM_KEYDOWN:
		{
			int	VKey = INT64TO32(pMsg->wParam);
			bool	bTryMainAccels = FALSE;	// assume failure
			if ((VKey >= VK_F1 && VKey <= VK_F24) || VKey == VK_ESCAPE) {
				bTryMainAccels = TRUE;	// function key or escape
			} else {
				bool	IsAlpha = VKey >= 'A' && VKey <= 'Z';
				CEdit	*pEdit = CFocusEdit::GetEdit();
				if (pEdit != NULL) {	// if an edit control has focus
					if ((IsAlpha									// if (alpha key
					&& strchr(EditBoxCtrlKeys, VKey) == NULL		// and unused by edit
					&& (GetKeyState(VK_CONTROL) & GKS_DOWN))		// and Ctrl is down)
					|| (IsAlpha										// or (alpha key
					&& pEdit->IsKindOf(RUNTIME_CLASS(CNumEdit))		// and numeric edit
					&& (GetKeyState(VK_SHIFT) & GKS_DOWN))			// and Shift is down)
					|| (VKey == VK_SPACE							// or (space key
					&& pEdit->IsKindOf(RUNTIME_CLASS(CNumEdit))))	// and numeric edit)
						bTryMainAccels = TRUE;	// give main accelerators a try
				} else {	// non-edit control has focus
					if (IsAlpha										// if alpha key
					|| VKey == VK_SPACE								// or space key
					|| (GetKeyState(VK_CONTROL) & GKS_DOWN)			// or Ctrl is down
					|| (GetKeyState(VK_SHIFT) & GKS_DOWN))			// or Shift is down
						bTryMainAccels = TRUE;	// give main accelerators a try
				}
			}
			if (bTryMainAccels) {
				HACCEL	hAccel = Main->GetAccelTable();
				if (hAccel != NULL
				&& TranslateAccelerator(Main->m_hWnd, hAccel, pMsg))
					return(TRUE);	// message was translated, stop dispatching
			}
		}
		break;
	case WM_SYSKEYDOWN:
		Main->SetFocus();	// causes main frame to display the appropriate menu
		return(TRUE);	// message was translated, stop dispatching
	}
	return(FALSE);	// continue dispatching
}

BOOL CFFRendApp::CMyDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle,
		DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
	// override document's default file prompting for open and save
	LPCTSTR	DefName = !bOpenFileDialog ? PathFindFileName(fileName) : NULL;
	return(theApp.GetMain()->PromptFile(CMainFrame::DIR_PROJECTS,
		bOpenFileDialog != 0, DefName, nIDSTitle, fileName));
}

bool CFFRendApp::GetShutdownPrivileges(UINT Flags)
{
	if (!(Flags & (EWX_POWEROFF | EWX_REBOOT | EWX_SHUTDOWN)))
		return(TRUE);	// no special privileges needed
	// try to obtain privileges needed for shutdown
	HANDLE	hToken;
	TOKEN_PRIVILEGES tkp;
	// get access token for this process
	OpenProcessToken(GetCurrentProcess(), 
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	// get LUID for shutdown privilege
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
	tkp.PrivilegeCount = 1; // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;   
	// request shutdown privilege
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	// AdjustTokenPrivileges return value gives false positives
	if (GetLastError() != ERROR_SUCCESS)
		return(FALSE);	// fail if privilege wasn't granted
	return(TRUE);
}

bool CFFRendApp::SetShutdown(bool Enable, UINT Flags)
{
	if (Enable) {
		if (!GetShutdownPrivileges(Flags))	// try to obtain shutdown privileges
			return(FALSE);	// fail if privilege wasn't granted
	}
	// ExitInstance does the actual shutdown just before the app terminates
	m_bExitWindows = Enable;
	m_uExitFlags = Flags;
	return(TRUE);
}

void CFFRendApp::DoWaitCursor(int nCode)
{
	// same as CWinApp implementation, except wait cursor is a member var
	ASSERT(nCode == 0 || nCode == 1 || nCode == -1);
	m_nWaitCursorCount += nCode;
	if (m_nWaitCursorCount > 0) {
		HCURSOR hcurPrev = ::SetCursor(m_hWaitCursor);
		if (nCode > 0 && m_nWaitCursorCount == 1)
			m_hcurWaitCursorRestore = hcurPrev;
	} else {
		// turn everything off
		m_nWaitCursorCount = 0;     // prevent underflow
		::SetCursor(m_hcurWaitCursorRestore);
	}
}

void CFFRendApp::EnableWaitCursor(bool Enable)
{
	if (Enable)
		m_hWaitCursor = m_hWaitCursorBackup;
	else	// disable wait cursor
		m_hWaitCursor = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CFFRendApp message handlers

// App command to run the dialog
void CFFRendApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CFFRendApp::OnAppHomePage() 
{
	if (!CHyperlink::GotoUrl(CAboutDlg::HOME_PAGE_URL))
		AfxMessageBox(IDS_HLINK_CANT_LAUNCH);
}

void CFFRendApp::OnHelpIndex()
{
	WinHelp(0);
}

BOOL CFFRendApp::ProcessMessageFilter(int code, LPMSG lpMsg) 
{
	// If a menu is displayed while an edit control has focus, the message loop
	// pauses periodically until the menu is closed; this applies to all menus,
	// including context and system menus, and it's a problem for timer-driven
	// apps that use edit controls.  The problem is caused by the undocumented
	// WM_SYSTIMER message (0x118), which Windows uses internally for various
	// purposes including scrolling and blinking the caret in an edit control.
	// The solution is to suppress WM_SYSTIMER, but only if the filter code is
	// MSGF_MENU, otherwise the caret won't blink while scrolling.  The caret
	// doesn't blink while a menu is displayed even without this workaround.
	//
	// if displaying a menu and message is WM_SYSTIMER
	if (code == MSGF_MENU && lpMsg->message == 0x118) {
		// use GetClassName because IsKindOf fails if the edit control doesn't
		// have a CEdit instance; see Microsoft knowledge base article Q145616
		TCHAR	szClassName[6];
		if (GetClassName(lpMsg->hwnd, szClassName, 6)
		&& !_tcsicmp(szClassName, _T("Edit"))) {	// if recipient is an edit control
			return TRUE;	// suppress WM_SYSTIMER
		}
	}
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
