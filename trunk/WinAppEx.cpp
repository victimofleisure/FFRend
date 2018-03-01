// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      06jan10	W64: in DeleteDirectory, use size_t
        02      13jan12	add GetTempPath and GetAppPath

        enhanced application
 
*/

// WinAppEx.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WinAppEx.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// MakeSureDirectoryPathExists doesn't support Unicode; SHCreateDirectoryEx
// is a reasonable substitute, but older versions of the SDK don't define it
#if defined(UNICODE)
#ifndef	SHCreateDirectoryEx
int WINAPI SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, const SECURITY_ATTRIBUTES *psa);
#define SHCreateDirectoryEx SHCreateDirectoryExW
#define SHCreateDirectoryProxy
#endif
#else
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinAppEx

BEGIN_MESSAGE_MAP(CWinAppEx, CWinApp)
	//{{AFX_MSG_MAP(CWinAppEx)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinAppEx construction

bool CWinAppEx::GetTempPath(CString& Path)
{
	LPTSTR	pBuf = Path.GetBuffer(MAX_PATH);
	DWORD	retc = ::GetTempPath(MAX_PATH, pBuf);
	Path.ReleaseBuffer();
	return(retc != 0);
}

bool CWinAppEx::GetTempFileName(CString& Path, LPCTSTR Prefix, UINT Unique)
{
	CString	TempPath;
	if (!GetTempPath(TempPath))
		return(FALSE);
	if (Prefix == NULL)
		Prefix = m_pszAppName;
	LPTSTR	pBuf = Path.GetBuffer(MAX_PATH);
	DWORD	retc = ::GetTempFileName(TempPath, Prefix, Unique, pBuf);
	Path.ReleaseBuffer();
	return(retc != 0);
}

bool CWinAppEx::GetAppDataFolder(CString& Folder)
{
	CPathStr	path;
	LPTSTR	p = path.GetBuffer(MAX_PATH);
	bool	retc = SUCCEEDED(SHGetSpecialFolderPath(NULL, p, CSIDL_APPDATA, 0));
	path.ReleaseBuffer();
	if (retc) {
		path.Append(m_pszAppName);
		Folder = path;
	}
	return(retc);
}

CString CWinAppEx::GetAppPath()
{
	CString	s = GetCommandLine();
	s.TrimLeft();	// trim leading whitespace just in case
	if (s[0] == '"')	// if first char is a quote
		s = s.Mid(1).SpanExcluding(_T("\""));	// span to next quote
	else
		s = s.SpanExcluding(_T(" \t"));	// span to next whitespace
	return(s);
}

CString CWinAppEx::GetAppFolder()
{
	CPathStr	path(GetAppPath());
	path.RemoveFileSpec();
	return(path);
}

bool CWinAppEx::GetIconSize(HICON Icon, CSize& Size)
{
	ICONINFO	IconInfo;
	if (!GetIconInfo(Icon, &IconInfo))
		return(FALSE);
	BITMAP	bmp;
	bool	retc = GetObject(IconInfo.hbmColor, sizeof(bmp), &bmp) != 0;
	if (retc)
		Size = CSize(bmp.bmWidth, bmp.bmHeight);
	DeleteObject(IconInfo.hbmColor);
	DeleteObject(IconInfo.hbmMask);
	return(retc);
}

void CWinAppEx::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	POSITION	TplPos = GetFirstDocTemplatePosition();
	while (TplPos != NULL) {	// for each document template
		CDocTemplate	*pTpl = GetNextDocTemplate(TplPos);
		POSITION	DocPos = pTpl->GetFirstDocPosition();
		while (DocPos != NULL) {	// for each document, update all its views
			CDocument	*pDoc = pTpl->GetNextDoc(DocPos);
			pDoc->UpdateAllViews(pSender, lHint, pHint);
		}
	}
}

bool CWinAppEx::DeleteDirectory(LPCTSTR lpszDir, bool bAllowUndo)
{
	size_t	len = _tcslen(lpszDir);
	TCHAR	*pszFrom = new TCHAR[len + 2];
	_tcscpy(pszFrom, lpszDir);
	pszFrom[len] = 0;
	pszFrom[len + 1] = 0;
	SHFILEOPSTRUCT fileop;
	fileop.hwnd   = NULL;	// no status display
	fileop.wFunc  = FO_DELETE;	// delete operation
	fileop.pFrom  = pszFrom;	// source file name as double null terminated string
	fileop.pTo    = NULL;	// no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;  // do not prompt the user
	if (bAllowUndo)
		fileop.fFlags |= FOF_ALLOWUNDO;
	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle = NULL;
	fileop.hNameMappings = NULL;
	int ret = SHFileOperation(&fileop);
	delete [] pszFrom;  
	return(ret == 0);
}

void CWinAppEx::EnableChildWindows(CWnd& Wnd, bool Enable)
{
	CWnd	*wp = Wnd.GetWindow(GW_CHILD);
	while (wp != NULL) {
		wp->EnableWindow(Enable);
		wp = wp->GetNextWindow();
	}
}

CString CWinAppEx::GetTitleFromPath(LPCTSTR Path)
{
	CPathStr	s(PathFindFileName(Path));
	s.RemoveExtension();
	return(s);
}

bool CWinAppEx::GetComputerName(CString& Name)
{
	DWORD	len = MAX_COMPUTERNAME_LENGTH + 1;
	LPTSTR	p = Name.GetBuffer(len);
	bool	retc = ::GetComputerName(p, &len) != 0;
	Name.ReleaseBuffer(len);
	return(retc);
}

// MakeSureDirectoryPathExists doesn't support Unicode; SHCreateDirectoryEx
// is a reasonable substitute, but older versions of the SDK don't define it
#ifdef SHCreateDirectoryProxy
int WINAPI SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, const SECURITY_ATTRIBUTES *psa)
{
	int	retc = ERROR_INVALID_FUNCTION;
	typedef int (WINAPI* lpfnSHCreateDirectoryExW)(HWND hwnd, LPCWSTR pszPath, const SECURITY_ATTRIBUTES *psa);
	HMODULE hShell = LoadLibrary(_T("shell32.dll"));
	lpfnSHCreateDirectoryExW lpfn = NULL;
	if (hShell) {
		lpfn = (lpfnSHCreateDirectoryExW)GetProcAddress(hShell, "SHCreateDirectoryExW");
		if (lpfn)
			retc = lpfn(hwnd, pszPath, psa);
		FreeLibrary(hShell);
	}
	return(retc);
}
#endif

bool CWinAppEx::CreateFolder(LPCTSTR Path)
{
#ifdef UNICODE
	return(SHCreateDirectoryEx(NULL, Path, NULL) == ERROR_SUCCESS);
#else
	CString	s(Path);
	LPTSTR	p = s.GetBuffer(MAX_PATH);
	PathAddBackslash(p);	// trailing backslash is required
	s.ReleaseBuffer();
	return(MakeSureDirectoryPathExists(s) != 0);	// very slow
#endif
}

// This function fixes the dot bitmap CCmdUI::SetRadio uses in radio-style menu
// items, which is too small if the menu font is large; taken from MSJ Jan 1998
void CWinAppEx::FixMFCDotBitmap()
{
	HBITMAP	hbmDot = GetMFCDotBitmap();
	if (hbmDot) {
		// Draw a centered dot of appropriate size
		BITMAP	bm;
		::GetObject(hbmDot, sizeof(bm), &bm);
		CRect rcDot(0, 0, bm.bmWidth, bm.bmHeight);
		rcDot.DeflateRect((bm.bmWidth >> 1) - 2, (bm.bmHeight >> 1) - 2);
		CWindowDC	dcScreen(NULL);
		CDC	memdc;
		memdc.CreateCompatibleDC(&dcScreen);
		int	nSave = memdc.SaveDC();
		memdc.SelectStockObject(BLACK_PEN);
		memdc.SelectStockObject(BLACK_BRUSH);
		memdc.SelectObject((HGDIOBJ)hbmDot);
		memdc.PatBlt(0, 0, bm.bmWidth, bm.bmHeight, WHITENESS);
		memdc.Ellipse(&rcDot);
		memdc.RestoreDC(nSave);
	}
}

HBITMAP CWinAppEx::GetMFCDotBitmap()
{
	// The bitmap is stored in afxData.hbmMenuDot, but afxData is MFC-private,
	// so the only way to get it is create a menu, set the radio check,
	// and then see what bitmap MFC set in the menu item.
	CMenu	menu;
	VERIFY(menu.CreateMenu());
	VERIFY(menu.AppendMenu(MFT_STRING, 0, (LPCTSTR)NULL));
	CCmdUI	cui;
	cui.m_pMenu = &menu;
	cui.m_nIndex = 0;
	cui.m_nIndexMax = 1;
	cui.SetRadio(TRUE);
	MENUITEMINFO	info;
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_CHECKMARKS;
	GetMenuItemInfo(menu, 0, MF_BYPOSITION, &info);
	HBITMAP hbmDot = info.hbmpChecked;
	menu.DestroyMenu();
	return hbmDot;
}
