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

        graph bar
 
*/

// GraphBar.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "GraphBar.h"
#include "GraphView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphBar

IMPLEMENT_DYNAMIC(CGraphBar, CMySizingControlBar);

CGraphBar::CGraphBar()
{
	m_View = NULL;
}

CGraphBar::~CGraphBar()
{
}

void CGraphBar::UpdateView()
{
	if (FastIsVisible())
		m_View->UpdateView();
}

/////////////////////////////////////////////////////////////////////////////
// CGraphBar message map

BEGIN_MESSAGE_MAP(CGraphBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CGraphBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphBar message handlers

int CGraphBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CGraphView);
	m_View = DYNAMIC_DOWNCAST(CGraphView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_GRAPH_VIEW, NULL))
		return -1;
		
	return 0;
}

void CGraphBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_View->MoveWindow(0, 0, cx, cy);
}

void CGraphBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	// we can't call UpdateView here, because the view's size may not be valid
	// yet, e.g. if we're being shown for the first time, the view's width and
	// height are both 0x7FFF until OnSize runs; instead we post a message, to
	// delay the update and give the view a chance to resize itself first
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		m_View->DelayedUpdate();
}
