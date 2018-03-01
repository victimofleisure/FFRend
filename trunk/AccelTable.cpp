// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04feb05 initial version
        01      15feb09 add copy ctor and array support

		wrapper for accelerator table
 
*/

#include "stdafx.h"
#include "AccelTable.h"

CAccelTable::CAccelTable()
{
	m_hAccel = NULL;
	m_bDestroy = FALSE;
}

CAccelTable::~CAccelTable()
{
	Destroy();
}

void CAccelTable::Destroy()
{
	if (m_hAccel != NULL && m_bDestroy)
		DestroyAcceleratorTable(m_hAccel);
	m_hAccel = NULL;
	m_bDestroy = FALSE;
}

void CAccelTable::Copy(const CAccelTable& Table)
{
	CAccelArray	AccArr;
	Table.GetArray(AccArr);
	LoadFromArray(AccArr);
}

bool CAccelTable::LoadFromRes(int ResID)
{
	Destroy();
	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(ResID));
	m_bDestroy = FALSE;	// tables loaded from resources are freed automatically
	return(m_hAccel != NULL);
}

bool CAccelTable::LoadFromHandle(HACCEL hAccel)
{
	CAccelArray	AccArr;
	GetArray(hAccel, AccArr);
	return(LoadFromArray(AccArr));
}

bool CAccelTable::LoadFromTable(const ACCEL *Table, int Entries)
{
	Destroy();
	if (Table != NULL) {
		m_hAccel = CreateAcceleratorTable(const_cast<ACCEL *>(Table), Entries);
		m_bDestroy = TRUE;	// we're responsible for destroying this table
	}
	return(m_hAccel != NULL);
}

int CAccelTable::GetEntries() const
{
	if (m_hAccel == NULL)
		return(0);
	return(CopyAcceleratorTable(m_hAccel, NULL, 0));
}

int CAccelTable::GetTable(ACCEL *Table, int Entries) const
{
	if (m_hAccel == NULL || Table == NULL)
		return(0);
	return(CopyAcceleratorTable(m_hAccel, Table, Entries));
}

int CAccelTable::GetArray(HACCEL hAccel, CAccelArray& AccArr)
{
	int	sz = CopyAcceleratorTable(hAccel, NULL, 0);
	AccArr.SetSize(sz);
	return(CopyAcceleratorTable(hAccel, AccArr.GetData(), sz));
}

bool CAccelTable::IsExtended(DWORD Key)
{
	switch (Key) {
	case VK_INSERT:
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_NEXT:  // Page Down
	case VK_PRIOR: // Page Up
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		return(TRUE);
	}
	return(FALSE);
}

CString CAccelTable::GetKeyNameText(LONG lParam)
{
	static const int MAX_KEY_NAME = 256;
	CString	s;
	::GetKeyNameText(lParam << 16, s.GetBuffer(MAX_KEY_NAME), MAX_KEY_NAME);
	s.ReleaseBuffer();
	return(s);
}

void CAccelTable::GetKeyName(const ACCEL& Accel, CString& KeyName)
{
	// this code thanks to Jörgen Sigvardsson
	CString	s;
	if (Accel.fVirt & FALT)
		s += GetKeyNameText(MapVirtualKey(VK_MENU, 0));
	if (Accel.fVirt & FCONTROL) {
		if (!s.IsEmpty())
			s += '+';
		s += GetKeyNameText(MapVirtualKey(VK_CONTROL, 0));
	}
	if (Accel.fVirt & FSHIFT) {
		if (!s.IsEmpty())
			s += '+';
		s += GetKeyNameText(MapVirtualKey(VK_SHIFT, 0));
	}
	if (Accel.fVirt & FVIRTKEY) {
		int	scancode = MapVirtualKey(Accel.key, 0);
		if (IsExtended(Accel.key))
			scancode |= 0x100; // add extended bit
		if (!s.IsEmpty())
			s += '+';
		s += GetKeyNameText(scancode);
	} else {	// ASCII key code
		if (!s.IsEmpty())
			s += '+';
		s += (char)Accel.key;
	}
	KeyName = s;
}

void CAccelTable::GetCmdName(const ACCEL& Accel, CString& CmdName)
{
	CString	s((LPCTSTR)Accel.cmd);	// load command's string resource
	int	pos = s.Find('\n');	// status bar hint + newline + tooltip
	CmdName = pos >= 0 ? s.Mid(pos + 1) : s;	// extract tooltip
}

void CAccelTable::GetCmdHelp(const ACCEL& Accel, CString& CmdHelp)
{
	CString	s((LPCTSTR)Accel.cmd);	// load command's string resource
	int	pos = s.Find('\n');	// status bar hint + newline + tooltip
	CmdHelp = pos >= 0 ? s.Left(pos) : s;	// extract status bar hint
}
