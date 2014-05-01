// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version
        01      24apr10	use ArrayEx

        embedded DLL info
 
*/

#ifndef CEMBDLLINFO_INCLUDED
#define CEMBDLLINFO_INCLUDED

#include "ArrayEx.h"

class CEmbDllInfo : public CObject {
public:
	DECLARE_SERIAL(CEmbDllInfo);

// Construction
	CEmbDllInfo();
	CEmbDllInfo(const CEmbDllInfo& Info);
	CEmbDllInfo& operator=(const CEmbDllInfo& Info);

// Public data
	// don't forget to add new members to ctor, Copy, and Serialize
	CString	m_Name;			// file name of DLL, without path
	UINT	m_Length;		// uncompressed length of DLL file, in bytes
	CTime	m_Created;		// time when DLL file was created
	CTime	m_LastWrite;	// time when DLL file was last modified

// Operations
	void	Serialize(CArchive& ar);

protected:
// Helpers
	void	Copy(const CEmbDllInfo& Info);
};

typedef CArrayEx<CEmbDllInfo, CEmbDllInfo&> CEmbDllInfoArray;

inline CEmbDllInfo::CEmbDllInfo(const CEmbDllInfo& Info)
{
	Copy(Info);
}

inline CEmbDllInfo& CEmbDllInfo::operator=(const CEmbDllInfo& Info)
{
	Copy(Info);
	return(*this);
}

template<> inline void AFXAPI 
SerializeElements<CEmbDllInfo>(CArchive& ar, CEmbDllInfo *pNewEmbDll, int nCount)
{
	for (int i = 0; i < nCount; i++, pNewEmbDll++)
		pNewEmbDll->Serialize(ar);
}

#endif
