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

#pragma once

typedef struct tagENGINE_OPTIONS {
	// APPEND ONLY! This struct is included in the job control archive, so 
	// don't delete or reorder members, else preexisting job control files
	// will be garbled. Only simple types that can handle bitwise copy are
	// permitted here; objects must be added to CEngineOptions instead.
	SIZE	m_FrameSize;		// size of plugin frame, in pixels
	float	m_FrameRate;		// frame rate, in frames per second
	UINT	m_ColorDepth;		// color depth, in bits per pixel
	int		m_RandSeed;			// seed for random number generator
	bool	m_RandUseTime;		// if true, use time as random seed
	UINT	m_FrameTimeout;		// frame timeout, in milliseconds
	int		m_Reserved;			// for future enhancements
} ENGINE_OPTIONS;

class CEngineOptions : public CObject, public ENGINE_OPTIONS {
public:
	DECLARE_SERIAL(CEngineOptions);

// Construction
	CEngineOptions();
	CEngineOptions(const CEngineOptions& Info);
	CEngineOptions& operator=(const CEngineOptions& Info);

// Constants
	enum {
		ARCHIVE_VERSION = 1			// archive version number
	};

// Public data; members MUST be included in Copy and Serialize

// Operations
	void	Serialize(CArchive& ar);
	ENGINE_OPTIONS&	GetBaseInfo();

protected:
// Helpers
	void	Copy(const CEngineOptions& Info);
};

inline CEngineOptions::CEngineOptions(const CEngineOptions& Info)
{
	Copy(Info);
}

inline CEngineOptions& CEngineOptions::operator=(const CEngineOptions& Info)
{
	Copy(Info);
	return(*this);
}

inline ENGINE_OPTIONS& CEngineOptions::GetBaseInfo()
{
	return(*this);	// automatic upcast to base struct
}

template<> inline void AFXAPI
SerializeElements<CEngineOptions>(CArchive& ar, CEngineOptions* pObj, int nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}
