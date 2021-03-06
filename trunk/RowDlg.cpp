// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      13aug05	initial version
		01		12sep05	add row position
		02		11jul07	remove row dialog tab message; use DS_CONTROL instead
		03		24mar09	add hook for dialog key handler
		04		20apr10	refactor
		05		06apr12	add shortcut key to reset column widths

         row dialog base class
 
*/

// RowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "RowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRowDlg dialog

IMPLEMENT_DYNAMIC(CRowDlg, CDialog);

CRowDlg::CRowDlg(UINT Template, CWnd* pParent /*=NULL*/)
	: CDialog(Template, pParent)
{
	//{{AFX_DATA_INIT(CRowDlg)
	//}}AFX_DATA_INIT
	m_RowIdx = 0;
	m_RowPos = 0;
}

void CRowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRowDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRowDlg, CDialog)
	//{{AFX_MSG_MAP(CRowDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRowDlg message handlers

void CRowDlg::OnOK()
{
}

void CRowDlg::OnCancel()
{
}

BOOL CRowDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		CRowView	*pView = GetView();
		CWnd	*pAccelWnd;
		HACCEL	Accel = pView->GetAccel(pAccelWnd);
		if (pAccelWnd != NULL) {
			if (Accel != NULL) {
				if (TranslateAccelerator(pAccelWnd->m_hWnd, Accel, pMsg))
					return(TRUE);
			} else {
				if (pMsg->message == WM_KEYDOWN	// for non-system keys only
				&& pAccelWnd->SendMessage(UWM_HANDLEDLGKEY, (WPARAM)pMsg))
					return(TRUE);
			}
		}
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ADD
		&& (GetKeyState(VK_CONTROL) & GKS_DOWN)) {
			pView->ResetColumnWidths();
			return(TRUE);
		}
	}
	// NOTE that the derived row dialog's resource is now assumed to have the
	// DS_CONTROL style, which makes a dialog work well as a child of another
	// dialog, e.g. by integrating the child dialog's tab layout into the tab
	// layout of its parent; in previous versions, the parent handled tabbing
	// explicitly, so we detected the tab key here and send a notification.
	return CDialog::PreTranslateMessage(pMsg);
}
