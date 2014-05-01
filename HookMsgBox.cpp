// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09aug07	initial version
		01		23nov07	support Unicode
		02		26may10	in IsMessageBox, check enum return

        intercept all message boxes
 
*/

#include "stdafx.h"
#include "HookMsgBox.h"

HHOOK	CHookMsgBox::m_hHook;
CHookMsgBox::MSGBOX_CALLBACK	CHookMsgBox::m_MsgBoxCallback;
PVOID	CHookMsgBox::m_Cookie;
CMap<HWND, HWND, LONG, LONG> CHookMsgBox::m_WndProcMap;

CHookMsgBox::CHookMsgBox()
{
	m_IsInstalled = FALSE;
}

CHookMsgBox::~CHookMsgBox()
{
	Remove();	// clean up resources
}

bool CHookMsgBox::Install(MSGBOX_CALLBACK Callback, PVOID Cookie)
{
	if (m_IsInstalled)
		return(FALSE);	// our instance already has a hook installed
	if (m_hHook != NULL)
		return(FALSE);	// assume another instance is using the hook
	m_WndProcMap.RemoveAll();
	m_MsgBoxCallback = Callback;
	m_Cookie = Cookie;
	m_hHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)CbtProc, NULL, GetCurrentThreadId());
	m_IsInstalled = (m_hHook != NULL);
	return(m_IsInstalled);
}

bool CHookMsgBox::Remove()
{
	if (!m_IsInstalled)
		return(FALSE);	// our instance doesn't have a hook installed
	if (m_hHook == NULL)
		return(FALSE);	// hook wasn't installed; should never happen
	if (!UnhookWindowsHookEx(m_hHook))
		return(FALSE);
	m_hHook = NULL;
	m_MsgBoxCallback = NULL;
	m_Cookie = NULL;
	m_WndProcMap.RemoveAll();
	return(TRUE);
}

BOOL CALLBACK CHookMsgBox::EnumChildProc(HWND hWnd, LPARAM lParam)
{
	CHILD_STATS	*pStats = (CHILD_STATS *)lParam;
	TCHAR	szBuf[256];
	GetClassName(hWnd, szBuf, 256);
	if (lstrcmp(szBuf, _T("Static")) == 0) {
		LONG	Style = GetWindowLong(hWnd, GWL_STYLE);
		if (Style & SS_ICON)
			pStats->Icons++;
		else
			pStats->Statics++;
	} else {
		if (lstrcmp(szBuf, _T("Button")) == 0)
			pStats->Buttons++;
		else
			pStats->Others++;
	}
	if (pStats->Others > 0	// if any statistic exceeds its limit
	|| pStats->Icons > 1
	|| pStats->Statics > 1
	|| pStats->Buttons > 3)
		return(FALSE);	// stop enumerating to save time
	return(TRUE);	// continue enumerating child controls
}

bool CHookMsgBox::IsMessageBox(HWND hWnd)
{
	CHILD_STATS	Stats;
	ZeroMemory(&Stats, sizeof(Stats));	// zero statistics
	if (!EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&Stats))
		return(FALSE);
	// dialog must have 1 icon, 1 static, 1 to 3 buttons, and no other children
	return(Stats.Others == 0
		&& Stats.Icons == 1
		&& Stats.Statics == 1
		&& Stats.Buttons > 0
		&& Stats.Buttons < 4);
}

bool CHookMsgBox::IsDialogBox(HWND hWnd)
{
	TCHAR	szBuf[256];
	GetClassName(hWnd, szBuf, 256);
	if (lstrcmp(szBuf, _T("#32770")))	// must have dialog class name
		return(FALSE);
	LONG	Style = GetWindowLong(hWnd, GWL_STYLE);
	if (!(Style & WS_CAPTION))	// must have caption style
		return(FALSE);
	return(TRUE);
}

void CHookMsgBox::GetText(HWND hWnd, CString& str)
{
	int nLength	= SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	LPTSTR	pszMsg = str.GetBuffer(nLength);
	SendMessage(hWnd, WM_GETTEXT, nLength + 1, (LPARAM)pszMsg);
	str.ReleaseBuffer(nLength);
}

LRESULT CALLBACK CHookMsgBox::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG	OldWndProc;
	if (!m_WndProcMap.Lookup(hWnd, OldWndProc))	// find original window procedure
		return 0;	// should never happen
	if (uMsg == WM_INITDIALOG && m_MsgBoxCallback != NULL) {
		if (IsMessageBox(hWnd)) {	// check for valid combination of child controls
			HWND	hStatic = GetDlgItem(hWnd, 65535);
			if (hStatic) {	// if dialog contains a static text control
				CString	MsgText;
				GetText(hStatic, MsgText);
				if (MsgText.GetLength()) {	// if text has non-zero length
					CString	Caption;
					GetText(hWnd, Caption);	// get dialog caption
					// pass caption and message text to callback function
					if (!m_MsgBoxCallback(m_Cookie, Caption, MsgText))
						// if callback function returned false
						SendMessage(hWnd, WM_CLOSE, 0, 0);	// close the dialog
				}
			}
		}
	}
	return(CallWindowProc((WNDPROC)OldWndProc, hWnd, uMsg, wParam, lParam));
}

LRESULT CALLBACK CHookMsgBox::CbtProc(int nCode, WPARAM wParam, LPARAM lParam)
{   
	if (nCode == HCBT_CREATEWND) {
		HWND	hWnd = (HWND)wParam;
		if (IsDialogBox(hWnd)) {	// if window is a dialog box
			// subclass dialog; replace its window procedure with our own
			LONG	OldWndProc = SetWindowLong(hWnd, GWL_WNDPROC, (LONG)WndProc);
			if (OldWndProc)	// if dialog was successfully subclassed
				m_WndProcMap.SetAt(hWnd, OldWndProc);	// save original window proc
		}
	}
	return CallNextHookEx(m_hHook, nCode, wParam, lParam );
}
