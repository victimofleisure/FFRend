// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		24may10	add archive version

		container for record information

*/

#include "stdafx.h"
#include "RecordInfo.h"

IMPLEMENT_SERIAL(CRecordInfo, CObject, 1);

CRecordInfo::CRecordInfo()
{
	ZeroMemory(&GetBaseInfo(), sizeof(RECORD_INFO));
}

void CRecordInfo::Copy(const CRecordInfo& Info)
{
	const RECORD_INFO *src = &Info;	// automatic upcast to base struct
	memcpy(&GetBaseInfo(), src, sizeof(RECORD_INFO));
}

void CRecordInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_VERSION;
		ar << sizeof(RECORD_INFO);
		ar.Write(&GetBaseInfo(), sizeof(RECORD_INFO));
	} else {
		int	Version;
		ar >> Version;
		if (Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		int	BaseInfoSize;
		ar >> BaseInfoSize;
		ar.Read(&GetBaseInfo(), BaseInfoSize);
	}
}

