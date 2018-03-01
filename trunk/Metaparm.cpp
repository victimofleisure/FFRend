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

#include "stdafx.h"
#include "Metaparm.h"
#include "MidiInfo.h"
#include "MetaparmGroup.h"

IMPLEMENT_SERIAL(CMetaparm, CObject, 1);

int	CMetaparm::m_Version;

CMetaparm::CMetaparm()
{
	m_Target.SlotIdx = SPI_INVALID;
	m_Target.ParmIdx = 0;
	m_Target.PropIdx = 0;
	m_RangeStart = 0;
	m_RangeEnd = 1;
	m_Value = 0;
	m_PluginUID = 0;
	m_Master = -1;
}

void CMetaparm::Copy(const CMetaparm& Info)
{
	METAPARM_BASE_INFO&	mpbi = *this;
	mpbi		= Info;	// copy base struct
	m_Name		= Info.m_Name;
	m_Slave.Copy(Info.m_Slave);
	m_Master	= Info.m_Master;
}

void CMetaparm::Reset()
{
	CMetaparm	DefaultParm;
	*this = DefaultParm;
}

void CMetaparm::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
#ifdef METAFFREND	// MetaFFRend DLL never stores, so save space
		ASSERT(0);
#else
		ar << m_Name;
		METAPARM_BASE_INFO&	mpbi = *this;
		ar.Write(&mpbi, sizeof(mpbi));	// write base struct
		m_Slave.Serialize(ar);
		ar << m_Master;
#endif
	} else {
		ar >> m_Name;
		if (m_Version < 5) {
			METAPARM_BASE_INFO_V1&	mpbi = *this;
			ar.Read(&mpbi, sizeof(mpbi));	// read version 1 base struct
		} else {
			METAPARM_BASE_INFO&	mpbi = *this;
			ar.Read(&mpbi, sizeof(mpbi));	// read base struct
			m_Slave.Serialize(ar);
			ar >> m_Master;
		}
	}
}

void CMetaparm::SetTargetPlugin(int SlotIdx)
{
	if (m_Target.SlotIdx >= 0)	// if target is a plugin parameter property 
		m_Target.SlotIdx = SlotIdx;	// set plugin index
	else if (m_Target.SlotIdx == SPI_PLUGIN)	// if target is a plugin property
		m_Target.ParmIdx = SlotIdx;	// parameter index is redefined as plugin index
	// otherwise target is a miscellaneous property, not associated with a plugin
}

int CMetaparm::GetTargetPlugin() const
{
	if (m_Target.SlotIdx >= 0)	// if target is a plugin parameter property 
		return(m_Target.SlotIdx);	// return plugin index
	if (m_Target.SlotIdx == SPI_PLUGIN)	// if target is a plugin property
		return(m_Target.ParmIdx);	// parameter index is redefined as plugin index
	return(-1);	// target is a miscellaneous property, not associated with a plugin
}
