// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		24may10	bump version number for V2

		container for job information

*/

#ifndef CJOBINFO_INCLUDED
#define CJOBINFO_INCLUDED

#include "FFProject.h"
#include "VideoComprState.h"
#include "EngineOptions.h"
#include "RecordInfo.h"

class CJobInfo : public CObject {
public:
	DECLARE_SERIAL(CJobInfo);

// Construction
	CJobInfo();
	CJobInfo(const CJobInfo& Info);
	CJobInfo& operator=(const CJobInfo& Info);

// Types

// Constants
	enum {
		ARCHIVE_VERSION = 2			// archive version number
	};

// Public data
	// REMEMBER to add new members to ctor, Copy, and Serialize
	CString	m_RecordPath;	// path to destination file
	CString	m_Name;			// job title
	CString	m_Source;		// source file name
	CString	m_Dest;			// destination file name
	CTime	m_Start;		// when job began
	CTime	m_End;			// when job ended
	int		m_Status;		// see status enum above
	int		m_ID;			// unique identifier
	CString	m_ErrorMsg;		// error message if any
	CFFProject	m_Project;	// project information
	CVideoComprState	m_ComprState;	// compressor state
	CEngineOptions	m_EngineOpts;	// options info
	CRecordInfo	m_RecInfo;	// record info

// Operations
	void	Serialize(CArchive& ar);

protected:
// Member data
	int		m_Version;				// archive version number

// Helpers
	void	Copy(const CJobInfo& Info);
};

inline CJobInfo::CJobInfo(const CJobInfo& Info)
{
	Copy(Info);
}

inline CJobInfo& CJobInfo::operator=(const CJobInfo& Info)
{
	Copy(Info);
	return(*this);
}

template<> inline void AFXAPI
SerializeElements<CJobInfo>(CArchive& ar, CJobInfo* pObj, int nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}

#endif
