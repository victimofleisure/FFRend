// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		24jul06	initial version
		01		23nov07	support Unicode

		wrapper for freeframe plugin DLL

*/

#ifndef CFFPLUGIN_INCLUDED
#define CFFPLUGIN_INCLUDED

#include "FreeFrame.h"

class CFFPlugin : public WObject {
public:
// Construction
	CFFPlugin();
	~CFFPlugin();
	bool	Load(LPCTSTR Path);
	bool	Free();

// Attributes
	bool	IsLoaded() const;
	const	PlugInfoStruct *GetInfo() const;
	bool	GetInfo(PlugInfoStruct& PlugInfo) const;
	bool	GetPluginName(CString& Name) const;
	bool	GetPluginId(CString& Id) const;
	int		GetNumParams() const;
	bool	GetParamName(int ParamIdx, CString& Name) const;
	float	GetParamDefault(int ParamIdx) const;
	int		GetPluginCaps(int CapsIdx) const;
	bool	GetExtendedInfo(PlugExtendedInfoStruct& PlugExtInfo) const;
	const	PlugExtendedInfoStruct *GetExtendedInfo() const;
	int		GetParamType(int ParamIdx) const;
	bool	GetPluginPath(CString& Path) const;

protected:
// Member data
	HINSTANCE	m_hInst;	// instance handle of the plugin's DLL
	FF_Main_FuncPtr	m_pff;	// pointer to the plugin's main function

// Helpers
	static	void	CopyFFString(CString& dst, const BYTE *src, DWORD len);

// Friends
	friend class CFFInstance;
};

inline bool CFFPlugin::IsLoaded() const
{
	return(m_pff != NULL);
}

#endif
