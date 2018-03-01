// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		30jul06	initial version
		01		24nov06	add MIDI to parm and plug info
		02		12dec06	make ModWave short to save space
		03		29jan07	add Serialize
		04		23nov07	support Unicode
		05		08may10	add clip path
		06		02may11	add thread count

		container for information about a freeframe plugin

*/

#include "stdafx.h"
#include "FFPlugInfo.h"

CFFPlugInfo::CFFPlugInfo()
{
	m_Bypass = FALSE;
	m_Threads = 1;
}

CFFPlugInfo::CFFPlugInfo(const FFPLUG_INFO& Info)
{
	SetInfo(Info);
}

void CFFPlugInfo::Copy(const CFFPlugInfo& Info)
{
	m_Path = Info.m_Path;
	m_ClipPath = Info.m_ClipPath;
	m_Bypass = Info.m_Bypass;
	m_Threads = Info.m_Threads;
	m_Parm.Copy(Info.m_Parm);
	memcpy(m_MidiInfo, Info.m_MidiInfo, sizeof(m_MidiInfo));
}

DWORD CFFPlugInfo::GetInfoSize() const
{
	// FFPLUG_INFO::Parm allocates one element; subtract one to compensate
	return(sizeof(FFPLUG_INFO) + (m_Parm.GetSize() - 1) * sizeof(FFPARM_INFO));
}

void CFFPlugInfo::GetInfo(FFPLUG_INFO& Info) const
{
	_tcsncpy(Info.Path, m_Path, MAX_PATH);
	_tcsncpy(Info.ClipPath, m_ClipPath, MAX_PATH);
	Info.Bypass = m_Bypass;
	ZeroMemory(Info.Rsvd, sizeof(Info.Rsvd));
	Info.Threads = m_Threads;
	Info.Parms = m_Parm.GetSize();
	int	i;
	for (i = 0; i < Info.Parms; i++)
		Info.Parm[i] = m_Parm[i];
	for (i = 0; i < PLUGIN_MIDI_PROPS; i++)
		Info.MidiInfo[i] = m_MidiInfo[i];
}

void CFFPlugInfo::SetInfo(const FFPLUG_INFO& Info)
{
	m_Path = Info.Path;
	m_ClipPath = Info.ClipPath;
	m_Bypass = Info.Bypass;
	m_Threads = Info.Threads;
	m_Parm.SetSize(Info.Parms);
	int	i;
	for (i = 0; i < Info.Parms; i++)
		m_Parm[i] = Info.Parm[i];
	for (i = 0; i < PLUGIN_MIDI_PROPS; i++)
		m_MidiInfo[i] = Info.MidiInfo[i];
}

void CFFPlugInfo::Serialize(CArchive& ar, int Version)
{
	if (ar.IsStoring()) {
		ar << m_Path;
		if (Version >= 6)
			ar << m_ClipPath;
		int	Bypass = m_Bypass;	// for backwards compatibility with MFC6 archives
		ar << Bypass;
		if (Version >= 7)
			ar << m_Threads;
		m_Parm.Serialize(ar);
		ar.Write(m_MidiInfo, sizeof(m_MidiInfo));
	} else {
		ar >> m_Path;
		if (Version >= 6)
			ar >> m_ClipPath;
		int	Bypass;
		ar >> Bypass;
		m_Bypass = Bypass != 0;
		if (Version >= 7)
			ar >> m_Threads;
		m_Parm.Serialize(ar);
		ar.Read(m_MidiInfo, sizeof(m_MidiInfo));
	}
}
