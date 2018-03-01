// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01may11	initial version
		01		30may11	replace app include with resource include

		progress control with percentage overlay
 
*/

// PercentProgressCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PercentProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPercentProgressCtrl

CPercentProgressCtrl::CPercentProgressCtrl()
{
}

CPercentProgressCtrl::~CPercentProgressCtrl()
{
}


BEGIN_MESSAGE_MAP(CPercentProgressCtrl, CProgressCtrl)
	//{{AFX_MSG_MAP(CPercentProgressCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPercentProgressCtrl message handlers

void CPercentProgressCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect	rcClient;
	GetClientRect(rcClient);
	int low, high;
	GetRange(low, high);
	int	delta = high - low;
	int pos = GetPos();
	float	fracPos = float(pos - low) / delta;
	int	x = rcClient.left + round(rcClient.Width() * fracPos);
	CRect	rcBar(rcClient);
	rcBar.right = x;
	dc.FillSolidRect(rcBar, GetSysColor(COLOR_HIGHLIGHT));
	CString	s;
	s.Format(_T("%.0f%%"), fracPos * 100);
	CSize	szText = dc.GetTextExtent(s);
	dc.SelectStockObject(DEFAULT_GUI_FONT);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextAlign(TA_CENTER);
	dc.SetTextColor(RGB(255, 255, 255));
	CPoint	pt(rcClient.Width() / 2, (rcClient.Height() - szText.cy) / 2);
	dc.TextOut(pt.x, pt.y, s);
	dc.ExcludeClipRect(rcBar);
	rcBar.left = x;
	rcBar.right = rcClient.right;
	dc.FillSolidRect(rcBar, GetSysColor(COLOR_3DFACE));
	dc.SetTextColor(RGB(0, 0, 0));
	dc.TextOut(pt.x, pt.y, s);
}

BOOL CPercentProgressCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;	
}
