// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		30jul06	initial version
		01		07oct06	add modulation enable
		02		24nov06	add MIDI to parm and plug info
		03		12dec06	make ModWave short to save space
		04		29jan07	add Serialize
		05		23nov07	support Unicode
		06		19apr10	add range struct
		07		08may10	add clip path
		08		02may11	add thread count, bump version

		container for information about a freeframe plugin

*/

#ifndef CFFPLUGININFO_INCLUDED
#define CFFPLUGININFO_INCLUDED

#include "ArrayEx.h"
#include "MidiInfo.h"

class CFFPlugInfo : public WObject {
public:
// Types
	typedef struct tagFRANGE {	// float range
		float	Start;		// start of range, or -1 if none
		float	End;		// end of range, or -1 if none
	} FRANGE;
	typedef struct tagFFPARM_INFO {	// parameter information
		float	Val;		// parameter's current value
		FRANGE	ModRange;	// modulation range
		bool	ModEnab;	// true if modulation is enabled
		char	Rsvd;		// reserved, initialized to zero
		short	ModWave;	// index of modulation waveform
		float	ModFreq;	// modulation frequency in hertz
		float	ModPW;		// pulse width, from 0..1; only meaningful for pulse wave
		CMidiInfo	MidiInfo[PARAM_MIDI_PROPS];	// parameter's MIDI properties
	} FFPARM_INFO;
	typedef struct tagFFPLUG_INFO {	// plugin information
		TCHAR	Path[MAX_PATH];	// path of plugin's DLL
		TCHAR	ClipPath[MAX_PATH];	// path of clip if any
		bool	Bypass;		// true if plugin is bypassed
		char	Rsvd[3];	// reserved, initialized to zero
		int		Threads;	// plugin's number of threads
		int		Parms;		// plugin's number of parameters
		CMidiInfo	MidiInfo[PLUGIN_MIDI_PROPS];	// plugin's MIDI properties
		FFPARM_INFO	Parm[1];	// array of parameter info
	} FFPLUG_INFO;
	typedef	CArrayEx<FFPARM_INFO, FFPARM_INFO&> CParmInfoArray;

// Construction
	CFFPlugInfo();
	CFFPlugInfo(const tagFFPLUG_INFO& Info);
	CFFPlugInfo(const CFFPlugInfo& Info);
	CFFPlugInfo& operator=(const CFFPlugInfo& Info);

// Constants
	enum {
		INFO_VERSION = 5	// uniquely identify each version of info structs
	};

// Public data
	CString	m_Path;			// path of plugin's DLL
	CString	m_ClipPath;		// path of clip if any
	bool	m_Bypass;		// true if plugin is bypassed
	int		m_Threads;		// plugin's number of threads
	CParmInfoArray	m_Parm;	// array of parameter info
	CMidiInfo	m_MidiInfo[PLUGIN_MIDI_PROPS];	// plugin's MIDI properties

// Attributes
	DWORD	GetInfoSize() const;
	void	GetInfo(FFPLUG_INFO& Info) const;
	void	SetInfo(const FFPLUG_INFO& Info);

// Operations
	void	Serialize(CArchive& ar, int Version);

protected:
// Helpers
	void	Copy(const CFFPlugInfo& Info);
};

inline CFFPlugInfo::CFFPlugInfo(const CFFPlugInfo& Info)
{
	Copy(Info);
}

inline CFFPlugInfo& CFFPlugInfo::operator=(const CFFPlugInfo& Info)
{
	Copy(Info);
	return(*this);
}

#endif
