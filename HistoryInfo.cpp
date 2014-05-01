// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29mar10	initial version

        history info
 
*/

#include "stdafx.h"
#include "HistoryInfo.h"

CHistoryInfo::CHistoryInfo()
{
	m_StartTime = 0;
}

void CHistoryInfo::CRow::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Name;
	} else {
		ar >> m_Name;
	}
	m_Sample.Serialize(ar);
}

void CHistoryInfo::Serialize(CArchive &ar)
{
	if (ar.IsStoring()) {
		ar << FILE_SIG;
		ar << FILE_VERSION;
		ar << m_StartTime;
	} else {
		int	Sig, Version;
		ar >> Sig;
		ar >> Version;
		if (Sig != FILE_SIG || Version > FILE_VERSION) {
			AfxThrowArchiveException(CArchiveException::badIndex, 
				ar.GetFile()->GetFileName());
		}
		ar >> m_StartTime;
	}
	m_Row.Serialize(ar);
}

bool CHistoryInfo::DoIO(LPCTSTR Path, UINT Mode)
{
	try {
		UINT	FileMode = Mode == CArchive::load ? 
			(CFile::modeRead | CFile::shareDenyWrite) :
			(CFile::modeCreate | CFile::modeWrite);
		CFile	fp(Path, FileMode);
		CArchive	ar(&fp, Mode);
		Serialize(ar);
	}
	catch (CArchiveException *e) {
		e->ReportError();
		delete e;
		return(FALSE);
	}
	catch (CFileException *e) {
		e->ReportError();
		delete e;
		return(FALSE);
	}
	return(TRUE);
}

bool CHistoryInfo::Write(LPCTSTR Path)
{
	return(DoIO(Path, CArchive::store));
}

bool CHistoryInfo::Read(LPCTSTR Path)
{
	return(DoIO(Path, CArchive::load));
}
