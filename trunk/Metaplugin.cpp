// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version
		01		01feb07	add metaparameters
		02		23nov07	support Unicode
		03		22dec07	in Read, allow unnamed metaparameter
		04		23dec07	pass version number to CMetaparm
		05		25dec07	add metaparameter groups
		06		27apr10	rename plugin index to slot index

        metaplugin data
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "Metaplugin.h"

#define	METAPLUG	_T("Metaplug\t%d\n")	// non-zero if metaplugin properties follow
#define METAPLUG_DETAILS	_T("%d\t%d\t%d\n")
					// PluginType, MajorVersion, MinorVersion
#define	METAPARM_COUNT	_T("Metaparms\t%d\n")	// number of metaparameters
#define	METAPARM_W	_T("%s\t%d\t%d\t%d\t%g\t%g\t%g\n")
					// Name, SlotIdx, ParmIdx, PropIdx, RangeStart, RangeEnd, Value
#define	METAPARM_R_NAME	_T("%16[^\t]%n")	// Name
#define	METAPARM_R	_T("%d\t%d\t%d\t%g\t%g\t%g\n")
					// SlotIdx, ParmIdx, PropIdx, RangeStart, RangeEnd, Value
#define METAPARM_GROUPS	_T("MPGroups\t%d\n")	// number of metaparameter groups

CMetaplugin::CMetaplugin()
{
	m_PluginType = 0;
	ZeroMemory(&m_Caps, sizeof(m_Caps));
	m_PluginMajorVersion = 1;
	m_PluginMinorVersion = 0;
	m_IsEmbedded = FALSE;
	m_ComprLength = 0;
}

void CMetaplugin::Copy(const CMetaplugin& Plug)
{
	m_PluginName			= Plug.m_PluginName;
	m_UniqueID				= Plug.m_UniqueID;
	m_Description			= Plug.m_Description; 
	m_AboutText				= Plug.m_AboutText;
	m_PluginType			= Plug.m_PluginType;
	m_Caps					= Plug.m_Caps;
	m_PluginMajorVersion	= Plug.m_PluginMajorVersion;
	m_PluginMinorVersion	= Plug.m_PluginMinorVersion;
	m_InpTargetIdx.Copy(Plug.m_InpTargetIdx);
	m_IsEmbedded			= Plug.m_IsEmbedded;
	m_ComprLength			= Plug.m_ComprLength;
	m_EmbDllInfo.Copy(Plug.m_EmbDllInfo);
	m_Metaparm.Copy(Plug.m_Metaparm);
}

void CMetaplugin::Serialize(CArchive& ar, int Version)
{
	CMetaparm::m_Version = Version;	// pass version number to CMetaparm
	if (ar.IsStoring()) {
#ifdef METAFFREND	// MetaFFRend DLL never stores, so save space
		ASSERT(0);
#else
		ar << m_PluginName;
		ar << m_UniqueID;
		ar << m_Description;
		ar << m_AboutText;
		ar << m_PluginType;
		ar.Write(&m_Caps, sizeof(m_Caps));
		ar << m_PluginMajorVersion;
		ar << m_PluginMinorVersion;
		m_InpTargetIdx.Serialize(ar);
		ar << m_IsEmbedded;
		ar << m_ComprLength;
		m_EmbDllInfo.Serialize(ar);
		m_Metaparm.Serialize(ar);
#endif
	} else {
		ar >> m_PluginName;
		ar >> m_UniqueID;
		ar >> m_Description;
		ar >> m_AboutText;
		ar >> m_PluginType;
		ar.Read(&m_Caps, sizeof(m_Caps));
		ar >> m_PluginMajorVersion;
		ar >> m_PluginMinorVersion;
		m_InpTargetIdx.Serialize(ar);
		ar >> m_IsEmbedded;
		ar >> m_ComprLength;
		m_EmbDllInfo.Serialize(ar);
		m_Metaparm.Serialize(ar);
	}
}

void CMetaplugin::Write(CStdioFile& fp) const
{
	CString	s;
	int	metaprops = !m_PluginName.IsEmpty();
	s.Format(METAPLUG, metaprops);
	fp.WriteString(s);
	if (metaprops) {
		fp.WriteString(m_PluginName + "\n");
		fp.WriteString(m_UniqueID + "\n");
		fp.WriteString(m_Description + "\n");
		fp.WriteString(m_AboutText + "\n");
		s.Format(METAPLUG_DETAILS, m_PluginType, 
			m_PluginMajorVersion, m_PluginMinorVersion);
		fp.WriteString(s);
		CString	t;
		int	inputs = m_InpTargetIdx.GetSize();
		s.Format(_T("%d\n"), inputs);
		for (int i = 0; i < inputs; i++) {
			t.Format(_T("%d\n"), m_InpTargetIdx[i]);
			s += t;
		}
		fp.WriteString(s);
	}
	int	metaparms = m_Metaparm.GetSize();
	s.Format(METAPARM_COUNT, metaparms);
	fp.WriteString(s);
	int	i;
	for (i = 0; i < metaparms; i++) {
		const CMetaparm& mp = m_Metaparm[i];
		s.Format(METAPARM_W, mp.m_Name, 
			mp.m_Target.SlotIdx, mp.m_Target.ParmIdx, mp.m_Target.PropIdx, 
			mp.m_RangeStart, mp.m_RangeEnd, mp.m_Value);
		fp.WriteString(s);
	}
	CMetaparmGroupList	mpgl;
	m_Metaparm.GetGroups(mpgl);
	int	groups = mpgl.GetSize();
	s.Format(METAPARM_GROUPS, groups);
	fp.WriteString(s);
	for (i = 0; i < groups; i++) {
		CMetaparmGroup	mpg = mpgl[i];
		int	Slaves = mpg.GetSlaveCount();
		s.Format(_T("%d\t%d"), mpg.m_Master, Slaves);
		CString	t;
		for (int j = 0; j < Slaves; j++) {
			t.Format(_T("\t%d"), mpg.m_Slave[j]);
			s += t;
		}
		s += _T("\n");
		fp.WriteString(s);
	}
}

bool CMetaplugin::Read(CStdioFile& fp)
{
	CString	s;
	fp.ReadString(s);
	int	metaprops;
	if (_stscanf(s, METAPLUG, &metaprops) < 1)
		return(FALSE);
	if (metaprops) {
		fp.ReadString(m_PluginName);
		fp.ReadString(m_UniqueID);
		fp.ReadString(m_Description);
		fp.ReadString(m_AboutText);
		fp.ReadString(s);
		if (_stscanf(s, METAPLUG_DETAILS, &m_PluginType, 
			&m_PluginMajorVersion, &m_PluginMinorVersion) < 3)
			return(FALSE);
		fp.ReadString(s);
		int	inputs, inpidx;
		if (_stscanf(s, _T("%d"), &inputs) < 1)
			return(FALSE);
		m_InpTargetIdx.SetSize(inputs);
		for (int i = 0; i < inputs; i++) {
			fp.ReadString(s);
			if (_stscanf(s, _T("%d"), &inpidx) < 1)
				return(FALSE);
			m_InpTargetIdx[i] = inpidx;
		}
	}
	fp.ReadString(s);
	int	metaparms;
	if (_stscanf(s, METAPARM_COUNT, &metaparms) < 1)
		return(FALSE);
	m_Metaparm.SetSize(metaparms);
	for (int i = 0; i < metaparms; i++) {
		fp.ReadString(s);
		CMetaparm& mp = m_Metaparm[i];
		LPCTSTR	name = mp.m_Name.GetBuffer(FF_MAX_PARAM_NAME + 1);	// allow for terminator
		int	CharsRead = 0;
		_stscanf(s, METAPARM_R_NAME, name, &CharsRead);	// name may be empty
		LPCTSTR	line = s;
		if (_stscanf(&line[CharsRead], METAPARM_R,
			&mp.m_Target.SlotIdx, &mp.m_Target.ParmIdx, &mp.m_Target.PropIdx, 
			&mp.m_RangeStart, &mp.m_RangeEnd, &mp.m_Value) < 6)
			return(FALSE);
		mp.m_Name.ReleaseBuffer();
	}
	fp.ReadString(s);
	int	groups;
	if (_stscanf(s, METAPARM_GROUPS, &groups) == 1) {
		CMetaparmGroupList	mpgl;
		mpgl.SetSize(groups);
		for (int i = 0; i < groups; i++) {
			fp.ReadString(s);
			CMetaparmGroup&	mpg = mpgl[i];
			int	Slaves, BytesRead;
			_stscanf(s, _T("%d\t%d%n"), &mpg.m_Master, &Slaves, &BytesRead);
			mpg.m_Slave.SetSize(Slaves);
			LPCTSTR	pBuf = s;
			for (int j = 0; j < Slaves; j++) {
				pBuf += BytesRead;
				_stscanf(pBuf, _T("%d%n"), &mpg.m_Slave[j], &BytesRead);
			}
		}
		m_Metaparm.SetGroups(mpgl);
	}
	return(TRUE);
}
