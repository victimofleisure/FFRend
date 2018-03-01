// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		24jul06	initial version
		01		06nov06	call initialise and deInitialise
		02		21jan07	GetPluginPath must call ReleaseBuffer
		03		23nov07	support Unicode

		wrapper for freeframe plugin DLL

*/

#include "stdafx.h"
#include "FFPlugin.h"

CFFPlugin::CFFPlugin()
{
	m_hInst = NULL;
	m_pff = NULL;
}

CFFPlugin::~CFFPlugin()
{
	Free();
}

bool CFFPlugin::Load(LPCTSTR Path)
{
	Free();
	m_hInst = LoadLibrary(Path);
	if (m_hInst != NULL) {
		m_pff = (FF_Main_FuncPtr)GetProcAddress(m_hInst, "plugMain");
		if (m_pff != NULL)
			return((int)m_pff(FF_INITIALISE, 0, 0) == FF_SUCCESS);
	}
	return(FALSE);
}

bool CFFPlugin::Free()
{
	if (m_pff != NULL) {
		(int)m_pff(FF_DEINITIALISE, 0, 0);
		m_pff = NULL;
	}
	if (m_hInst != NULL) {
		FreeLibrary(m_hInst);
		m_hInst = NULL;
		return(TRUE);
	}
	return(FALSE);
}

const PlugInfoStruct *CFFPlugin::GetInfo() const
{
	return((PlugInfoStruct *)m_pff(FF_GETINFO, 0, 0));
}

bool CFFPlugin::GetInfo(PlugInfoStruct& PlugInfo) const
{
	const PlugInfoStruct *pis = GetInfo();
	if (pis == NULL)
		return(FALSE);
	PlugInfo = *GetInfo();
	return(TRUE);
}

void CFFPlugin::CopyFFString(CString& dst, const BYTE *src, DWORD len)
{
	// src should be fixed-length, but it might be null-terminated instead
	LPTSTR	buf = dst.GetBuffer(len + 1);
#ifdef UNICODE
	for (DWORD i = 0; i < len; i++)
		buf[i] = src[i];
#else
	memcpy(buf, src, len);
#endif
	buf[len] = 0;
	dst.ReleaseBuffer();
	dst.TrimRight();
}

bool CFFPlugin::GetPluginName(CString& Name) const
{
	const PlugInfoStruct *pis = GetInfo();
	if (pis == NULL)
		return(FALSE);
	CopyFFString(Name, GetInfo()->pluginName, 16);
	return(TRUE);
}

bool CFFPlugin::GetPluginId(CString& Id) const
{
	const PlugInfoStruct *pis = GetInfo();
	if (pis == NULL)
		return(FALSE);
	CopyFFString(Id, GetInfo()->uniqueID, 4);
	return(TRUE);
}

int CFFPlugin::GetNumParams() const
{
	return((int)m_pff(FF_GETNUMPARAMETERS, 0, 0));
}

bool CFFPlugin::GetParamName(int ParamIdx, CString& Name) const
{
	const BYTE *p = (const BYTE *)m_pff(FF_GETPARAMETERNAME, LPVOID(ParamIdx), 0);
	if (p == NULL)
		return(FALSE);
	CopyFFString(Name, p, 16);
	return(TRUE);
}

float CFFPlugin::GetParamDefault(int ParamIdx) const
{
	plugMainUnionTag	pmu;
	pmu.ivalue = (int)m_pff(FF_GETPARAMETERDEFAULT, LPVOID(ParamIdx), 0);
	return(pmu.fvalue);
}

int CFFPlugin::GetPluginCaps(int CapsIdx) const
{
	return((int)m_pff(FF_GETPLUGINCAPS, LPVOID(CapsIdx), 0));
}

const PlugExtendedInfoStruct *CFFPlugin::GetExtendedInfo() const
{
	return((PlugExtendedInfoStruct *)m_pff(FF_GETEXTENDEDINFO, 0, 0));
}

bool CFFPlugin::GetExtendedInfo(PlugExtendedInfoStruct& PlugExtInfo) const
{
	const PlugExtendedInfoStruct *peis = GetExtendedInfo();
	if (peis == NULL)
		return(FALSE);
	PlugExtInfo = *peis;
	return(TRUE);
}

int CFFPlugin::GetParamType(int ParamIdx) const
{
	return((int)m_pff(FF_GETPARAMETERTYPE, LPVOID(ParamIdx), 0));
}

bool CFFPlugin::GetPluginPath(CString& Path) const
{
	if (IsLoaded()) {
		LPTSTR	buf = Path.GetBuffer(MAX_PATH);
		bool	retc = GetModuleFileName(m_hInst, buf, MAX_PATH) != 0;
		Path.ReleaseBuffer();
		return(retc);
	}
	Path.Empty();
	return(TRUE);
}
