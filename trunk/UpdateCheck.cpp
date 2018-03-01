// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jan12	initial version

        check for updates
 
*/

#include "stdafx.h"
#include "FFRend.h"
#include "UpdateCheck.h"
#include "HttpDownload.h"
#include "ProgressDlg.h"
#include "VersionInfo.h"
#include "PathStr.h"
#include "Minizip.h"

#define DOWNLOAD_PAGE_URL	_T("http://ffrend.sourceforge.net/download.html")
#define VERSION_PREFIX		_T("ffrend")
#define VERSION_POSTFIX		_T("bin.zip")
#define INSTALLER_NAME		_T("FFRend.msi")
#define RK_UPDATE_DECLINED	_T("UpdateDeclined")

class CProgressHttpDownload : public CHttpDownload
{
public:
	bool	GetHttpDownload(LPCTSTR DownloadURL, LPCTSTR DestPath, DWORD& TotalRcvd);
	virtual	bool	OnDownloadProgress(UINT FileSize, UINT BytesRcvd);

protected:
	CProgressDlg	m_ProgressDlg;
};

bool CProgressHttpDownload::GetHttpDownload(LPCTSTR DownloadURL, LPCTSTR DestPath, DWORD& TotalRcvd)
{
	m_ProgressDlg.Create();
	CString	caption;
	caption.Format(IDS_UPCK_DOWNLOADING_CAPTION, theApp.m_pszAppName);
	m_ProgressDlg.SetWindowText(caption);
	CWaitCursor	wc;
	bool	retc = CHttpDownload::GetHttpDownload(DownloadURL, DestPath, TotalRcvd);
	m_ProgressDlg.DestroyWindow();
	if (!retc)
		DeleteFile(DestPath);
	return(retc);
}

bool CProgressHttpDownload::OnDownloadProgress(UINT FileSize, UINT TotalRcvd)
{
	if (FileSize) {	// file size may be zero if unavailable
		int	pos = round(double(TotalRcvd) / FileSize * 100);
		m_ProgressDlg.SetPos(pos);
	}
	return(!m_ProgressDlg.Canceled());
}

bool CUpdateCheck::Check(bool Explicit)
{
	CString	msg;
	ULARGE_INTEGER	DownloadVersion;
	CString	DownloadURL;
	CHttpDownload	httpdl;
	CWaitCursor	wc;
	// download specified download page and parse it for download URL
	if (!httpdl.GetDownloadURL(DOWNLOAD_PAGE_URL, VERSION_PREFIX, VERSION_POSTFIX, DownloadVersion, DownloadURL)) {
		if (Explicit) {	// suppress message for automatic checks
			msg.Format(IDS_UPCK_CANT_GET_VERSION, httpdl.GetErrorMessage(), DOWNLOAD_PAGE_URL);
			AfxMessageBox(msg);
		}
		return(FALSE);
	}
	return(Check(DownloadVersion, DownloadURL, Explicit));
}

#include "CustomMsgBox.h"

bool CUpdateCheck::Check(ULARGE_INTEGER DownloadVersion, LPCTSTR DownloadURL, bool Explicit)
{
	CString	msg;
	VS_FIXEDFILEINFO	AppInfo;
	CVersionInfo::GetFileInfo(AppInfo, NULL);
	ULARGE_INTEGER	CurrentVersion = {AppInfo.dwFileVersionLS, AppInfo.dwFileVersionMS};
	if (CurrentVersion.QuadPart < DownloadVersion.QuadPart) {	// if newer version
		if (!Explicit) {	// if automatic check
			ULONGLONG	DeclinedVersion = 0;
			if (theApp.RdRegStruct(RK_UPDATE_DECLINED, DeclinedVersion, DeclinedVersion)) {
				// if this update was previously declined according to registry
				if (DeclinedVersion == DownloadVersion.QuadPart)
					return(FALSE);	// quietly suppress notification
			}
		}
		msg.Format(IDS_UPCK_WANT_NEWER_VERSION, theApp.m_pszAppName,
			HIWORD(CurrentVersion.HighPart), LOWORD(CurrentVersion.HighPart),
			HIWORD(CurrentVersion.LowPart), LOWORD(CurrentVersion.LowPart),
			HIWORD(DownloadVersion.HighPart), LOWORD(DownloadVersion.HighPart),
			HIWORD(DownloadVersion.LowPart), LOWORD(DownloadVersion.LowPart));
		if (Explicit) {	// if explicit check, ignore previously declined update
			UINT	nType = MB_ICONQUESTION | MB_YESNO;
			if (AfxMessageBox(msg, nType) != IDYES)	// if update unwanted
				return(FALSE);
		} else {	// automatic check
			const int	BTNS = 2;
			LPCTSTR	BtnText[BTNS] = {NULL, _T("&Later")};
			UINT	nType = MB_ICONQUESTION | MB_YESNOCANCEL;
			int	retc = CCustomMsgBox::MsgBox(msg, nType, BtnText, BTNS);
			if (retc == IDNO) {	// if update declined
				// save declined update's version number in registry
				theApp.WrRegStruct(RK_UPDATE_DECLINED, DownloadVersion.QuadPart);
			}
			if (retc != IDYES)
				return(FALSE);
		}
	} else {	// already have latest version
		if (Explicit) {	// suppress message for automatic checks
			msg.Format(IDS_UPCK_HAVE_LATEST_VERSION, theApp.m_pszAppName);
			AfxMessageBox(msg, MB_ICONINFORMATION);
		}
		return(FALSE);
	}
	CEngine::CPause	pause(theApp.GetEngine());	// pause engine while in scope
	// get temp file name for download
	DWORD	TotalRcvd;
	CString	DownloadPath;
	if (!theApp.GetTempFileName(DownloadPath, _T("cfu"), 0)) {
		AfxMessageBox(IDS_UPCK_CANT_GET_TEMP_FILE);
		return(FALSE);
	}
	// download zip file
	CProgressHttpDownload	httpdl;
	CWaitCursor	wc;
	if (!httpdl.GetHttpDownload(DownloadURL, DownloadPath, TotalRcvd)) {
		msg.Format(IDS_UPCK_CANT_DOWNLOAD_FILE, httpdl.GetErrorMessage(), DownloadURL);
		AfxMessageBox(msg);
		return(FALSE);
	}
	// extract installer from zip file
	CPathStr	InstallerPath;
	theApp.GetTempPath(InstallerPath);
	InstallerPath.Append(INSTALLER_NAME);	// make installer path
	bool	UnzipOK;
	{
		CMinizip	zip;
		UnzipOK = zip.Open(DownloadPath) 
			&& zip.Extract(INSTALLER_NAME, InstallerPath);
	}
	DeleteFile(DownloadPath);	// delete zip file either way
	if (!UnzipOK) {
		msg.Format(IDS_UPCK_CANT_UNZIP_DOWNLOAD, INSTALLER_NAME);
		AfxMessageBox(msg);
		return(FALSE);
	}
	// launch reinstall script
	if (!Reinstall(InstallerPath)) {
		DeleteFile(InstallerPath);	// delete installer
		return(FALSE);
	}
	theApp.GetMain()->PostMessage(WM_CLOSE);	// exit so installer can run
	return(TRUE);
}

bool CUpdateCheck::Reinstall(LPCTSTR InstallerPath)
{
	enum {
		INITIAL_DELAY = 1,	// approximate initial delay in seconds
		PING_COUNT = INITIAL_DELAY + 1,	// first ping doesn't count
	};
	CString	msg;
	// verify reinstall script exists, because ShellExecute won't care
	LPCTSTR	ScriptName = _T("reinstall.bat");
	CPathStr	ScriptPath(theApp.GetAppFolder());
	ScriptPath.Append(ScriptName);
	if (!PathFileExists(ScriptPath)) {
		msg.Format(IDS_UPCK_SCRIPT_NOT_FOUND, ScriptPath);	// shouldn't happen
		AfxMessageBox(msg);
		return(FALSE);
	}
	// app must exit before installer runs, so prompt user to save changes if any
	theApp.GetMain()->SendMessage(WM_COMMAND, ID_FILE_NEW);
	if (theApp.GetMain()->IsModified()) {	// if user canceled at save changes prompt
		AfxMessageBox(IDS_UPCK_UPDATE_CANCELED);
		return(FALSE);
	}
	// launch reinstall script
	CString	CmdLine;
	CmdLine.Format(_T("/c %s %d \"%s\" %s"), ScriptName, 
		PING_COUNT, InstallerPath, theApp.m_pszExeName);
	int	ShellRet = int(ShellExecute(NULL, NULL, 
		_T("cmd"), CmdLine, theApp.GetAppFolder(), SW_SHOW));
	if (ShellRet <= 32) {	// if ShellExecute failed
		msg.Format(IDS_UPCK_CANT_RUN_INSTALLER, ShellRet);
		AfxMessageBox(msg);
		return(FALSE);
	}
	return(TRUE);
}

void CUpdateCheck::CheckAsync(HWND hWnd)
{
	ASSERT(IsWindow(hWnd));
	AfxBeginThread(CheckWorker, hWnd, 0, 0, 0, NULL);
}

UINT CUpdateCheck::CheckWorker(LPVOID pParam)
{
	HWND	hWnd = static_cast<HWND>(pParam);
	DWORD	CHECK_DELAY = 1000;	// give app and user time to settle down
	Sleep(CHECK_DELAY);
	CHttpDownload	httpdl;
	CString	DownloadURL;
	ULARGE_INTEGER	DownloadVersion;
	if (httpdl.GetDownloadURL(DOWNLOAD_PAGE_URL, VERSION_PREFIX, VERSION_POSTFIX, DownloadVersion, DownloadURL)) {
		CAppUpdateInfo	*pInfo = new CAppUpdateInfo;	// recipient must delete
		// strings mustn't be shared between threads: locking source string
		// forces assignment to actually copy instead of adding a reference
		DownloadURL.LockBuffer();	// disable reference counting
		pInfo->m_URL = DownloadURL;
		pInfo->m_Version.QuadPart = DownloadVersion.QuadPart;
		PostMessage(hWnd, UWM_APPUPDATEINFO, WPARAM(pInfo), 0);
	}
	return(0);
}

void CUpdateCheck::OnAppUpdateInfo(WPARAM wParam, LPARAM lParam)
{
	CAppUpdateInfo	*pInfo = reinterpret_cast<CAppUpdateInfo *>(wParam);
	// if exclusive on a single monitor, suppress update to avoid UI lockup
	if (!theApp.GetMain()->IsSingleMonitorExclusive())
		Check(pInfo->m_Version, pInfo->m_URL, FALSE);
	delete pInfo;	// allocated by worker thread
}
