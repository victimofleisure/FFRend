// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version

		container for job information

*/

#include "stdafx.h"
#include "JobInfo.h"

IMPLEMENT_SERIAL(CJobInfo, CObject, 1);

CJobInfo::CJobInfo()
{
	m_Start		= 0;
	m_End		= 0;
	m_Status	= 0;
	m_ID		= 0;
	m_Version	= ARCHIVE_VERSION;
}

void CJobInfo::Copy(const CJobInfo& Info)
{
	m_Version		= Info.m_Version;
	m_RecordPath	= Info.m_RecordPath;
	m_Name			= Info.m_Name;
	m_Source		= Info.m_Source;
	m_Dest			= Info.m_Dest;
	m_Start			= Info.m_Start;
	m_End			= Info.m_End;
	m_Status		= Info.m_Status;
	m_ID			= Info.m_ID;
	m_ErrorMsg		= Info.m_ErrorMsg;
	m_Project		= Info.m_Project;
	m_ComprState	= Info.m_ComprState;
	m_EngineOpts	= Info.m_EngineOpts;
	m_RecInfo		= Info.m_RecInfo;
}

void CJobInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Version;
		ar << m_RecordPath;
		ar << m_Name;
		ar << m_Source;
		ar << m_Dest;
		ar << m_Start;
		ar << m_End;
		ar << m_Status;
		ar << m_ID;
		ar << m_ErrorMsg;
	} else {
		ar >> m_Version;
		if (m_Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		ar >> m_RecordPath;
		ar >> m_Name;
		ar >> m_Source;
		ar >> m_Dest;
		ar >> m_Start;
		ar >> m_End;
		ar >> m_Status;
		ar >> m_ID;
		ar >> m_ErrorMsg;
	}
	m_Project.Serialize(ar);
	m_ComprState.Serialize(ar);
	m_EngineOpts.Serialize(ar);
	m_RecInfo.Serialize(ar);
}
