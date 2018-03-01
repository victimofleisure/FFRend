// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		06may10	initial version

		engine with clip player support
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "ClipEngine.h"
#include "FFPluginEx.h"

bool CClipEngine::CreateClipPlayer(int SlotIdx, LPCTSTR Path, UINT Action)
{
	// extend slot change, to prevent clip from running between insert/load and open
	CExtendSlotChange	extend(*this);
	if (Action == CA_INSERT) {
		if (!InsertPlayer(SlotIdx, m_ClipPlayerPath))
			return(FALSE);
	} else {	// load
		if (!LoadPlayer(SlotIdx, m_ClipPlayerPath))
			return(FALSE);
	}
	if (Path == NULL)
		return(TRUE);
	if (!GetSlot(SlotIdx)->OpenClip(Path)) {
		// can't open clip: reverse action
		if (Action == CA_INSERT)
			Delete(SlotIdx);
		else	// load
			Unload(SlotIdx);
		return(FALSE);
	}
	return(TRUE);
}

int CClipEngine::FindFirstClipPlayer() const
{
	int	plugs = GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		const CFFPluginEx&	plug = GetPlugin(PlugIdx);
		if (plug.IsClipPlayer())
			return(plug.GetSlotIdx());
	}
	return(-1);
}

bool CClipEngine::SetProject(const CFFProject& Project)
{
	CExtendSlotChange	extend(*this);	// extend slot change to include our method
	if (!CMidiEngine::SetProject(Project))	// call base class
		return(FALSE);
	if (!Project.m_VideoPath.IsEmpty()) {
		// if first slot contains a null plugin, replace it with a clip player
		int	CurSel = m_CurSel;
		static const UINT FFID_Null = MAKE_FFUNIQUEID('N', 'U', 'L', 'L');
		if (GetSlotCount() && IsLoaded(0) && GetSlot(0)->GetFFUniqueID() == FFID_Null
		&& !_tcscmp(GetSlot(0)->GetName(), _T("Null")))
			LoadClipPlayer(0, Project.m_VideoPath);
		else {
			if (InsertClipPlayer(0, Project.m_VideoPath))	 // insert player in first slot
				CurSel++;	// bump current selection to compensate for insert
		}
		m_CurSel = CurSel;	// load/insert changes current selection, so restore it
	}
	return(TRUE);
}
