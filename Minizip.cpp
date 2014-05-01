// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jan12	initial version

        wrapper for Gilles Vollant's minizip library
 
*/

#include "stdafx.h"
#include "Minizip.h"

CMinizip::CMinizip()
{
	m_ZipFile = NULL;
}

CMinizip::~CMinizip()
{
	Close();
}

bool CMinizip::Open(LPCTSTR ZipPath)
{
	USES_CONVERSION;
	m_ZipFile = unzOpen(T2CA(ZipPath));
	return(m_ZipFile != NULL);
}

bool CMinizip::Close()
{
	if (m_ZipFile == NULL)
		return(TRUE);
	unzCloseCurrentFile(m_ZipFile);	// in case it didn't get closed
	if (unzClose(m_ZipFile) != UNZ_OK)
		return(FALSE);
	m_ZipFile = NULL;
	return(TRUE);
}

bool CMinizip::Extract(LPCTSTR FileName, LPCTSTR DestPath, UINT BufSize)
{
	USES_CONVERSION;
	if (unzLocateFile(m_ZipFile, T2CA(FileName), 0) != UNZ_OK)
		return(FALSE);
	if (unzOpenCurrentFile(m_ZipFile) != UNZ_OK)
		return(FALSE);
	CByteArray	ba;
	ba.SetSize(BufSize);
	bool	retc = FALSE;	// assume failure
	TRY {
		CFile	DestFile(DestPath, CFile::modeCreate | CFile::modeWrite);
		int	BytesRead;
		while ((BytesRead = unzReadCurrentFile(m_ZipFile, ba.GetData(), BufSize)) > 0) {
			DestFile.Write(ba.GetData(), BytesRead);
		}
		if (!BytesRead)	// if EOF
			retc = TRUE;	// success provided current file closes OK
	}
	CATCH (CFileException, e) {
		e->ReportError();
	}
	END_CATCH
	if (unzCloseCurrentFile(m_ZipFile) != UNZ_OK)	// close fails if bad CRC
		return(FALSE);
	return(retc);
}
