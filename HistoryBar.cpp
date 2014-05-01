// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		26mar10	initial version
		01		29aug10	remove dirty view flag

        history bar
 
*/

// HistoryBar.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "HistoryBar.h"
#include "HistoryView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar

IMPLEMENT_DYNAMIC(CHistoryBar, CMySizingControlBar);

CHistoryBar::CHistoryBar()
{
	m_View = NULL;
}

CHistoryBar::~CHistoryBar()
{
}

void CHistoryBar::UpdateView()
{
	if (FastIsVisible())
		m_View->UpdateView();
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar message map

BEGIN_MESSAGE_MAP(CHistoryBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CHistoryBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryBar message handlers

int CHistoryBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CHistoryView);
	m_View = DYNAMIC_DOWNCAST(CHistoryView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_HISTORY_VIEW, NULL))
		return -1;
		
	return 0;
}

void CHistoryBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_View->MoveWindow(0, 0, cx, cy);
}

void CHistoryBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		TimerHook();	// also updates view; assume hook not called while we're hidden
}
