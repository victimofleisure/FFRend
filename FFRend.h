// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      22nov11	override DoWaitCursor
        02      23nov11	add full screen command line flag
		03		27mar12	add ProcessMessageFilter to fix edit box/menu pauses
		04		06may12	change GetEngine to return most-derived engine

		FFRend application
 
*/

// FFRend.h : main header file for the FFREND application
//

#if !defined(AFX_FFREND_H__9B14D1CE_236B_4341_8431_5CADCAE890EC__INCLUDED_)
#define AFX_FFREND_H__9B14D1CE_236B_4341_8431_5CADCAE890EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFFRendApp:
// See FFRend.cpp for the implementation of this class
//

#include "MainFrm.h"
#include "WinAppEx.h"

class CFFRendApp : public CWinAppEx
{
public:
// Construction
	CFFRendApp();

// Attributes
	CMainFrame	*GetMain();
	CFFRendView	*GetView();
	CMainEngine&	GetEngine();
	static	DWORD	GetDisplayColorDepth();

// Operations
	static	void	UpdateMenu(CWnd *pWnd, CMenu *pMenu);
	static	int		FindMenuItem(const CMenu *Menu, UINT ItemID);
	static	bool	HandleDlgKeyMsg(MSG* pMsg);
	void	FastBeginWaitCursor();
	void	FastEndWaitCursor();
	static	bool	GetShutdownPrivileges(UINT Flags);
	bool	SetShutdown(bool Enable, UINT Flags);
	void	EnableWaitCursor(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFRendApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL IsIdleMessage(MSG* pMsg);
	virtual int ExitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	//}}AFX_VIRTUAL
	virtual void DoWaitCursor(int nCode);
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);

// Implementation
	//{{AFX_MSG(CFFRendApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppHomePage();
	afx_msg void OnHelpIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
// Nested types
	class CMyCmdLineInfo : public CCommandLineInfo {
	public:
	// Construction
		CMyCmdLineInfo();

	// Public data
		bool	m_FullScreen;	// if true, start in full screen exclusive mode

	protected:
	// Constants
		enum {	// command line flags; must match m_FlagName
			CLF_FULL_SCREEN,		// start in full screen exclusive mode
			CL_FLAGS
		};
		static const LPCTSTR m_FlagName[CL_FLAGS];	// command line flag names

	// Data members
		int		m_PrevFlag;		// index of previous flag, or -1 if none

	// Overrides
		void	ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
	};

// Data members
	bool	m_bExitWindows;		// if true, exit windows from ExitInstance
	UINT	m_uExitFlags;		// flags to pass to ExitWindowsEx
	bool	m_HelpInit;			// true if help was initialized
	HCURSOR	m_hWaitCursor;		// wait cursor
	HCURSOR	m_hWaitCursorBackup;	// backup copy of wait cursor

// Helpers
	class CMyDocManager : public CDocManager {
		BOOL	DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
	};
	friend class CFastWaitCursor;
};

extern CFFRendApp theApp;

inline CMainFrame *CFFRendApp::GetMain()
{
	return((CMainFrame *)m_pMainWnd);
}

inline CFFRendView *CFFRendApp::GetView()
{
	return(GetMain()->GetView());
}

inline CMainEngine& CFFRendApp::GetEngine()
{
	return(GetMain()->GetEngine());
}

inline void CFFRendApp::FastBeginWaitCursor()
{
	if (!m_nWaitCursorCount)
		DoWaitCursor(1);
	else	// avoid needlessly resetting cursor; just bump count
		m_nWaitCursorCount++;
}

inline void CFFRendApp::FastEndWaitCursor()
{
	if (m_nWaitCursorCount <= 1)
		DoWaitCursor(-1);
	else	// avoid needlessly resetting cursor; just bump count
		m_nWaitCursorCount--;
}

class CFastWaitCursor : public WObject {
public:
	CFastWaitCursor();
	~CFastWaitCursor();
};

inline CFastWaitCursor::CFastWaitCursor()
{
	theApp.FastBeginWaitCursor();
}

inline CFastWaitCursor::~CFastWaitCursor()
{
	theApp.FastEndWaitCursor();
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFREND_H__9B14D1CE_236B_4341_8431_5CADCAE890EC__INCLUDED_)
