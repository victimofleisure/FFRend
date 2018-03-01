// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29oct06	initial version

        uncaptioned checkbox with focus rectangle
 
*/

// FocusCheckbox.cpp : implementation file
//

#include "stdafx.h"
#include "FocusCheckbox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFocusCheckbox

IMPLEMENT_DYNAMIC(CFocusCheckbox, CButton);

CFocusCheckbox::CFocusCheckbox()
{
}

CFocusCheckbox::~CFocusCheckbox()
{
}

BEGIN_MESSAGE_MAP(CFocusCheckbox, CButton)
	//{{AFX_MSG_MAP(CFocusCheckbox)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFocusCheckbox message handlers

HBRUSH CFocusCheckbox::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	CRect r;
	GetClientRect(r);
	r.InflateRect(3, 1, 1, 1);
	FrameRect(pDC->m_hDC, r, GetSysColorBrush(COLOR_3DFACE));
	if (GetFocus() == this)
		pDC->DrawFocusRect(r);
	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return NULL;
}
