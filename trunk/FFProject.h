// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		30jul06	initial version
		01		04nov06	add MIDI assignments
		02		19dec06	add routing
		03		29jan07	add Serialize
		04		30jan07	add metaplugin
		05		08aug07 add DeletePlugin
		06		23nov07	support Unicode
		07		23dec07	bump version number
		08		19apr10	move routing here
		09		08may10	bump version for clip path
		10		02may11	add load balance, bump version

		container for a chain of freeframe plugins

*/

#ifndef CFFPROJECT_INCLUDED
#define CFFPROJECT_INCLUDED

#include "ArrayEx.h"
#include "FFPlugInfo.h"
#include "MidiInfo.h"
#include "Metaplugin.h"

class CFFProject : public WObject {
public:
// Construction
	CFFProject();
	CFFProject(const CFFProject& Project);
	CFFProject& operator=(const CFFProject& Project);

// Types
	typedef CArrayEx<CFFPlugInfo, CFFPlugInfo&> CFFPlugInfoArray;
	typedef CArrayEx<CMidiAssign, CMidiAssign&> CMidiAssignList;
	typedef struct tagROUTE {
		int		SrcSlot;	// source plugin's slot index, or -1 for default
		int		DstSlot;	// target plugin's slot index
		int		InpIdx;		// index of target plugin's input
	} ROUTE;
	typedef CArrayEx<ROUTE, ROUTE&> CRouting;
	typedef struct tagLOAD_BALANCE {
		int		SlotIdx;	// slot index of plugin
		int		Threads;	// plugin's thread count
	} LOAD_BALANCE;
	typedef CArrayEx<LOAD_BALANCE, LOAD_BALANCE&> CLoadBalanceArray;

// Constants
	enum {
		ARCHIVE_SIG = 0x50524646,	// archive signature (FFRP)
		FILE_VERSION = 7	// current file version number
	};

// Public data
	CFFPlugInfoArray	m_PlugInfo;	// array of plugin data
	CMidiAssignList		m_MidiAssignList;	// array of MIDI assignments
	CString	m_VideoPath;	// path to source video or picture
	CRouting	m_Routing;	// array of plugin connections
	int		m_Version;	// read file version number
	int		m_CurSel;	// current selection
	float	m_Speed;	// master speed
	CMetaplugin	m_Metaplugin;	// metaplugin data

// Operations
	bool	Write(CStdioFile& fp) const;
	bool	Read(CStdioFile& fp);
	bool	Write(LPCTSTR Path) const;
	bool	Read(LPCTSTR Path);
	void	Serialize(CArchive& ar);

protected:
// Helpers
	void	Copy(const CFFProject& Info);
};

inline CFFProject::CFFProject(const CFFProject& Info)
{
	Copy(Info);
}

inline CFFProject& CFFProject::operator=(const CFFProject& Info)
{
	Copy(Info);
	return(*this);
}

#endif
