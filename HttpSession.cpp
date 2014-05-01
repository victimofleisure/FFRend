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

#include "stdafx.h"
#include "HttpSession.h"

CHttpSession::CHttpSession(LPCTSTR pstrAgent, DWORD dwContext, DWORD dwAccessType, LPCTSTR pstrProxyName, LPCTSTR pstrProxyBypass, DWORD dwFlags)
{
	m_hFile = NULL;
	m_dwContext = dwContext;
	m_ReadBufSize = 4096;
	m_ReadPos = 0;
	m_ReadCount = 0;
	m_hSession = InternetOpen(pstrAgent, dwAccessType, pstrProxyName, pstrProxyBypass, dwFlags);
	if (m_hSession == NULL)
		AfxThrowInternetException(dwContext);
}

CHttpSession::~CHttpSession()
{
	CloseURL();
	InternetCloseHandle(m_hSession);
}


void CHttpSession::CloseURL()
{
	if (m_hFile != NULL) {
		InternetCloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

void CHttpSession::OpenURL(LPCTSTR lpszUrl, DWORD dwContext, DWORD dwFlags, LPCTSTR lpszHeaders, DWORD dwHeadersLength)
{
	CloseURL();
	m_hFile = InternetOpenUrl(m_hSession, lpszUrl, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
	if (m_hFile == NULL)
		AfxThrowInternetException(dwContext);
	m_ReadBuf.SetSize(m_ReadBufSize);
	m_ReadPos = 1;
	m_ReadCount = 1;
}

DWORD CHttpSession::Read(LPVOID lpBuf, DWORD nCount)
{
	ASSERT(m_hFile != NULL);
	DWORD	nBytesRead;
	if (!InternetReadFile(m_hFile, lpBuf, nCount, &nBytesRead))
		AfxThrowInternetException(m_dwContext);
	return(nBytesRead);
}

bool CHttpSession::ReadString(CString& str)
{
	CString	s;
	int	CurLen = 0;
	while (m_ReadCount > 0) {
		if (m_ReadPos < m_ReadCount) {
			bool	FoundTerm = FALSE;
			DWORD	BufPos;
			for (BufPos = m_ReadPos; BufPos < m_ReadCount; BufPos++) {
				if (m_ReadBuf[BufPos] == '\n') {	// if found terminator
					FoundTerm = TRUE;
					break;
				}
			}
			int	AddLen = BufPos - m_ReadPos;
			int	NewLen = CurLen + AddLen;
			LPTSTR	pDest = s.GetBufferSetLength(NewLen) + CurLen;
			CurLen = NewLen;
			const BYTE	*pSrc = m_ReadBuf.GetData() + m_ReadPos;
			// copy additional characters from buffer to string
			for (int i = 0; i < AddLen; i++)
				*pDest++ = *pSrc++;
			if (FoundTerm) {	// if terminator was found
				BufPos++;	// increment buffer position past newline
				// if string's last character is a carriage return
				if (NewLen > 0 && s[NewLen - 1] == '\r')
					NewLen--;	// remove carriage return
			}
			s.ReleaseBuffer(NewLen);	// release string
			m_ReadPos = BufPos;	// update read position
			if (FoundTerm)	// if terminator was found
				break;	// success
		}
		m_ReadPos = 0;	// buffer is empty
		if (!InternetReadFile(m_hFile, m_ReadBuf.GetData(), m_ReadBufSize, &m_ReadCount))
			AfxThrowInternetException(m_dwContext);
	}
	str = s;
	return(m_ReadCount > 0);
}

bool CHttpSession::QueryInfo(DWORD dwInfoLevel, CString& str, LPDWORD lpdwIndex) const
{
	// cribbed from CHttpFile
	ASSERT(dwInfoLevel <= HTTP_QUERY_MAX && dwInfoLevel >= 0);
	BOOL bRet;
	DWORD dwLen = 0;
	// ask for nothing to see how long the return really is
	str.Empty();
	if (HttpQueryInfo(m_hFile, dwInfoLevel, NULL, &dwLen, 0))
		bRet = TRUE;
	else {
		// now that we know how long it is, ask for exactly that much
		// space and really request the header from the API
		LPTSTR pstr = str.GetBufferSetLength(dwLen);
		bRet = HttpQueryInfo(m_hFile, dwInfoLevel, pstr, &dwLen, lpdwIndex);
		if (bRet)
			str.ReleaseBuffer(dwLen);
		else
			str.ReleaseBuffer(0);
	}
	return bRet != 0;
}
