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
		02		04nov06	add MIDI assignments
		03		04nov06	V2 files omit FFRow tag
		04		12dec06	make ModWave short to save space
		05		19dec06	add routing
		06		28jan07	in Copy, include MIDI assignments and routing
		07		29jan07	add Serialize
		08		30jan07	add metaplugin
		09		01feb07	in CMidiInfo, rename row to parm
		10		08aug07 add DeletePlugin
		11		23nov07	support Unicode
		12		29jan08	in Read, use SetMsg to fix warnings
		13		19apr10	move routing here
		14		24apr10	in Read, allow reinit
		15		27apr10	rename plugin index to slot index
		16		08may10	add clip path
		17		02may11	add load balance

		container for a chain of freeframe plugins

*/

#include "stdafx.h"
#include "Resource.h"
#include "FFProject.h"

// define format strings for each line type
#define	FFP_FILE_ID	_T("FFProject\t%d\n")				// file identifier and version
#define	FFP_VIDEO_PATH_W	_T("FFVideoPath\t\"%s\"\n")		// path to source video or picture
#define	FFP_VIDEO_PATH_R	_T("FFVideoPath\t\"%[^\"]\"\n")	// path to source video or picture
#define	FFP_PLUGINS	_T("FFPlugins\t%d\n")				// number of loaded plugins
#define	FFP_CURSEL	_T("FFCurSel\t%d\n")				// index of current selected plugin
#define	FFP_SPEED	_T("FFSpeed\t%g\n")					// master speed, as a scaling factor
#define	FFP_PLUG_W	_T("FFPlug\t%d\t%d\t\"%s\"\n")	// # of parms, bypass, plugin path
#define	FFP_CLIP_W	_T("FFPlug\t%d\t%d\t\"%s\"\t\"%s\"\n")	// # of parms, bypass, plugin path, clip path
#define	FFP_PLUG_R	_T("FFPlug\t%d\t%d\t\"%[^\"]\"\t\"%[^\"]\"\n")	// # of parms, bypass, plugin path, clip path
#define	FFP_ROW_V1	_T("FFRow\t%g\t%g\t%g\t%d\t%d\t%g\t%g\n")	// used in V1 files only
					// ParmVal, SelMin, SelMax, ModEnab, ModWave, ModFreq, ModPW
#define	FFP_ROW_V2	_T("%g\t%g\t%g\t%d\t%d\t%g\t%g\n")			// V2 files omit FFRow tag
					// ParmVal, SelMin, SelMax, ModEnab, ModWave, ModFreq, ModPW
#define	MIDI_ASSIGNS	_T("MidiAssigns\t%d\n")
#define	MIDI_ASSIGN_ROW	_T("%d\t%d\t%d\t%g\t%d\t%d\t%d\n")
						// SlotIdx, RowIdx, PropIdx, Range, Event, Chan, Ctrl
#define	ROUTE_COUNT	_T("Routes\t%d\n")		// number of input connections
#define	ROUTE_ROW	_T("%d\t%d\t%d\n")		// source plugin, target plugin, input index
#define	LOAD_BAL_HDR	_T("LoadBal\t%d\n")	// # of load balance rows
#define	LOAD_BAL_ROW	_T("%d\t%d\n")	// slot index, thread count

CFFProject::CFFProject()
{
	m_PlugInfo.SetSize(0);
	m_VideoPath.Empty();
	m_Version = FILE_VERSION;
	m_CurSel = -1;
	m_Speed = 1;
}

void CFFProject::Copy(const CFFProject& Project)
{
	m_PlugInfo.Copy(Project.m_PlugInfo);
	m_MidiAssignList.Copy(Project.m_MidiAssignList);
	m_VideoPath = Project.m_VideoPath;
	m_Routing.Copy(Project.m_Routing);
	m_Version = Project.m_Version;
	m_CurSel = Project.m_CurSel;
	m_Speed = Project.m_Speed;
	m_Metaplugin = Project.m_Metaplugin;
}

bool CFFProject::Write(CStdioFile& fp) const
{
	int	slots = m_PlugInfo.GetSize();
	CString	s;
	s.Format(FFP_FILE_ID, FILE_VERSION);
	fp.WriteString(s);
	s.Format(FFP_VIDEO_PATH_W, m_VideoPath);
	fp.WriteString(s);
	s.Format(FFP_PLUGINS, slots);
	fp.WriteString(s);
	s.Format(FFP_CURSEL, m_CurSel);
	fp.WriteString(s);
	s.Format(FFP_SPEED, m_Speed);
	fp.WriteString(s);
	CLoadBalanceArray	LoadBal;
	int	i;
	for (i = 0; i < slots; i++) {
		const CFFPlugInfo	&ffpi = m_PlugInfo[i];
		int	rows = ffpi.m_Parm.GetSize();
		LPCTSTR	fmt = ffpi.m_ClipPath.IsEmpty() ? FFP_PLUG_W : FFP_CLIP_W;
		s.Format(fmt, ffpi.m_Parm.GetSize(), ffpi.m_Bypass, ffpi.m_Path, ffpi.m_ClipPath);
		fp.WriteString(s);
		for (int j = 0; j < rows; j++) {
			const CFFPlugInfo::FFPARM_INFO& ip = ffpi.m_Parm[j];
			s.Format(FFP_ROW_V2, ip.Val, ip.ModRange.Start, ip.ModRange.End, 
				ip.ModEnab, ip.ModWave, ip.ModFreq, ip.ModPW);
			fp.WriteString(s);
		}
		if (ffpi.m_Threads > 1) {
			LOAD_BALANCE	lb = {i, ffpi.m_Threads};
			LoadBal.Add(lb);
		}
	}
	int	asscnt = m_MidiAssignList.GetSize();
	s.Format(MIDI_ASSIGNS, asscnt);
	fp.WriteString(s);
	for (i = 0; i < asscnt; i++) {
		const CMidiAssign&	ma = m_MidiAssignList[i];
		s.Format(MIDI_ASSIGN_ROW, ma.m_SlotIdx, ma.m_ParmIdx, ma.m_PropIdx,
			ma.m_Range, ma.m_Event, ma.m_Chan, ma.m_Ctrl);
		fp.WriteString(s);
	}
	int	routes = m_Routing.GetSize();
	s.Format(ROUTE_COUNT, routes);
	fp.WriteString(s);
	for (i = 0; i < routes; i++) {
		const ROUTE&	cn = m_Routing[i];
		s.Format(ROUTE_ROW, cn.SrcSlot, cn.DstSlot, cn.InpIdx);
		fp.WriteString(s);
	}
	m_Metaplugin.Write(fp);
	int	LoadBalCount = LoadBal.GetSize();
	s.Format(LOAD_BAL_HDR, LoadBalCount);
	fp.WriteString(s);
	for (i = 0; i < LoadBalCount; i++) {
		const LOAD_BALANCE&	lb = LoadBal[i];
		s.Format(LOAD_BAL_ROW, lb.SlotIdx, lb.Threads);
		fp.WriteString(s);
	}
	return(TRUE);
}

bool CFFProject::Read(CStdioFile& fp)
{
	CString	s;
	fp.ReadString(s);
	if (_stscanf(s, FFP_FILE_ID, &m_Version) < 1)
		return(FALSE);
	fp.ReadString(s);
	m_VideoPath.Empty();
	LPCTSTR	p = m_VideoPath.GetBuffer(MAX_PATH);
	if (_stscanf(s, FFP_VIDEO_PATH_R, p) < 0)	// path can be empty
		return(FALSE);
	m_VideoPath.ReleaseBuffer();
	fp.ReadString(s);
	int	slots;
	if (_stscanf(s, FFP_PLUGINS, &slots) < 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FFP_CURSEL, &m_CurSel) < 1)
		return(FALSE);
	fp.ReadString(s);
	if (_stscanf(s, FFP_SPEED, &m_Speed) < 1)
		return(FALSE);
	m_PlugInfo.SetSize(slots);
	int	i;
	LPCTSTR	RowFmt = m_Version >= 2 ? FFP_ROW_V2 : FFP_ROW_V1;
	for (i = 0; i < slots; i++) {
		CFFPlugInfo	&ffpi = m_PlugInfo[i];
		int	rows, bypass;
		LPCTSTR	path = ffpi.m_Path.GetBuffer(MAX_PATH);
		LPCTSTR	ClipPath = ffpi.m_ClipPath.GetBuffer(MAX_PATH);
		fp.ReadString(s);
		if (_stscanf(s, FFP_PLUG_R, &rows, &bypass, path, ClipPath) < 2)	// paths can be empty
			return(FALSE);
		ffpi.m_Path.ReleaseBuffer();
		ffpi.m_ClipPath.ReleaseBuffer();
		ffpi.m_Bypass = bypass != 0;
		ffpi.m_Parm.SetSize(rows);
		for (int j = 0; j < rows; j++) {
			CFFPlugInfo::FFPARM_INFO& ip = ffpi.m_Parm[j];
			fp.ReadString(s);
			int	ModEnab;
			int	ModWave;
			if (_stscanf(s, RowFmt, &ip.Val, &ip.ModRange.Start, &ip.ModRange.End,
				 &ModEnab, &ModWave, &ip.ModFreq, &ip.ModPW) < 7)
				return(FALSE);
			ip.ModEnab = ModEnab != 0;	// convert int to bool
			ip.ModWave = static_cast<short>(ModWave);	// convert int to short
		}
	}
	if (m_Version >= 2) {
		fp.ReadString(s);
		int	asscnt;
		if (_stscanf(s, MIDI_ASSIGNS, &asscnt) < 1)
			return(FALSE);
		m_MidiAssignList.SetSize(asscnt);
		for (i = 0; i < asscnt; i++) {
			fp.ReadString(s);
			CMidiAssign&	ma = m_MidiAssignList[i];
			int	event, chan, ctrl;	// these aren't ints
			if (_stscanf(s, MIDI_ASSIGN_ROW, &ma.m_SlotIdx, &ma.m_ParmIdx, &ma.m_PropIdx,
				&ma.m_Range, &event, &chan, &ctrl) < 7)
				return(FALSE);
			ma.SetMsg(event, chan, ctrl);
		}
	} else
		m_MidiAssignList.RemoveAll();
	if (m_Version >= 3) {
		fp.ReadString(s);
		int	routes;
		if (_stscanf(s, ROUTE_COUNT, &routes) < 1)
			return(FALSE);
		m_Routing.SetSize(routes);
		for (i = 0; i < routes; i++) {
			fp.ReadString(s);
			ROUTE&	cn = m_Routing[i];
			if (_stscanf(s, ROUTE_ROW, &cn.SrcSlot, &cn.DstSlot, &cn.InpIdx) < 3)
				return(FALSE);
		}
	} else
		m_Routing.RemoveAll();
	if (m_Version >= 4) {
		if (!m_Metaplugin.Read(fp))
			return(FALSE);
	}
	if (m_Version >= 7) {
		fp.ReadString(s);
		int	LoadBalCount;
		if (_stscanf(s, LOAD_BAL_HDR, &LoadBalCount) < 1)
			return(FALSE);
		for (i = 0; i < LoadBalCount; i++) {
			fp.ReadString(s);
			LOAD_BALANCE	lb;
			if (_stscanf(s, LOAD_BAL_ROW, &lb.SlotIdx, &lb.Threads) < 2)
				return(FALSE);
			m_PlugInfo[lb.SlotIdx].m_Threads = lb.Threads;
		}
	}
	return(TRUE);
}

bool CFFProject::Write(LPCTSTR Path) const
{
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeCreate | CFile::modeWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		if (!Write(fp)) {
			CString	s;
			AfxFormatString1(s, IDS_FF_CANT_WRITE, Path);
			AfxMessageBox(s);
			return(FALSE);
		}
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CFFProject::Read(LPCTSTR Path)
{
	CStdioFile	fp;
	CFileException	e;
	if (!fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite, &e)) {
		e.ReportError();
		return(FALSE);
	}
	TRY {
		if (!Read(fp)) {
			CString	s;
			AfxFormatString1(s, IDS_FF_CANT_READ, Path);
			AfxMessageBox(s);
			return(FALSE);
		}
	}
	CATCH(CFileException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

void CFFProject::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_SIG;
		ar << m_Version;
		int	slots = m_PlugInfo.GetSize();
		ar << slots;
		for (int i = 0; i < slots; i++)
			m_PlugInfo[i].Serialize(ar, m_Version);
		m_MidiAssignList.Serialize(ar);
		ar << m_VideoPath;
		m_Routing.Serialize(ar);
		ar << m_CurSel;
		ar << m_Speed;
		m_Metaplugin.Serialize(ar, m_Version);
	} else {
		int	sig;
		ar >> sig;
		ar >> m_Version;
		if (sig != ARCHIVE_SIG || m_Version > FILE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		int	slots;
		ar >> slots;
		m_PlugInfo.SetSize(slots);
		for (int i = 0; i < slots; i++)
			m_PlugInfo[i].Serialize(ar, m_Version);
		m_MidiAssignList.Serialize(ar);
		ar >> m_VideoPath;
		m_Routing.Serialize(ar);
		ar >> m_CurSel;
		ar >> m_Speed;
		m_Metaplugin.Serialize(ar, m_Version);
	}
}
