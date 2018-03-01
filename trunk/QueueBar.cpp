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
		02		18nov11	inline UpdateView

        queue bar
 
*/

// QueueBar.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "QueueBar.h"
#include "QueueView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueueBar

IMPLEMENT_DYNAMIC(CQueueBar, CMySizingControlBar);

CQueueBar::CQueueBar()
{
	m_View = NULL;
}

CQueueBar::~CQueueBar()
{
}

/////////////////////////////////////////////////////////////////////////////
// CQueueBar message map

BEGIN_MESSAGE_MAP(CQueueBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CQueueBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueueBar message handlers

int CQueueBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CQueueView);
	m_View = DYNAMIC_DOWNCAST(CQueueView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_QUEUE_VIEW, NULL))
		return -1;
		
	return 0;
}

void CQueueBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_View->MoveWindow(0, 0, cx, cy);
}

void CQueueBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		UpdateView();
}
