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
        02      23apr10	make GUID callback Unicode-compatible

        enumerate monitors
 
*/

#pragma once

class CMonitorInfo : public WObject {
public:
	static	HMONITOR	GetFullScreenRect(HWND hWnd, CRect& rc);
	static	bool	GetMonitorGUID(HMONITOR hMon, GUID& MonGuid);

private:
	typedef struct tagMONGUID {
		HMONITOR	Mon;
		GUID		Guid;
		BOOL		Valid;
	} MONGUID;
	static	BOOL WINAPI GetGUIDCallback(GUID FAR *lpGUID, LPTSTR lpDriverDescription,
		LPTSTR lpDriverName, LPVOID lpContext, HMONITOR hm);
};
