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

#pragma once

#include "EngineTypes.h"

class CHistoryInfo : public WObject {
public:
// Construction
	CHistoryInfo();

// Types
	typedef CArrayEx<PROCESS_HISTORY_SAMPLE, PROCESS_HISTORY_SAMPLE&> CSampleArray;
	class CRow : public WObject {
	public:
		CString			m_Name;		// history row name
		CSampleArray	m_Sample;	// array of samples
		void	Serialize(CArchive& ar);
	};
	typedef CArrayEx<CRow, CRow&> CRowArray;

// Constants
	enum {
		FILE_SIG = 0x74734850,		// file signature (PHst)
		FILE_VERSION = 1,			// file version
	};

// Member data
	DWORD			m_StartTime;	// start time of history
	CRowArray		m_Row;			// array of history rows

// Operations
	void	Serialize(CArchive& ar);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);

protected:
// Helpers
	bool	DoIO(LPCTSTR Path, UINT Mode);
};

template<> inline void AFXAPI
SerializeElements<CHistoryInfo::CRow>(CArchive& ar, CHistoryInfo::CRow* pObj, W64INT nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}
