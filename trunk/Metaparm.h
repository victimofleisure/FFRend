// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
		01		22dec07	add Reset
		02		23dec07	add MIDI info
		03		25dec07	add metaparameter groups
		04		27apr10	rename plugin index to slot index

        metaparameter data
 
*/

#ifndef CMETAPARM_INCLUDED
#define CMETAPARM_INCLUDED

#include "MidiInfo.h"
#include "MetaparmGroup.h"

enum {	// Freeframe constraints
	FF_MAX_PLUGIN_NAME	= 16,	// maximum length of a plugin name
	FF_MAX_PARAM_NAME	= 16,	// maximum length of a parameter name
	FF_MAX_UNIQUE_ID	= 4		// maximum length of a plugin's unique ID
};

// metaparameter control targets are uniquely addressed using same scheme
// used for MIDI control targets; see CMidiAssign comment in MidiInfo.h
typedef struct tagMETAPARM_TARGET {
	int		SlotIdx;	// index into main plugin array, or if negative, special
						// plugin index which alters meaning of other indices
	int		ParmIdx;	// index of plugin parameter, plugin, or miscellaneous
						// property, depending on value of m_SlotIdx
	int		PropIdx;	// index of parameter property, plugin property,
						// or unused, depending on value of m_SlotIdx
} METAPARM_TARGET;

// version 1 of METAPARM_BASE_INFO, for backwards compatibility
typedef struct tagMETAPARM_BASE_INFO_V1 {
	METAPARM_TARGET	m_Target;	// control target of this metaparameter; see above
	float	m_RangeStart;		// start of target range; range may be inverted
	float	m_RangeEnd;			// end of target range; range may be inverted
	float	m_Value;			// metaparameter value, normalized from 0 to 1
	UINT	m_PluginUID;		// unique identifer of plugin associated with this
								// metaparameter, or zero if none
} METAPARM_BASE_INFO_V1;

typedef struct tagMETAPARM_BASE_INFO : METAPARM_BASE_INFO_V1 {
	CMidiInfo	m_MidiInfo;		// MIDI properties
} METAPARM_BASE_INFO;

class CMetaparm : public CObject, public METAPARM_BASE_INFO {	// common base struct
	DECLARE_SERIAL(CMetaparm);
public:
// Construction
	CMetaparm();
	CMetaparm(const CMetaparm& Info);
	CMetaparm& operator=(const CMetaparm& Info);

// Types
	typedef METAPARM_TARGET TARGET;
	typedef CMetaparmGroup::CSlaveArray CSlaveArray;

// Public data
	// don't forget to add new members to ctor, Copy, and Serialize
	CString m_Name;			// metaparameter name, 16 characters maximum;
							// empty if control target is unassigned
	CSlaveArray	m_Slave;	// if master, array of slave indices
	int		m_Master;		// if slave, index of master, else -1
	static	int		m_Version;	// workaround for useless archive schema

// Attributes
	bool	IsAssigned() const;
	void	SetTargetPlugin(int SlotIdx);
	int		GetTargetPlugin() const;
	bool	IsMaster() const;
	bool	IsSlave() const;
	bool	IsGrouped() const;
	int		GetSlaveCount() const;

// Operations
	void	Reset();
	void	Serialize(CArchive& ar);

protected:
// Helpers
	void	Copy(const CMetaparm& Info);
};

template<> inline void AFXAPI 
SerializeElements<CMetaparm>(CArchive& ar, CMetaparm *pNewParm, int nCount)
{
	for (int i = 0; i < nCount; i++, pNewParm++)
		pNewParm->Serialize(ar);
}

inline CMetaparm::CMetaparm(const CMetaparm& Info)
{
	Copy(Info);
}

inline CMetaparm& CMetaparm::operator=(const CMetaparm& Info)
{
	Copy(Info);
	return(*this);
}

inline bool CMetaparm::IsAssigned() const
{
	return(!m_Name.IsEmpty());
}

inline int CMetaparm::GetSlaveCount() const
{
	return(m_Slave.GetSize());
}

inline bool CMetaparm::IsMaster() const
{
	return(m_Slave.GetSize() != 0);
}

inline bool CMetaparm::IsSlave() const
{
	return(m_Master >= 0);
}

inline bool CMetaparm::IsGrouped() const
{
	return(IsMaster() || IsSlave());
}

#endif
