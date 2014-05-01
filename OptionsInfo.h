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
		02      04may11	add ctor that takes base structs
		03		01dec11	add run while loading, frame memory limit
		04		23jan12	add check for updates

		container for options information

*/

#pragma once

#include "EngineOptions.h"

typedef struct tagOPTIONS_INFO {
	bool	m_SaveChgsWarn;		// if true, enable save changes warning
	bool	m_UndoUnlimited;	// if true, undo is unlimited
	bool	m_CheckForUpdates;	// if true, check for updates
	int		m_UndoLevels;		// number of undo levels if limited
	int		m_HistorySize;		// history size in samples
	float	m_ViewFreq;			// view refresh frequency in Hertz
	bool	m_CacheThumbs;		// true if caching thumbnails
	SIZE	m_ThumbSize;		// thumbnail size, in pixels
	bool	m_MonitorQuality;	// monitor quality: 0 = fast, 1 = smooth
	bool	m_LockFrameRate;	// if true, lock frame rate to timer
	bool	m_UseMMTimer;		// if true, use multimedia timer
	bool	m_RunWhileLoading;	// if true, run while loading project
	UINT	m_FrameMemoryLimit;	// maximum memory allocated for frames, in MB
} OPTIONS_INFO;

class COptionsInfo : public CEngineOptions, public OPTIONS_INFO {
public:
	DECLARE_SERIAL(COptionsInfo);

// Construction
	COptionsInfo();
	COptionsInfo(const COptionsInfo& Info);
	COptionsInfo(const ENGINE_OPTIONS& EngineOptions, const OPTIONS_INFO& OptionsInfo);
	COptionsInfo& operator=(const COptionsInfo& Info);

// Attributes
	void	SetEngineOptions(const ENGINE_OPTIONS& EngineOptions);
	void	SetOptionsInfo(const OPTIONS_INFO& OptionsInfo);

// Constants
	enum {
		ARCHIVE_VERSION = 1			// archive version number
	};

// Public data; members MUST be included in Copy and Serialize
	CString	m_MidiDevName;		// name of current MIDI device

// Operations
	void	Serialize(CArchive& ar);
	OPTIONS_INFO&	GetBaseInfo();

protected:
// Helpers
	void	Copy(const COptionsInfo& Info);
};

inline COptionsInfo::COptionsInfo(const COptionsInfo& Info)
{
	Copy(Info);
}

inline COptionsInfo& COptionsInfo::operator=(const COptionsInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline void COptionsInfo::SetEngineOptions(const ENGINE_OPTIONS& EngineOptions)
{
	ENGINE_OPTIONS&	MyEngOpts = *this;	// upcast
	MyEngOpts = EngineOptions;
}

inline void COptionsInfo::SetOptionsInfo(const OPTIONS_INFO& OptionsInfo)
{
	OPTIONS_INFO&	MyInfo = *this;	// upcast
	MyInfo = OptionsInfo;
}

inline COptionsInfo::COptionsInfo(const ENGINE_OPTIONS& EngineOptions, const OPTIONS_INFO& OptionsInfo)
{
	SetEngineOptions(EngineOptions);
	SetOptionsInfo(OptionsInfo);
}

inline OPTIONS_INFO& COptionsInfo::GetBaseInfo()
{
	return(*this);	// automatic upcast to base struct
}

template<> inline void AFXAPI
SerializeElements<COptionsInfo>(CArchive& ar, COptionsInfo* pObj, int nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}
