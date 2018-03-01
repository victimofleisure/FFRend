// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03nov06	initial version
		01		24nov06	add RemoveMidiDups
		02		03jan07	for bypass via MIDI, don't touch modify flag
		03		19jan07	remove DoMidiSetup 
		04		21jan07	replace AfxGetMainWnd with GetThis
		05		01feb07	in CMidiInfo, rename row to parm
		06		01feb07	in SetMidiProperty, add metaparameter hack
		07		23nov07	support Unicode
		08		22dec07	in SetMidiAssignments, verify plugin slot is loaded
		09		22dec07	on invalid plugin index, fail instead of asserting
		10		23dec07	add metaparameter page
		11		25dec07	add metaparameter groups
		12		29jan08	add static casts to fix warnings
		13		27apr10	move from main frame to engine

        engine MIDI implementation
 
*/

#include "stdafx.h"
#include "FFRend.h"
#include "MidiEngine.h"
#include "FFPluginEx.h"
#include "FFRendDoc.h"
#include "FFRendView.h"
#include "FFPlugsRow.h"

// names of parameter MIDI properties; must match enum in MidiInfo.h
const int CMidiEngine::m_ParamMidiPropName[PARAM_MIDI_PROPS] = {
	IDS_MP_PARAMETER,
	IDS_MP_MOD_ENABLE,
	IDS_MP_MOD_WAVE,
	IDS_MP_MOD_FREQ,
	IDS_MP_MOD_PW
};

// names of plugin MIDI properties; must match enum in MidiInfo.h
const int CMidiEngine::m_PluginMidiPropName[PLUGIN_MIDI_PROPS] = {
	IDS_MP_BYPASS
};

// names of miscellaneous MIDI properties; must match enum in MidiInfo.h
const int CMidiEngine::m_MiscMidiPropName[MISC_MIDI_PROPS] = {
	IDS_MP_MASTER_SPEED
};

CMidiEngine::CMidiEngine(CRenderer& Renderer) :
	CFFEngine(Renderer)
{
	ResetMidiMap();
}

void CMidiEngine::SetMidiDefaults()
{
	CMidiInfo	def;
	int	plugs = GetPluginCount();
	for (int PlugIdx = SPI_FIRST; PlugIdx < plugs; PlugIdx++) {
		int	SlotIdx = PlugIdx >= 0 ? GetPlugin(PlugIdx).GetSlotIdx() : PlugIdx;
		int	parms = GetMidiParmCount(SlotIdx);
		int	PageType = GetMidiPageType(SlotIdx);
		int	props = max(GetMidiPropCount(PageType), 1);
		for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
			for (int PropIdx = 0; PropIdx < props; PropIdx++)
				SetMidiInfo(SlotIdx, ParmIdx, PropIdx, def);
		}
	}
}

bool CMidiEngine::GetMidiInfo(int SlotIdx, int ParmIdx, int PropIdx, CMidiInfo& Info) const
{
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		if (!IsLoaded(SlotIdx))
			return(FALSE);
		GetSlot(SlotIdx)->GetParmMidiInfo(ParmIdx, PropIdx, Info);
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			if (!IsLoaded(ParmIdx))
				return(FALSE);
			GetSlot(ParmIdx)->GetMidiInfo(PropIdx, Info);
			break;
		case SPI_MISC:	// misc page: parm indexes misc property
			ASSERT(ParmIdx >= 0 && ParmIdx < MISC_MIDI_PROPS);
			Info = m_MiscMidiProp[ParmIdx];
			break;
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			Info = m_Metaplugin.m_Metaparm[ParmIdx].m_MidiInfo;
			break;
		default:
			return(FALSE);
		}
	}
	return(TRUE);
}

bool CMidiEngine::SetMidiInfo(int SlotIdx, int ParmIdx, int PropIdx, const CMidiInfo& Info)
{
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		if (!IsLoaded(SlotIdx))
			return(FALSE);
		GetSlot(SlotIdx)->SetParmMidiInfo(ParmIdx, PropIdx, Info);
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			if (!IsLoaded(ParmIdx))
				return(FALSE);
			GetSlot(ParmIdx)->SetMidiInfo(PropIdx, Info);
			break;
		case SPI_MISC:	// misc page: parm indexes misc property
			ASSERT(ParmIdx >= 0 && ParmIdx < MISC_MIDI_PROPS);
			m_MiscMidiProp[ParmIdx] = Info;
			break;
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			m_Metaplugin.m_Metaparm[ParmIdx].m_MidiInfo = Info;
			break;
		default:
			return(FALSE);
		}
	}
	return(TRUE);
}

bool CMidiEngine::GetMidiPlugName(int SlotIdx, CString& Name) const
{
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		if (!IsLoaded(SlotIdx))
			return(FALSE);
		Name = GetSlot(SlotIdx)->GetName();
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			Name.LoadString(IDS_MS_TAB_PLUGIN);
			break;
		case SPI_MISC:	// misc page: parm indexes misc property
			Name.LoadString(IDS_MS_TAB_MISC);
			break;
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			Name.LoadString(IDS_MS_TAB_METAPARM);
			break;
		default:
			return(FALSE);
		}
	}
	return(TRUE);
}

bool CMidiEngine::GetMidiParmName(int SlotIdx, int ParmIdx, CString& Name) const
{
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		if (!IsLoaded(SlotIdx))
			return(FALSE);
		Name = GetSlot(SlotIdx)->GetParmName(ParmIdx);
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			if (!IsLoaded(ParmIdx))
				return(FALSE);
			Name = GetSlot(ParmIdx)->GetName();
			break;
		case SPI_MISC:	// misc page: parm indexes misc property
			ASSERT(ParmIdx >= 0 && ParmIdx < MISC_MIDI_PROPS);
			Name.LoadString(m_MiscMidiPropName[ParmIdx]);
			break;
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			Name = m_Metaplugin.m_Metaparm[ParmIdx].m_Name;
			break;
		default:
			return(FALSE);
		}
	}
	return(TRUE);
}

int CMidiEngine::GetMidiParmCount(int SlotIdx) const
{
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		if (IsLoaded(SlotIdx))
			return(GetSlot(SlotIdx)->GetParmCount());
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			return(GetSlotCount());
		case SPI_MISC:	// misc page: parm indexes misc property
			return(MISC_MIDI_PROPS);
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			return(m_Metaplugin.m_Metaparm.GetSize());
		}
	}
	return(0);
}

int CMidiEngine::GetMidiPropCount(int PageType) const
{
	switch (PageType) {
	case MPT_PARAM:
		return(PARAM_MIDI_PROPS);
	case MPT_PLUGIN:
		return(PLUGIN_MIDI_PROPS);
	}
	return(0);
}

void CMidiEngine::GetMidiPropName(int PageType, int PropIdx, CString& Name) const
{
	switch (PageType) {
	case MPT_PARAM:
		Name.LoadString(m_ParamMidiPropName[PropIdx]);
		break;
	case MPT_PLUGIN:
		Name.LoadString(m_PluginMidiPropName[PropIdx]);
		break;
	default:
		Name.Empty();
	}
}

bool CMidiEngine::GetMidiMapping(const CMidiInfo& Info, MIDI_TARGET& Target) const
{
	switch (Info.m_Event) {
	case MET_CTRL:
		Target = m_MidiMap.Ctrl[Info.m_Chan][Info.m_Ctrl];
		break;
	case MET_NOTE:
		Target = m_MidiMap.Note[Info.m_Chan][Info.m_Ctrl];
		break;
	case MET_PITCH:
		Target = m_MidiMap.Pitch[Info.m_Chan];
		break;
	default:
		return(FALSE);
	}
	return(Target.dw != -1);
}

bool CMidiEngine::GetMidiMapping(const CMidiInfo& Info, int& SlotIdx, int& ParmIdx, int& PropIdx) const
{
	MIDI_TARGET	mt;
	if (!GetMidiMapping(Info, mt))
		return(FALSE);
	SlotIdx = mt.s.SlotIdx;
	ParmIdx = mt.s.ParmIdx;
	PropIdx = mt.s.PropIdx;
	return(TRUE);
}

void CMidiEngine::SetMidiMapping(int SlotIdx, int ParmIdx, int PropIdx, const CMidiInfo& Info)
{
	MIDI_TARGET	mt;
	mt.s.SlotIdx = static_cast<short>(SlotIdx);	// limited to 32K slots
	mt.s.ParmIdx = static_cast<BYTE>(ParmIdx);	// limited to 256 parameters
	mt.s.PropIdx = static_cast<BYTE>(PropIdx);	// limited to 256 properties
	switch (Info.m_Event) {
	case MET_CTRL:
		m_MidiMap.Ctrl[Info.m_Chan][Info.m_Ctrl] = mt;
		break;
	case MET_NOTE:
		m_MidiMap.Note[Info.m_Chan][Info.m_Ctrl] = mt;
		break;
	case MET_PITCH:
		m_MidiMap.Pitch[Info.m_Chan] = mt;
		break;
	}
}

void CMidiEngine::ResetMidiMap()
{
	memset(&m_MidiMap, -1, sizeof(m_MidiMap));
}

void CMidiEngine::MakeMidiMap()
{
	ResetMidiMap();
	CMidiInfo	Info;
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		int	SlotIdx = GetPlugin(PlugIdx).GetSlotIdx();
		ASSERT(IsLoaded(SlotIdx));
		int	parms = GetSlot(SlotIdx)->GetParmCount();
		for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
			for (int PropIdx = 0; PropIdx < PARAM_MIDI_PROPS; PropIdx++) {
				GetSlot(SlotIdx)->GetParmMidiInfo(ParmIdx, PropIdx, Info);
				if (Info.m_Event != MET_OFF)
					SetMidiMapping(SlotIdx, ParmIdx, PropIdx, Info);
			}
		}
		for (int PropIdx = 0; PropIdx < PLUGIN_MIDI_PROPS; PropIdx++) {
			GetSlot(SlotIdx)->GetMidiInfo(PropIdx, Info);
			if (Info.m_Event != MET_OFF)
				SetMidiMapping(SPI_PLUGIN, SlotIdx, PropIdx, Info);
		}
	}
	for (int PropIdx = 0; PropIdx < MISC_MIDI_PROPS; PropIdx++) {
		if (m_MiscMidiProp[PropIdx].m_Event != MET_OFF)
			SetMidiMapping(SPI_MISC, PropIdx, 0, m_MiscMidiProp[PropIdx]);
	}
	int	Metaparms = m_Metaplugin.m_Metaparm.GetSize();
	for (int MetaIdx = 0; MetaIdx < Metaparms; MetaIdx++) {
		CMidiInfo	&MidiInfo = m_Metaplugin.m_Metaparm[MetaIdx].m_MidiInfo;
		if (MidiInfo.m_Event != MET_OFF)
			SetMidiMapping(SPI_METAPARM, MetaIdx, 0, MidiInfo);
	}
#ifdef MIDI_NATTER
	DumpMidi();
#endif
}

void CMidiEngine::AssignMidi(int SlotIdx, int ParmIdx, int PropIdx, const CMidiInfo& Info)
{
	if (Info.m_Event != MET_OFF) {
		MIDI_TARGET	Prev;
		if (GetMidiMapping(Info, Prev)) {	// if message is already mapped
			MIDI_TARGET	New;
			New.s.SlotIdx = static_cast<short>(SlotIdx);	// limited to 32K slots
			New.s.ParmIdx = static_cast<BYTE>(ParmIdx);	// limited to 256 parameters
			New.s.PropIdx = static_cast<BYTE>(PropIdx);	// limited to 256 properties
			if (New.dw != Prev.dw) {	// if it's mapped to a different target
				CMidiInfo	PrevInfo;
				GetMidiInfo(Prev.s.SlotIdx, Prev.s.ParmIdx, Prev.s.PropIdx, PrevInfo);
				PrevInfo.m_Event = MET_OFF;	// remove previous mapping
				SetMidiInfo(Prev.s.SlotIdx, Prev.s.ParmIdx, Prev.s.PropIdx, PrevInfo);
			}
		}
	}
	SetMidiInfo(SlotIdx, ParmIdx, PropIdx, Info);
	MakeMidiMap();
}

void CMidiEngine::GetMidiAssignments(CMidiAssignList& AssList) const
{
	CMidiAssign	ma;
	int	plugs = GetPluginCount();
	for (int PlugIdx = SPI_FIRST; PlugIdx < plugs; PlugIdx++) {
		int	SlotIdx = PlugIdx >= 0 ? GetPlugin(PlugIdx).GetSlotIdx() : PlugIdx;
		int	parms = GetMidiParmCount(SlotIdx);
		int	PageType = GetMidiPageType(SlotIdx);
		int	props = max(GetMidiPropCount(PageType), 1);
		for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
			for (int PropIdx = 0; PropIdx < props; PropIdx++) {
				if (GetMidiInfo(SlotIdx, ParmIdx, PropIdx, ma) && !ma.IsDefault()) {
					ma.m_SlotIdx = SlotIdx;
					ma.m_ParmIdx = ParmIdx;
					ma.m_PropIdx = PropIdx;
					AssList.Add(ma);
				}
			}
		}
	}
}

void CMidiEngine::SetMidiAssignments(const CMidiAssign *AssList, int AssCount)
{
	SetMidiDefaults();
	for (int i = 0; i < AssCount; i++) {
		const CMidiAssign&	ma = AssList[i];
		if (ma.m_SlotIdx >= 0) {	// parameter page: parm indexes parameter
			if (!IsLoaded(ma.m_SlotIdx))	// verify slot is loaded
				continue;
		} else {
			switch (ma.m_SlotIdx) {
			case SPI_PLUGIN:	// plugin page: parm indexes plugin
				if (!IsLoaded(ma.m_ParmIdx))	// verify slot is loaded
					continue;
				break;
			}
		}
		SetMidiInfo(ma.m_SlotIdx, ma.m_ParmIdx, ma.m_PropIdx, ma);
	}
	MakeMidiMap();
}

void CMidiEngine::SetMidiAssignments(const CMidiAssignList& AssList)
{
	SetMidiAssignments(AssList.GetData(), AssList.GetSize());
}

void CMidiEngine::DumpMidi()
{
	CMidiAssignList	malist;
	GetMidiAssignments(malist);
	_tprintf(_T("DumpMidi: %d assignments\n"), malist.GetSize());
	for (int i = 0; i < malist.GetSize(); i++) {
		CMidiAssign&	ma = malist[i];
		CString	PlugName, ParmName, PropName;
		GetMidiPlugName(ma.m_SlotIdx, PlugName);
		GetMidiParmName(ma.m_SlotIdx, ma.m_ParmIdx, ParmName);
		int	PageType = GetMidiPageType(ma.m_SlotIdx);
		GetMidiPropName(PageType, ma.m_PropIdx, PropName);
		_tprintf(_T("[%s][%s][%s]: %g %d %d %d\n"), PlugName, ParmName, PropName,
			ma.m_Range, ma.m_Event, ma.m_Chan, ma.m_Ctrl);
	}
}

void CMidiEngine::SetMidiProperty(int Event, MIDI_MSG Msg)
{
	int		SlotIdx, ParmIdx, PropIdx;
	float	Val;
	bool	Toggle;
	if (Event >= 0) {	// if normal MIDI event
		CMidiInfo	Info(0, Event, Msg.s.cmd & 0x0f, Msg.s.p1);
		MIDI_TARGET	mt;
		if (!GetMidiMapping(Info, mt))
			return;
		SlotIdx = mt.s.SlotIdx;
		ParmIdx = mt.s.ParmIdx;
		PropIdx = mt.s.PropIdx;
		GetMidiInfo(SlotIdx, ParmIdx, PropIdx, Info);	// get range
		Val = Msg.s.p2 / 127.0f * Info.m_Range;	// normalize value
		if (Info.m_Range < 0)	// if negative range
			Val = -Info.m_Range + Val;	// invert controller
		Toggle = Event == MET_NOTE;
	} else {	// metaparameter hack
		CMetaparm	*mp = (CMetaparm *)Msg.dw;	// cast Msg to metaparameter pointer
		SlotIdx = mp->m_Target.SlotIdx;
		ParmIdx = mp->m_Target.ParmIdx;
		PropIdx = mp->m_Target.PropIdx;
		float	Delta = mp->m_RangeEnd - mp->m_RangeStart;
		Val = mp->m_RangeStart + mp->m_Value * Delta;
		Toggle = FALSE;
	}
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		CFFPluginEx	*pPlug = GetSlot(SlotIdx);
		CFFPlugsRow	*pRow;
		if (SlotIdx == GetCurSel()) {
			CMainFrame	*pMain = theApp.GetMain();
			CFFRendView	*pView = pMain->GetView();
			pRow = pView->GetRow(ParmIdx);
		} else
			pRow = NULL;
		switch (PropIdx) {
		case MP_PARAM:
			if (Toggle)
				Val = pPlug->GetParmVal(ParmIdx) < .5;
			Val = CLAMP(Val, 0, 1);
			pPlug->SetParmVal(ParmIdx, Val);
			if (pRow != NULL)
				pRow->SetVal(Val);
			break;
		case MP_MOD_ENAB:
			if (Toggle)
				Val = !pPlug->GetModEnable(ParmIdx);
			pPlug->SetModEnable(ParmIdx, Val >= .5);
			if (pRow != NULL)
				pRow->SetModEnable(Val >= .5);
			break;
		case MP_MOD_WAVE:
			if (Toggle)
				Val = pPlug->GetModWaveNorm(ParmIdx) < .5;
			Val = CLAMP(Val, 0, 1);
			{
				int Wave = CFFParm::DenormModWave(Val);
				pPlug->SetModWave(ParmIdx, Wave);
				if (pRow != NULL)
					pRow->SetModWave(Wave);
			}
			break;
		case MP_MOD_FREQ:
			if (Toggle)
				Val = pPlug->GetModFreq(ParmIdx) < .5;
			pPlug->SetModFreq(ParmIdx, Val);
			if (pRow != NULL)
				pRow->SetModFreq(Val);
			break;
		case MP_MOD_PW:
			if (Toggle)
				Val = pPlug->GetModPulseWidth(ParmIdx) < .5;
			Val = CLAMP(Val, 0, 1);
			pPlug->SetModPulseWidth(ParmIdx, Val);
			if (pRow != NULL)
				pRow->SetModPulseWidth(Val);
			break;
		default:
			NODEFAULTCASE;
		}
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			switch (PropIdx) {
			case MP_BYPASS:
				if (Toggle)
					Val = !GetSlot(ParmIdx)->GetBypass();
				Bypass(ParmIdx, Val >= .5);
				break;
			default:
				NODEFAULTCASE;
			}
			break;
		case SPI_MISC:	// misc page: parm indexes misc property
			switch (ParmIdx) {
			case MP_MASTER_SPEED:
				if (Toggle)
					Val = GetSpeedNorm() < .5;
				SetSpeedNorm(Val);
				break;
			default:
				NODEFAULTCASE;
			}
			break;
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			if (Toggle)
				Val = m_Metaplugin.m_Metaparm[ParmIdx].m_Value < .5;
			theApp.GetMain()->GetMetaparmBar().SetValue(ParmIdx, Val);
			break;
		}
	}
}

float CMidiEngine::GetMidiProperty(int SlotIdx, int ParmIdx, int PropIdx) const
{
	if (SlotIdx >= 0) {	// parameter page: parm indexes parameter
		const CFFPluginEx	*pPlug = GetSlot(SlotIdx);
		switch (PropIdx) {
		case MP_PARAM:
			return(pPlug->GetParmVal(ParmIdx));
		case MP_MOD_ENAB:
			return(pPlug->GetModEnable(ParmIdx));
		case MP_MOD_WAVE:
			return(pPlug->GetModWaveNorm(ParmIdx));
		case MP_MOD_FREQ:
			return(pPlug->GetModFreq(ParmIdx));
		case MP_MOD_PW:
			return(pPlug->GetModPulseWidth(ParmIdx));
		default:
			NODEFAULTCASE;
		}
	} else {
		switch (SlotIdx) {
		case SPI_PLUGIN:	// plugin page: parm indexes plugin
			switch (PropIdx) {
			case MP_BYPASS:
				return(GetSlot(ParmIdx)->GetBypass());
			default:
				NODEFAULTCASE;
			}
			break;
		case SPI_MISC:	// misc page: parm indexes misc property
			switch (ParmIdx) {
			case MP_MASTER_SPEED:
				return(GetSpeedNorm());
			default:
				NODEFAULTCASE;
			}
			break;
		case SPI_METAPARM:	// metaparameter page: parm indexes metaparameter
			return(m_Metaplugin.m_Metaparm[ParmIdx].m_Value);
		}
	}
	return(0);
}

void CMidiEngine::MakeTarget(int PageType, int SlotIdx, int ParmIdx, int PropIdx, METATARGET& Target)
{
	// see CMidiAssign comment in MidiInfo.h
	switch (PageType) {
	case MPT_PARAM:		// 3D: target is a plugin parameter property 
		Target.SlotIdx = SlotIdx;
		Target.ParmIdx = ParmIdx;
		Target.PropIdx = PropIdx;
		break;
	case MPT_PLUGIN:	// 2D: target is a plugin property
		Target.SlotIdx = SPI_PLUGIN;
		Target.ParmIdx = SlotIdx;
		Target.PropIdx = PropIdx;
		break;
	case MPT_MISC:		// 1D: target is a miscellaneous property
		Target.SlotIdx = SPI_MISC;
		Target.ParmIdx = PropIdx;
		Target.PropIdx = 0;
		break;
	case MPT_METAPARM:	// 1D: target is a metaparameter
		Target.SlotIdx = SPI_METAPARM;
		Target.ParmIdx = PropIdx;
		Target.PropIdx = 0;
		break;
	default:
		NODEFAULTCASE;
	}
}

int CMidiEngine::RemoveMidiDups(int SlotIdx, const CMidiAssignList& PrevAss)
{
	// if any of the specified plugin's MIDI messages are found in the
	// previous assignment list, remove them from the specified plugin
	CMidiAssignList	CurAss;
	GetMidiAssignments(CurAss);	// get current assignments
	CMidiInfo	def;
	int	Dups = 0;
	for (int i = 0; i < CurAss.GetSize(); i++) {
		CMidiAssign&	ma = CurAss[i];
		if (ma.m_SlotIdx == SlotIdx	// if assignment is specified plugin's
		|| (ma.m_SlotIdx == SPI_PLUGIN && ma.m_ParmIdx == SlotIdx)) {
			for (int j = 0; j < PrevAss.GetSize(); j++) {
				// if MIDI message matches (ignore range)
				if (ma.m_Event == PrevAss[j].m_Event
				&& ma.m_Chan == PrevAss[j].m_Chan
				&& ma.m_Ctrl == PrevAss[j].m_Ctrl) {
					// remove duplicate MIDI assignment from specified plugin
					SetMidiInfo(ma.m_SlotIdx, ma.m_ParmIdx, ma.m_PropIdx, def);
					Dups++;
				}
			}
		}
	}
	return(Dups);	// number of duplicates removed
}

void CMidiEngine::GetProject(CFFProject& Project)
{
	CFFEngine::GetProject(Project);	// call base class
	GetMidiAssignments(Project.m_MidiAssignList);
	Project.m_Metaplugin = m_Metaplugin;
}

bool CMidiEngine::SetProject(const CFFProject& Project)
{
	CExtendSlotChange	extend(*this);	// extend slot change to include our method
	bool	retc = CFFEngine::SetProject(Project);	// call base class
	m_Metaplugin = Project.m_Metaplugin;
	UpdateMetaparmLinks();	// order matters; metaparms may have MIDI assignments
	SetMidiAssignments(Project.m_MidiAssignList);
	return(retc);
}

void CMidiEngine::OnEndSlotChange()
{
	CFFEngine::OnEndSlotChange();	// must call base class
	MakeMidiMap();	// rebuild MIDI map
	if (m_Metaplugin.m_Metaparm.GetSize())	// if metaparms exist
		RebuildMetaparmArray();	// rebuild metaparm array
}

float CMidiEngine::GetSpeedNorm() const
{
	CMasterDlg&	dlg = theApp.GetMain()->GetMasterBar().GetDlg();
	return(static_cast<float>(dlg.GetSpeedNorm()));
}

void CMidiEngine::SetSpeedNorm(float Speed)
{
	CMasterDlg&	dlg = theApp.GetMain()->GetMasterBar().GetDlg();
	dlg.SetSpeedNorm(Speed);
	SetSpeed(static_cast<float>(dlg.GetSpeed()));
}

bool CMidiEngine::UpdateMetaparmLinks()
{
	int	BrokenLinks = 0;
	int	parms = m_Metaplugin.m_Metaparm.GetSize();
	for (int i = 0; i < parms; i++) {
		CMetaparm	&parm = m_Metaplugin.m_Metaparm[i];
		if (parm.IsAssigned()) {	// if metaparameter has a target
			int	SlotIdx = parm.GetTargetPlugin();
			// if target is associated with a valid plugin slot
			if (SlotIdx >= 0 && SlotIdx <= GetSlotCount() && IsLoaded(SlotIdx))
				parm.m_PluginUID = GetSlot(SlotIdx)->GetUID();	// copy plugin's UID
			else
				BrokenLinks++;
		}
	}
	return(!BrokenLinks);
}

void CMidiEngine::RebuildMetaparmArray()
{
	CMap<UINT, UINT, int, int>	PlugHash;	// map plugin UIDs to plugin indices
	int	slots = GetSlotCount();
	for (int SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
		const CFFPluginEx	*slot = GetSlot(SlotIdx);
		if (slot != NULL)	// ignore empty slots
			PlugHash.SetAt(slot->GetUID(), SlotIdx);	// add plugin to hash
	}
	int	i = 0;
	while (i < m_Metaplugin.m_Metaparm.GetSize()) {	// don't cache size; we're deleting elements
		CMetaparm&	mp = m_Metaplugin.m_Metaparm[i];
		// if metaparameter has a target, and target is associated with a plugin
		if (mp.IsAssigned() && mp.GetTargetPlugin() >= 0) {
			int	SlotIdx;
			if (PlugHash.Lookup(mp.m_PluginUID, SlotIdx)) {	// if plugin found in hash
				mp.SetTargetPlugin(SlotIdx);	// update metaparameter's target plugin
				i++;	// next metaparameter
			} else	// plugin not found; delete orphaned metaparameter
				m_Metaplugin.m_Metaparm.RemoveAt(i);	// decrements size, so don't increment loop var
		} else
			i++;	// next metaparameter
	}
#ifdef METAGROUP_NATTER
	m_Metaplugin.m_Metaparm.DumpGroups();
#endif
}

void CMidiEngine::ApplyMetaparm(CMetaparm& Metaparm)
{
	MIDI_MSG	msg;
	ASSERT(sizeof(LPVOID) == sizeof(msg.dw));	// casting pointer to DWORD
	msg.dw = (DWORD)&Metaparm;	// cast metaparameter pointer to MIDI message
	SetMidiProperty(-1, msg);	// negative Event signals metaparameter hack
}
