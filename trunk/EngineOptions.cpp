// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		25may10	initial version

		container for engine options

*/

#include "stdafx.h"
#include "EngineOptions.h"

IMPLEMENT_SERIAL(CEngineOptions, CObject, 1);

CEngineOptions::CEngineOptions()
{
	ZeroMemory(&GetBaseInfo(), sizeof(ENGINE_OPTIONS));
}

void CEngineOptions::Copy(const CEngineOptions& Info)
{
	const ENGINE_OPTIONS *src = &Info;	// automatic upcast to base struct
	memcpy(&GetBaseInfo(), src, sizeof(ENGINE_OPTIONS));
}

void CEngineOptions::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_VERSION;
		ar << sizeof(ENGINE_OPTIONS);
		ar.Write(&GetBaseInfo(), sizeof(ENGINE_OPTIONS));
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
