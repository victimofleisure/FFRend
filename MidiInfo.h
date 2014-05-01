// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03nov06	initial version
 		01		01feb07	rename row to parm
		02		22dec07	add CMidiAssign ctor
		03		23dec07	add metaparameter page
		04		29jan08	add SetMsg to fix warnings
		05		27apr10	rename plugin index to slot index

		MIDI assignment information
 
*/

#ifndef CMIDIINFO_DEFINED
#define	CMIDIINFO_DEFINED

enum {	// MIDI event types
	MET_OFF,
	MET_CTRL,
	MET_NOTE,
	MET_PITCH,
	MIDI_EVENT_TYPES
};

enum {	// MIDI page types
	MPT_PARAM,		// parameter page
	MPT_PLUGIN,		// plugin page
	MPT_MISC,		// miscellaneous page
	MPT_METAPARM,	// metaparameter page
	MIDI_PAGE_TYPES
};

enum {	// special plugin indices, all negative
	SPI_PLUGIN		= -MPT_PLUGIN,		// plugin page
	SPI_MISC		= -MPT_MISC,		// miscellaneous page
	SPI_METAPARM	= -MPT_METAPARM,	// metaparameter page
	SPI_FIRST		= SPI_METAPARM,		// lowest valid special plugin index
	SPI_INVALID		= SHRT_MIN			// invalid control target
};

enum {	// parameter MIDI properties
	MP_PARAM,
	MP_MOD_ENAB,
	MP_MOD_WAVE,
	MP_MOD_FREQ,
	MP_MOD_PW,
	PARAM_MIDI_PROPS
};

enum {	// plugin MIDI properties
	MP_BYPASS,
	PLUGIN_MIDI_PROPS
};

enum {	// miscellaneous MIDI properties
	MP_MASTER_SPEED,
	MISC_MIDI_PROPS
};

class CMidiInfo {
public:
// Construction
	CMidiInfo();
	CMidiInfo(float Range, int Event, int Chan, int Ctrl);

// Attributes
	bool	IsDefault() const;
	void	SetMsg(int Event, int Chan, int Ctrl);

// Operations
	bool	operator==(const CMidiInfo& Info) const;
	bool	operator!=(const CMidiInfo& Info) const;
	void	SetDefault();

// Public data
	float	m_Range;	// range of the target property
	WORD	m_Event;	// type of MIDI event; use WORD to avoid garbage byte
	BYTE	m_Chan;		// MIDI channel number (0..15)
	BYTE	m_Ctrl;		// MIDI controller/note number (0..127)

// Constants
	static	const	CMidiInfo	m_Default;
	static	const	int		CmdToEvent[];
};

class CMidiAssign : public CMidiInfo {
public:
	CMidiAssign();
/*	
	Each MIDI control target has a unique address, consisting of plugin index,
	parameter index, and property index. These three indices form a coordinate
	space which has a variable number of dimensions, ranging from three to one.
	NOTE that plugin index is a SLOT index and may access an EMPTY slot.

	Plugin index can have special NEGATIVE values, which require the other two
	indices to be interpreted differently. These special plugin indices have a
	prefix of SPI_ and are defined in MidiInfo.h. In effect, all three indices
	are overloaded: the parameter and property indices have different meanings
	depending on the value of the plugin index.

	if (plugin index >= 0)	// 3D: target is a plugin parameter property 
	{
		plugin index	= index of slot within engine's slot array
		parameter index	= index of Freeframe parameter within this plugin
		property index	= index of parameter property; 0 to PARAM_MIDI_PROPS - 1
	}
	else if (plugin index == SPI_PLUGIN)	// 2D: target is a plugin property
	{
		parameter index	= index of slot within engine's slot array
		property index	= index of plugin property; 0 to PLUGIN_MIDI_PROPS - 1
	}
	else if (plugin index == SPI_MISC)		// 1D: target is a miscellaneous property
	{
		parameter index	= index of miscellaneous property; 0 to MISC_MIDI_PROPS - 1
		property index	= NOT USED
	}
	else if (plugin index == SPI_METAPARM)	// 1D: target is a metaparameter
	{
		parameter index	= index of metaparameter; 0 to metaparameters - 1
		property index	= NOT USED
	}
*/	
	int		m_SlotIdx;	// index of plugin in engine's slot array, or if negative,
						// special index which alters meaning of other indices
	int		m_ParmIdx;	// index of parameter within this plugin, or a special 
						// case, depending on value of m_SlotIdx; see above
	int		m_PropIdx;	// index of parameter or plugin property, or not used,
						// depending on value of m_SlotIdx; see above
};

inline bool CMidiInfo::IsDefault() const
{
	return(*this == m_Default);
}

inline bool CMidiInfo::operator==(const CMidiInfo& Info) const
{
	return(!memcmp(&Info, this, sizeof(CMidiInfo)));
}

inline bool CMidiInfo::operator!=(const CMidiInfo& Info) const
{
	return(!(Info == *this));
}

inline void CMidiInfo::SetDefault()
{
	*this = m_Default;
}

inline void CMidiInfo::SetMsg(int Event, int Chan, int Ctrl)
{
	m_Event = static_cast<WORD>(Event);
	m_Chan = static_cast<BYTE>(Chan);
	m_Ctrl = static_cast<BYTE>(Ctrl);
}

#endif
