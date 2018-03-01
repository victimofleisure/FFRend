// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03nov06	initial version
		01		27apr10	move from main frame to engine

        engine MIDI implementation
 
*/

#pragma once

#include "FFEngine.h"
#include "MidiTypes.h"
#include "FFProject.h"

class CMidiEngine : public CFFEngine {
public:
// Construction
	CMidiEngine(CRenderer& Renderer);

// Types
	typedef CFFProject::CMidiAssignList CMidiAssignList;
	typedef CMetaparm::TARGET METATARGET;
	
// Attributes
	void	SetMidiDefaults();
	bool	GetMidiInfo(int SlotIdx, int ParmIdx, int PropIdx, CMidiInfo& Info) const;
	bool	SetMidiInfo(int SlotIdx, int ParmIdx, int PropIdx, const CMidiInfo& Info);
	bool	GetMidiPlugName(int SlotIdx, CString& Name) const;
	bool	GetMidiParmName(int SlotIdx, int ParmIdx, CString& Name) const;
	int		GetMidiParmCount(int SlotIdx) const;
	int		GetMidiPageType(int SlotIdx) const;
	int		GetMidiPropCount(int PageType) const;
	void	GetMidiPropName(int PageType, int PropIdx, CString& Name) const;
	bool	GetMidiMapping(const CMidiInfo& Info, int& SlotIdx, int& ParmIdx, int& PropIdx) const;
	void	GetMidiAssignments(CMidiAssignList& AssList) const;
	void	SetMidiAssignments(const CMidiAssign *AssList, int AssCount);
	void	SetMidiAssignments(const CMidiAssignList& AssList);
	void	SetMidiProperty(int Event, MIDI_MSG Msg);
	float	GetMidiProperty(int SlotIdx, int ParmIdx, int PropIdx) const;
	float	GetMidiProperty(const METATARGET& Target);
	void	GetProject(CFFProject& Project);
	bool	SetProject(const CFFProject& Project);
	float	GetSpeedNorm() const;
	void	SetSpeedNorm(float Speed);
	CMetaplugin&	GetMetaplugin();

// Operations
	void	ResetMidiMap();
	void	MakeMidiMap();
	void	AssignMidi(int SlotIdx, int ParmIdx, int PropIdx, const CMidiInfo& Info);
	static	void	MakeTarget(int PageType, int SlotIdx, int ParmIdx, int PropIdx, METATARGET& Target);
	bool	UpdateMetaparmLinks();
	void	ApplyMetaparm(CMetaparm& Metaparm);

// Implementation
protected:
// Types
	typedef union tagMIDI_TARGET {
		DWORD	dw;
		struct {	// use short to avoid garbage byte
			short	SlotIdx;	// plugin's slot index; can be negative
			BYTE	ParmIdx;	// parameter index
			BYTE	PropIdx;	// property index
		} s;
	} MIDI_TARGET;
	typedef struct tagMIDI_MAP {
		MIDI_TARGET	Ctrl[MIDI_CHANS][MIDI_PARMS];
		MIDI_TARGET	Note[MIDI_CHANS][MIDI_PARMS];
		MIDI_TARGET	Pitch[MIDI_CHANS];
	} MIDI_MAP;

// Constants
	static const int	m_ParamMidiPropName[PARAM_MIDI_PROPS];
	static const int	m_PluginMidiPropName[PLUGIN_MIDI_PROPS];
	static const int	m_MiscMidiPropName[MISC_MIDI_PROPS];

// Member data
	CMidiInfo	m_MiscMidiProp[MISC_MIDI_PROPS];	// miscellaneous MIDI properties
	MIDI_MAP	m_MidiMap;		// map of MIDI message targets
	CMetaplugin	m_Metaplugin;	// metaplugin data

// Overrides
	virtual	void	OnEndSlotChange();

// Helpers
	bool	GetMidiMapping(const CMidiInfo& Info, MIDI_TARGET& Target) const;
	void	SetMidiMapping(int SlotIdx, int ParmIdx, int PropIdx, const CMidiInfo& Info);
	void	DumpMidi();
	int		RemoveMidiDups(int SlotIdx, const CMidiAssignList& PrevAss);
	void	RebuildMetaparmArray();
};

inline int CMidiEngine::GetMidiPageType(int SlotIdx) const
{
	return(max(-SlotIdx, 0));
}

inline CMetaplugin& CMidiEngine::GetMetaplugin()
{
	return(m_Metaplugin);
}

inline float CMidiEngine::GetMidiProperty(const METATARGET& Target)
{
	return(GetMidiProperty(Target.SlotIdx, Target.ParmIdx, Target.PropIdx));
}
