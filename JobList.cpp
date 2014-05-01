// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		23nov07	support Unicode

		container for list of jobs

*/

#include "stdafx.h"
#include "JobList.h"

IMPLEMENT_SERIAL(CJobList, CObject, 1);

CJobList::CJobList()
{
	m_NextID = 0;
	m_Version = ARCHIVE_VERSION;
}

void CJobList::Copy(const CJobList& List)
{
	m_Info.Copy(List.m_Info);
	m_NextID	= List.m_NextID;
	m_Version	= List.m_Version;
}

void CJobList::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_SIG;
		ar << m_Version;
		m_Info.Serialize(ar);
		ar << m_NextID;
	} else {
		int	sig;
		ar >> sig;
		ar >> m_Version;
		if (sig != ARCHIVE_SIG || m_Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		m_Info.Serialize(ar);
		ar >> m_NextID;
	}
}

bool CJobList::DoIO(LPCTSTR Path, DWORD FileMode, DWORD ArchiveMode)
{
	CFile	fp;
	CFileException	e;
	if (!fp.Open(Path, FileMode, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		CArchive	ar(&fp, ArchiveMode);
		ar.m_strFileName = fp.GetFileName();
		Serialize(ar);
	}
	CATCH(CArchiveException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CJobList::Write(LPCTSTR Path)
{
	return(DoIO(Path, CFile::modeCreate | CFile::modeWrite, CArchive::store));
}

bool CJobList::Read(LPCTSTR Path)
{
	return(DoIO(Path, CFile::modeRead | CFile::shareDenyWrite, CArchive::load));
}
