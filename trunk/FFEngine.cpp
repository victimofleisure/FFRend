// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		27apr10	add MIDI
		02		31mar11	in SetFrameProps, set members before updating plugins
		03		01may11	add get/set load balance
		04		07may11	add sync oscillators
		05		18nov11	remove get/set load balance
		06		01dec11	make run while loading optional
		07		05jan12	override Run to rewind oscillators on stop
		08		11jan12	in GetProject, get synchronized modulations
		09		13apr12	add FindPluginByUID
		10		06may12	make FindPluginByUID const, fix Load assert
		11		22may12	in GetSyncMods, access plugin info via slot index

		parallel FreeFrame plugin engine
 
*/

#include "stdafx.h"
#include "FFEngine.h"
#include "FFPluginEx.h"
#include "Renderer.h"
#include "Benchmark.h"
#include "MapEx.h"

CFFEngine::CFFEngine(CRenderer& Renderer) :
	CEngine(Renderer)
{
	m_FrameSize = CSize(0, 0);
	m_ColorDepth = 0;
	m_FFColorDepth = 0;
	m_CurSel = -1;
	m_Speed = 1;
	m_FrameRate = 0;
	m_CurUID = 0;
	m_PendingAcks = 0;
	m_InSolo = FALSE;
	m_OpeningClip = FALSE;
	m_RandUseTime = FALSE;
	m_RunWhileLoading = TRUE;
	m_RandSeed = 1;
}

CFFEngine::~CFFEngine()
{
}

bool CFFEngine::Create()
{
	if (!CEngine::Create())
		return(FALSE);
	if (!m_ModalDone.Create(NULL, TRUE, FALSE, NULL)) {	// manual reset
		OnError(ENGERR_CANT_CREATE_EVENT);
		return(FALSE);
	}
	return(TRUE);
}

void CFFEngine::Bypass(int SlotIdx, bool Enable)
{
	ASSERT(IsValidSlot(SlotIdx) && IsLoaded(SlotIdx));
	GetSlot(SlotIdx)->SetBypass(Enable);
}

void CFFEngine::SetCurSel(int SlotIdx)
{
	ASSERT(SlotIdx < GetSlotCount());
	if (SlotIdx == m_CurSel)
		return;
	m_CurSel = SlotIdx;
}

void CFFEngine::OnEndSlotChange()
{
	CEngine::OnEndSlotChange();	// must call base class
	int	slots = GetSlotCount();
	if (m_CurSel >= slots)
		m_CurSel = slots - 1;
	// if multiple instances of same plugin, decorate names to differentiate them
	typedef CMapEx<CString, LPCTSTR, NAME_DEF, NAME_DEF&> CNameDefMap;
	CNameDefMap	map;
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plug = GetPlugin(PlugIdx);
		CString	FFName(plug.GetFFName());
		CNameDefMap::CPair	*pair;
		pair = map.PLookup(FFName);
		if (pair != NULL) {	// if name already used
			// if second usage, decorate original usage
			if (pair->value.RefCount == 1)
				GetPlugin(pair->value.FirstIdx).SetName(FFName + _T("-1"));
			pair->value.RefCount++;
			CString	DecoName;
			DecoName.Format(_T("%s-%d"), FFName, pair->value.RefCount);
			plug.SetName(DecoName);	// decorate this usage
		} else {	// name not used yet
			NAME_DEF	info;
			info.RefCount = 1;
			info.FirstIdx = PlugIdx;
			map.SetAt(FFName, info);
			plug.SetName(FFName);
		}
	}
}

bool CFFEngine::SetFrameProps(CSize FrameSize, UINT ColorDepth)
{
	if (FrameSize == m_FrameSize && ColorDepth == m_ColorDepth)
		return(TRUE);
	UINT	FFColorDepth;	
	switch (ColorDepth) {
	case 16:
		FFColorDepth = FF_CAP_16BITVIDEO;
		break;
	case 24:
		FFColorDepth = FF_CAP_24BITVIDEO;
		break;
	case 32:
		FFColorDepth = FF_CAP_32BITVIDEO;
		break;
	default:
		return(FALSE);	// invalid color depth
	}
	STOP_ENGINE(*this);
	m_FrameSize = FrameSize;	// set members first; plugin helpers use these
	m_ColorDepth = ColorDepth;
	m_FFColorDepth = FFColorDepth;
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		if (!GetPlugin(PlugIdx).SetFrameProps(FrameSize, FFColorDepth))
			return(FALSE);
	}
	if (!m_Renderer.SetFrameProps(FrameSize, ColorDepth))
		return(FALSE);
	if (!SetFrameLength(FrameSize.cx * FrameSize.cy * (ColorDepth >> 3)))
		return(FALSE);
	return(TRUE);
}

bool CFFEngine::SetFrameSize(CSize FrameSize)
{
	return(SetFrameProps(FrameSize, m_ColorDepth));
}

bool CFFEngine::SetColorDepth(UINT ColorDepth)
{
	return(SetFrameProps(m_FrameSize, ColorDepth));
}

void CFFEngine::SetFrameRate(float Freq)
{
	if (Freq == m_FrameRate)
		return;
	m_FrameRate = Freq;
	m_Renderer.SetFramePeriod(round(1000.0f / Freq));
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++)
		GetPlugin(PlugIdx).UpdateFrameRate();
}

CFFPluginEx *CFFEngine::CreatePlugin(LPCTSTR Path)
{
	CFFPluginEx	*plug = new CFFPluginEx;
	// create freeframe instance only, without launching worker thread
	if (!plug->CreateFF(*this, Path)) {
		delete plug;	// clean up
		return(NULL);
	}
	return(plug);
}

bool CFFEngine::Insert(int SlotIdx, LPCTSTR Path)
{
	ASSERT(SlotIdx >= 0 && SlotIdx <= GetSlotCount());
	CExtendSlotChange	extend(*this);	// extend change to include freeframe init
	// create freeframe instance before actual slot change, to minimize downtime
	CFFPluginEx	*plug = CreatePlugin(Path);
	if (plug == NULL)
		return(FALSE);
	CSlotPtr	slot(plug);	// slot dtor deletes plugin if we abort
	SLOT_CHANGE(*this);		// begin slot change, if not already in one
	if (!plug->Launch())	// launch plugin's worker thread
		return(FALSE);
	m_CurSel = SlotIdx;
	return(CEngine::Insert(SlotIdx, slot));
}

bool CFFEngine::Insert(int SlotIdx, const CFFPlugInfo& Info)
{
	ASSERT(SlotIdx >= 0 && SlotIdx <= GetSlotCount());
	// extend slot change so SetInfo can access plugin directly instead of queueing
	CExtendSlotChange	extend(*this);
	if (!Info.m_ClipPath.IsEmpty()) {	// if clip path not empty
		if (!InsertPlayer(SlotIdx, Info.m_Path))	// insert clip player
			return(FALSE);
	} else {
		if (!Insert(SlotIdx, Info.m_Path))	// insert normal plugin
			return(FALSE);
	}
	GetSlot(SlotIdx)->SetInfo(Info);
	return(TRUE);
}

bool CFFEngine::Load(int SlotIdx, const CFFPlugInfo& Info)
{
	ASSERT(IsValidSlot(SlotIdx));
	// extend slot change so SetInfo can access plugin directly instead of queueing
	CExtendSlotChange	extend(*this);
	if (!Info.m_ClipPath.IsEmpty()) {	// if clip path not empty
		if (!LoadPlayer(SlotIdx, Info.m_Path))	// insert clip player
			return(FALSE);
	} else {
		if (!Load(SlotIdx, Info.m_Path))	// insert normal plugin
			return(FALSE);
	}
	GetSlot(SlotIdx)->SetInfo(Info);
	return(TRUE);
}

bool CFFEngine::InsertEmpty(int SlotIdx)
{
	ASSERT(SlotIdx >= 0 && SlotIdx <= GetSlotCount());
	m_CurSel = SlotIdx;
	return(CEngine::Insert(SlotIdx, NULL));
}

bool CFFEngine::Delete(int SlotIdx)
{
	ASSERT(IsValidSlot(SlotIdx));
	m_CurSel = SlotIdx;
	return(CEngine::Delete(SlotIdx));
}

bool CFFEngine::Load(int SlotIdx, LPCTSTR Path)
{
	ASSERT(IsValidSlot(SlotIdx));
	CExtendSlotChange	extend(*this);	// extend change to include freeframe init
	// create freeframe instance before actual slot change, to minimize downtime
	CFFPluginEx	*plug = CreatePlugin(Path);
	if (plug == NULL)
		return(FALSE);
	CSlotPtr	slot(plug);	// slot dtor deletes plugin if we abort
	SLOT_CHANGE(*this);		// begin slot change, if not already in one
	if (!plug->Launch())	// launch plugin's worker thread
		return(FALSE);
	m_CurSel = SlotIdx;
	return(CEngine::Load(SlotIdx, slot));
}

bool CFFEngine::Unload(int SlotIdx)
{
	ASSERT(IsValidSlot(SlotIdx));
	m_CurSel = SlotIdx;
	return(CEngine::Load(SlotIdx, NULL));
}

bool CFFEngine::Move(int Src, int Dst)
{
	ASSERT(Src >= 0 && Src < GetSlotCount());
	ASSERT(Dst >= 0 && Dst <= GetSlotCount());
	m_CurSel = Dst;
	return(CEngine::Move(Src, Dst));
}

void CFFEngine::GetRouting(int SlotIdx, CRouting& Routing) const
{
	Routing.RemoveAll();
	for (int DstSlot = 0; DstSlot < GetSlotCount(); DstSlot++) {
		if (IsLoaded(DstSlot)) {
			const CPlugin	*plug = m_Slot[DstSlot];
			int	Inputs = plug->GetNumInputs();
			for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {
				int	SrcSlot = plug->GetInputSlot(InpIdx);
				if (SrcSlot >= 0) {	// if explicit input
					// if source or target matches argument, or wildcard
					if (SrcSlot == SlotIdx || DstSlot == SlotIdx || SlotIdx < 0) {
						ROUTE	rt;
						rt.SrcSlot = SrcSlot;
						rt.DstSlot = DstSlot;
						rt.InpIdx = InpIdx;
						Routing.Add(rt);
					}
				}
			}
		}
	}
}

void CFFEngine::SetRouting(const CRouting& Routing)
{
	int	routes = Routing.GetSize();
	for (int ri = 0; ri < routes; ri++) {
		const ROUTE& rt = Routing[ri];
		// if destination is loaded, and source is default or loaded,
		// and input index is within destination plugin's input count
		if (IsLoaded(rt.DstSlot) && (rt.SrcSlot < 0 || IsLoaded(rt.SrcSlot))
		&& rt.InpIdx < m_Slot[rt.DstSlot]->GetNumInputs())
			m_Slot[rt.DstSlot]->SetInputSlot(rt.InpIdx, rt.SrcSlot);	// connect
	}
}

bool CFFEngine::GetSyncMods(UINT FrameCount, CFFProject::CFFPlugInfoArray& Info)
{
	int	plugs = GetPluginCount();
	if (!plugs)	// if no plugins
		return(TRUE);	// nothing to do
	m_PendingAcks = plugs;	// all plugins must acknowledge broadcast
	m_ModalDone.Reset();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plug = GetPlugin(PlugIdx);
		int	SlotIdx = plug.GetSlotIdx();	// plugin info uses slot indices
		plug.GetSyncMods(FrameCount, Info[SlotIdx].m_Parm);
	}
	return(WaitForModalDone());	// last plugin to acknowledge broadcast sets done
}

void CFFEngine::GetProject(CFFProject& Project)
{
	int	slots = GetSlotCount();
	Project.m_PlugInfo.SetSize(slots);
	for (int SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
		CFFPlugInfo&	plug = Project.m_PlugInfo[SlotIdx];
		CFFPluginEx	*slot = GetSlot(SlotIdx);
		if (slot != NULL)
			slot->GetInfo(plug);
	}
	UINT	FrameCount = m_Renderer.GetFrameCounter();
	GetSyncMods(FrameCount, Project.m_PlugInfo);
	GetRouting(-1, Project.m_Routing);
	Project.m_CurSel = m_CurSel;
	Project.m_Speed = m_Speed;
}

bool CFFEngine::SetProject(const CFFProject& Project)
{
	CExtendSlotChange	extend(*this);	// extend change to include freeframe init
	int	slots;
	// if we should run while loading project, create freeframe instances before
	// actual slot change; minimizes downtime at cost of increased memory usage
	if (m_RunWhileLoading)
		slots = Project.m_PlugInfo.GetSize();
	else	// stop before loading project; longer downtime but uses less memory
		slots = 0;	// disable early creation of freeframe instances
	CArrayEx<CFFPluginEx *, CFFPluginEx *> 	pa;
	pa.SetSize(slots);
	int SlotIdx;
	for (SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
		const CFFPlugInfo&	info = Project.m_PlugInfo[SlotIdx];
		if (!info.m_Path.IsEmpty()) {
			if (!info.m_ClipPath.IsEmpty())	// if clip path not empty
				pa[SlotIdx] = CreatePlayer(info.m_Path);	// create clip player
			else
				pa[SlotIdx] = CreatePlugin(info.m_Path);	// create normal plugin
		}
	}
	CSlotChange	sc(*this);	// begin actual slot change
	if (!sc) {	// if begin slot change failed
		for (SlotIdx = 0; SlotIdx < slots; SlotIdx++)	// delete freeframe instances
			delete pa[SlotIdx];
		return(FALSE);
	}
	RemoveAll();
	m_Speed = Project.m_Speed;
	bool	retc = TRUE;	// assume success
	if (slots) {	// if freeframe instances already created above
		m_Slot.SetSize(slots);
		for (SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
			const CFFPlugInfo&	info = Project.m_PlugInfo[SlotIdx];
			if (!info.m_Path.IsEmpty()) {
				CFFPluginEx	*plug = pa[SlotIdx];
				CSlotPtr	slot(plug);	// slot dtor deletes plugin if launch fails
				if (plug != NULL && plug->Launch()) {
					plug->SetInfo(info);
					m_Slot[SlotIdx] = slot;
				} else
					retc = FALSE;
			}
		}
	} else {	// freeframe instances not created yet
		slots = Project.m_PlugInfo.GetSize();
		m_Slot.SetSize(slots);
		for (SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
			const CFFPlugInfo&	info = Project.m_PlugInfo[SlotIdx];
			if (!info.m_Path.IsEmpty()) {
				CFFPluginEx	*plug = new CFFPluginEx;
				CSlotPtr	slot(plug);	// slot dtor deletes plugin if launch fails
				if (plug->Create(*this, info.m_Path)) {
					plug->SetInfo(info);
					m_Slot[SlotIdx] = slot;
				} else
					retc = FALSE;
			}
		}
	}	
	SetRouting(Project.m_Routing);
	m_CurSel = Project.m_CurSel;
	m_InSolo = FALSE;
	m_Renderer.SetFrameCounter(0);
	return(retc);
}

void CFFEngine::SetSpeed(float Speed)
{
	m_Speed = Speed;
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++)
		GetPlugin(PlugIdx).UpdateSpeed();
}

void CFFEngine::Solo(int SlotIdx)
{
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plug = GetPlugin(PlugIdx);
		if (!m_InSolo)
			plug.SetSoloBypass(plug.GetBypass());
		Bypass(plug.GetSlotIdx(), plug.GetSlotIdx() != SlotIdx);
	}
	m_InSolo = TRUE;
}

void CFFEngine::EndSolo()
{
	if (m_InSolo) {
		int	plugs = GetPluginCount();
		for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
			CFFPluginEx&	plug = GetPlugin(PlugIdx);
			Bypass(plug.GetSlotIdx(), plug.GetSoloBypass());
		}
		m_InSolo = FALSE;
	}
}

bool CFFEngine::SyncOscillators()
{
	STOP_ENGINE(*this);
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plug = GetPlugin(PlugIdx);
		int	parms = plug.GetParmCount();
		for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
			if (plug.IsModulating(ParmIdx)) {
				CFFPlugInfo::FRANGE	range = plug.GetModRange(ParmIdx);
				plug.SetParmVal(ParmIdx, max(range.Start, 0));	// reset oscillator
			}
		}
	}
	return(TRUE);
}

bool CFFEngine::Run(bool Enable)
{
	if (!CEngine::Run(Enable))	// must call base class
		return(FALSE);
	if (!Enable) {	// if stopping
		int	plugs = GetPluginCount();
		for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
			GetPlugin(PlugIdx).RewindOscillators();
		}
	}
	return(TRUE);
}

int CFFEngine::FindPluginByUID(UINT TargetUID) const
{
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		if (GetPlugin(PlugIdx).GetUID() == TargetUID)
			return(PlugIdx);
	}
	return(-1);
}
