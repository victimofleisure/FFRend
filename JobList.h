// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version

		container for list of jobs

*/

#ifndef CJOBLIST_INCLUDED
#define CJOBLIST_INCLUDED

#include "JobInfo.h"

class CJobList : public CObject {
public:
	DECLARE_SERIAL(CJobList);

// Construction
	CJobList();
	CJobList(const CJobList& List);
	CJobList& operator=(const CJobList& List);

// Types
	typedef CArray<CJobInfo, CJobInfo&> CJobInfoArray;

// Constants
	enum {
		ARCHIVE_SIG = 0x4a524646,	// archive signature (FFRJ)
		ARCHIVE_VERSION = 1			// archive version number
	};

// Public data
	CJobInfoArray	m_Info;			// array of job information
	int		m_NextID;				// next unique identifier

// Operations
	void	Serialize(CArchive& ar);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);

protected:
// Member data
	int		m_Version;				// read file version number

// Helpers
	void	Copy(const CJobList& List);
	bool	DoIO(LPCTSTR Path, DWORD FileMode, DWORD ArchiveMode);
};

inline CJobList::CJobList(const CJobList& List)
{
	Copy(List);
}

inline CJobList& CJobList::operator=(const CJobList& List)
{
	Copy(List);
	return(*this);
}

#endif
