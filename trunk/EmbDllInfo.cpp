// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version

        embedded DLL info
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "EmbDllInfo.h"

IMPLEMENT_SERIAL(CEmbDllInfo, CObject, 1);

CEmbDllInfo::CEmbDllInfo()
{
	m_Length = 0;
	m_Created = 0;
	m_LastWrite = 0;
}

void CEmbDllInfo::Copy(const CEmbDllInfo& Info)
{
	m_Name			= Info.m_Name;
	m_Length		= Info.m_Length;
	m_Created		= Info.m_Created;
	m_LastWrite		= Info.m_LastWrite;
}

void CEmbDllInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Name;
		ar << m_Length;
		ar << m_Created;
		ar << m_LastWrite;
	} else {
		ar >> m_Name;
		ar >> m_Length;
		ar >> m_Created;
		ar >> m_LastWrite;
	}
}
