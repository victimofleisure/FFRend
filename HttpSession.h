// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		23jan12	initial version

		HTTP session

*/

#ifndef CHTTPSESSION
#define CHTTPSESSION

#include "afxinet.h"

class CHttpSession : public WObject {
public:
// Construction
	CHttpSession(LPCTSTR pstrAgent = NULL, DWORD dwContext = 1, DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG, LPCTSTR pstrProxyName = NULL, LPCTSTR pstrProxyBypass = NULL, DWORD dwFlags = 0);
	~CHttpSession();

// Attributes
	bool	IsURLOpen() const;

// Operations
	void	OpenURL(LPCTSTR lpszUrl, DWORD dwContext = 1, DWORD dwFlags = 0, LPCTSTR lpszHeaders = NULL, DWORD dwHeadersLength = 0);
	void	CloseURL();
	DWORD	Read(LPVOID lpBuf, DWORD nCount);
	bool	ReadString(CString& str);
	bool	QueryInfo(DWORD dwInfoLevel, CString& str, LPDWORD lpdwIndex = NULL) const;

protected:
// Data members
	HINTERNET	m_hSession;			// session handle
	HINTERNET	m_hFile;			// HTTP file handle
	DWORD	m_dwContext;			// session context
	DWORD	m_ReadBufSize;			// read buffer size
	CByteArray	m_ReadBuf;			// read buffer
	DWORD	m_ReadPos;				// current position in read buffer
	DWORD	m_ReadCount;			// number of bytes in read buffer
};

inline bool CHttpSession::IsURLOpen() const
{
	return(m_hFile != NULL);
}

#endif
