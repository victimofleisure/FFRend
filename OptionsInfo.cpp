// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		25may10	derive from engine options

		container for options information

*/

#include "stdafx.h"
#include "OptionsInfo.h"

IMPLEMENT_SERIAL(COptionsInfo, CObject, 1);

COptionsInfo::COptionsInfo()
{
	ZeroMemory(&GetBaseInfo(), sizeof(OPTIONS_INFO));
}

void COptionsInfo::Copy(const COptionsInfo& Info)
{
	CEngineOptions::Copy(Info);	// copy base class
	const OPTIONS_INFO *src = &Info;	// automatic upcast to base struct
	memcpy(&GetBaseInfo(), src, sizeof(OPTIONS_INFO));
	m_MidiDevName	= Info.m_MidiDevName;
}

void COptionsInfo::Serialize(CArchive& ar)
{
	CEngineOptions::Serialize(ar);	// serialize base class
	if (ar.IsStoring()) {
		ar << ARCHIVE_VERSION;
		ar << sizeof(OPTIONS_INFO);
		ar.Write(&GetBaseInfo(), sizeof(OPTIONS_INFO));
		ar << m_MidiDevName;
	} else {
		int	Version;
		ar >> Version;
		if (Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		int	BaseInfoSize;
		ar >> BaseInfoSize;
		ar.Read(&GetBaseInfo(), BaseInfoSize);
		ar >> m_MidiDevName;
	}
}
