// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10may05	initial version
        01      06mar10	separate from BackBufDD
		02		23apr10	make GUID callback Unicode-compatible

        enumerate monitors
 
*/

#include "stdafx.h"
#include "MonitorInfo.h"
#include <ddraw.h>

HMONITOR CMonitorInfo::GetFullScreenRect(HWND hWnd, CRect& rc)
{
	CRect	wr;
	::GetWindowRect(hWnd, wr);
	// try to get screen size from monitor API in case we're dual-monitor
	MONITORINFO	mi;
	mi.cbSize = sizeof(mi);
	HMONITOR	hMon = MonitorFromRect(wr, MONITOR_DEFAULTTONEAREST);
	if (hMon != NULL && GetMonitorInfo(hMon, &mi)) {
		rc = mi.rcMonitor;
	} else {	// fall back to older API
		rc = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN));
	}
	return(hMon);
}

BOOL WINAPI CMonitorInfo::GetGUIDCallback(GUID FAR *lpGUID, LPTSTR lpDriverDescription, 
	LPTSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	MONGUID	*mg = (MONGUID *)lpContext;
	if (hm == mg->Mon) {	// if it's the monitor we're looking for
		mg->Guid = *lpGUID;	// pass its GUID to caller
		mg->Valid = TRUE;	// tell caller we found it
		return(DDENUMRET_CANCEL);	// stop enumerating
	}
	return(DDENUMRET_OK);	// continue enumerating
}

bool CMonitorInfo::GetMonitorGUID(HMONITOR hMon, GUID& MonGuid)
{
	MONGUID	mg;
	ZeroMemory(&mg, sizeof(mg));
	mg.Mon = hMon;	// tell callback which monitor to look for
	HRESULT	hr = DirectDrawEnumerateEx(GetGUIDCallback, &mg, 
		DDENUM_ATTACHEDSECONDARYDEVICES);
	if (FAILED(hr))
		return(FALSE);
	if (!mg.Valid)	// if callback didn't find the monitor
		return(FALSE);
	MonGuid = mg.Guid;	// pass monitor's GUID to caller
	return(TRUE);
}
