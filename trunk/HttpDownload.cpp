// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jan12	initial version

        download files via HTTP
 
*/

#include "stdafx.h"
#include "HttpDownload.h"

CHttpDownload::~CHttpDownload()
{
}

bool CHttpDownload::GetDownloadURL(LPCTSTR InputURL, LPCTSTR VersionPrefix, LPCTSTR VersionPostfix, ULARGE_INTEGER& DownloadVersion, CString& DownloadURL)
{
	// This function downloads an input text file, for example the download
	// page of a project web site, and parses it for a download URL which
	// contains a version number. The download URL must have this format:
	//
	// "*/prefix-N.N.N.N-postfix*"
	//
	// where prefix is replaced by the Prefix parameter, typically the name
	// of the application in lower case, postfix is replaced by the Postfix
	// parameter, typically including the file extension, and each wildcard
	// represents zero or more instances of valid URL characters. The entire
	// download URL must be enclosed in double quotes. Assuming a prefix of
	// "myapp" and a postfix of "bin.zip", here's an example match:
	//
	// <a href="http://foo.org/files/myapp-1.2.3.4-bin.zip">download</a>
	//
	CHttpSession	m_Session;
	m_ErrorMsg.Empty();
	bool	retc = FALSE;
	TRY {
		DWORD	dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;
		m_Session.OpenURL(InputURL, 1, dwFlags);
		CString	line;
		while (m_Session.ReadString(line)) {	// for each line
			int	PostfixPos = line.Find(VersionPostfix);	// find postfix
			if (PostfixPos < 0)		// if not found
				continue;
			CString	s(line.Left(PostfixPos));	// starting at postfix
			int	FileNamePos = s.ReverseFind('/');		// reverse find file name
			if (FileNamePos < 0)	// if not found
				continue;
			int	UrlStartPos = s.ReverseFind('"');		// reverse find start of URL
			if (UrlStartPos < 0)	// if not found
				continue;
			int	UrlEndPos = line.Find('"', PostfixPos);	// find end of URL
			if (UrlEndPos < 0)		// if not found
				continue;
			int	ver[4];
			CString	VersionFmt;
			VersionFmt.Format(_T("/%s-%%d.%%d.%%d.%%d"), VersionPrefix);	// build format
			CString	FileName(s.Mid(FileNamePos));
			int	levels = _stscanf(FileName, VersionFmt,	// scan version number
				&ver[3], &ver[2], &ver[1], &ver[0]);
			if (levels == 4) {	// if version number scanned correctly
				DownloadVersion.LowPart = MAKELONG(ver[0], ver[1]);
				DownloadVersion.HighPart = MAKELONG(ver[2], ver[3]);
				int	UrlLen = UrlEndPos - UrlStartPos - 1;
				DownloadURL = line.Mid(UrlStartPos + 1, UrlLen);
				retc = TRUE;	// success
				break;	// exit loop
			}
		}
		if (!retc)
			m_ErrorMsg = _T("Download URL not found");
	}
	CATCH (CException, e) {
		HandleException(e);
	}
	END_CATCH
	return(retc);
}

bool CHttpDownload::GetHttpDownload(LPCTSTR DownloadURL, LPCTSTR DestPath, DWORD& TotalRcvd, UINT BufferSize)
{
	CHttpSession	m_Session;
	m_ErrorMsg.Empty();
	TotalRcvd = 0;
	bool	retc = FALSE;
	TRY {
		DWORD	dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;
		m_Session.OpenURL(DownloadURL, 0, dwFlags);
		CFile	DestFile(DestPath, CFile::modeCreate | CFile::modeWrite);
		CString ContentLen;
		DWORD	FileSize;	// query file size
		if (m_Session.QueryInfo(HTTP_QUERY_CONTENT_LENGTH, ContentLen))
			FileSize = _ttoi(ContentLen);
		else	// file size unavailable; non-fatal
			FileSize = 0;
		CByteArray	buf;
		buf.SetSize(BufferSize);	// create buffer
		UINT	rcvd;
		while ((rcvd = m_Session.Read(buf.GetData(), BufferSize)) != 0) {
			TotalRcvd += rcvd;
			if (!OnDownloadProgress(FileSize, TotalRcvd))	// call derived handler
				 AfxThrowInternetException(0, ERROR_CANCELLED);
			DestFile.Write(buf.GetData(), rcvd);	// append to destination file
		}
		retc = TRUE;	// success
	}
	CATCH (CException, e) {
		HandleException(e);
	}
	END_CATCH
	return(retc);
}

bool CHttpDownload::OnDownloadProgress(UINT FileSize, UINT TotalRcvd)
{
	return(TRUE);	// false to cancel download
}

void CHttpDownload::HandleException(CException *e)
{
	UINT	nMaxError = 255;
	LPTSTR	pBuf = m_ErrorMsg.GetBuffer(nMaxError);
	e->GetErrorMessage(pBuf, nMaxError);
}
