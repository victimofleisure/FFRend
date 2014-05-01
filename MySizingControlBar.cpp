// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan07	initial version
        01      18apr08	add show window handler
		02		14mar09	add style changed handler
		03		25may10	add size valid flag

        wrapper for Cristi Posea's sizable control bar
 
*/

// MySizingControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MySizingControlBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySizingControlBar dialog

IMPLEMENT_DYNAMIC(CMySizingControlBar, CSizingControlBarG);

CMySizingControlBar::CMySizingControlBar()
{
	//{{AFX_DATA_INIT(CMySizingControlBar)
	//}}AFX_DATA_INIT
	m_IsVisible = FALSE;
	m_IsSizeValid = FALSE;
}

BEGIN_MESSAGE_MAP(CMySizingControlBar, CSizingControlBarG)
	//{{AFX_MSG_MAP(CMySizingControlBar)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_WM_STYLECHANGED()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySizingControlBar message handlers

void CMySizingControlBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	if (!(lpwndpos->flags & SWP_NOSIZE))	// if size is being changed
		m_IsSizeValid = TRUE;	// OnSize arguments are valid from now on
	// this is the correct way to catch show/hide; OnShowWindow is unreliable
	CSizingControlBarG::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		m_IsVisible = TRUE;
	else if (lpwndpos->flags & SWP_HIDEWINDOW)
		m_IsVisible = FALSE;
}

void CMySizingControlBar::OnDestroy() 
{
	CSizingControlBarG::OnDestroy();
	m_IsVisible = FALSE;
}

void CMySizingControlBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	// must detect show here too, e.g. if bar is created visible
	CSizingControlBarG::OnShowWindow(bShow, nStatus);
	if (bShow)
		m_IsVisible = TRUE;
}

void CMySizingControlBar::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	if (nStyleType == GWL_STYLE) {
		BOOL	visOld = (lpStyleStruct->styleOld & WS_VISIBLE);
		BOOL	visNew = (lpStyleStruct->styleNew & WS_VISIBLE);
		if (visOld && !visNew)	// if WS_VISIBLE was removed
			m_IsVisible = FALSE;
	}
}
