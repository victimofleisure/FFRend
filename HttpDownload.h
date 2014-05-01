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

#ifndef CHTTPDOWNLOAD
#define CHTTPDOWNLOAD

#include "HttpSession.h"

class CHttpDownload : public WObject {
public:
// Construction
	virtual	~CHttpDownload();

// Attributes
	CString	GetErrorMessage() const;

// Operations
	bool	GetDownloadURL(LPCTSTR InputURL, LPCTSTR VersionPrefix, LPCTSTR VersionPostfix, ULARGE_INTEGER& DownloadVersion, CString& DownloadURL);
	bool	GetHttpDownload(LPCTSTR DownloadURL, LPCTSTR DestPath, DWORD& TotalRcvd, UINT BufferSize = 4096);

protected:
// Data members
	CString	m_ErrorMsg;			// most recent error message

// Overrideables
	virtual	bool	OnDownloadProgress(UINT FileSize, UINT TotalRcvd);

// Helpers
	void	HandleException(CException *e);
};

inline CString CHttpDownload::GetErrorMessage() const
{
	return(m_ErrorMsg);
}

#endif
