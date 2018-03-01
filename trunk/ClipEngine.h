// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		06may10	initial version

        engine with clip player support
 
*/

#pragma once

#include "MidiEngine.h"

class CClipEngine : public CMidiEngine {
public:
// Construction
	CClipEngine(CRenderer& Renderer);

// Attributes
	void	SetClipPlayerPath(LPCTSTR Path);

// Operations
	bool	InsertClipPlayer(int SlotIdx, LPCTSTR Path);
	bool	LoadClipPlayer(int SlotIdx, LPCTSTR Path);
	int		FindFirstClipPlayer() const;
	bool	SetProject(const CFFProject& Project);

// Implementation
protected:
// Constants
	enum {	// create action
		CA_INSERT,
		CA_LOAD,
	};

// Data members
	CString	m_ClipPlayerPath;	// path of clip player plugin

// Helpers
	bool	CreateClipPlayer(int SlotIdx, LPCTSTR Path, UINT Action);
};

inline CClipEngine::CClipEngine(CRenderer& Renderer) :
	CMidiEngine(Renderer)
{
}

inline void CClipEngine::SetClipPlayerPath(LPCTSTR Path)
{
	m_ClipPlayerPath = Path;
}

inline bool CClipEngine::InsertClipPlayer(int SlotIdx, LPCTSTR Path)
{
	return(CreateClipPlayer(SlotIdx, Path, CA_INSERT));
}

inline bool CClipEngine::LoadClipPlayer(int SlotIdx, LPCTSTR Path)
{
	return(CreateClipPlayer(SlotIdx, Path, CA_LOAD));
}
