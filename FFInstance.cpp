// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		24jul06	initial version
		01		19dec06	add process frame copy
		02		03apr11	add attach/detach

		wrapper for freeframe plugin instance

*/

#include "stdafx.h"
#include "FFInstance.h"

CFFInstance::CFFInstance()
{
	m_pff = NULL;
	m_Instance = NULL;
}

CFFInstance::~CFFInstance()
{
	Destroy();
}

bool CFFInstance::Create(CFFPlugin *Plugin, const VideoInfoStruct& VideoInfo)
{
	if (Plugin == NULL)
		return(FALSE);
	Destroy();
	m_pff = Plugin->m_pff;
	m_Instance = m_pff(FF_INSTANTIATE, LPVOID(&VideoInfo), 0);
	if (m_Instance == LPVOID(FF_FAIL))
		m_Instance = NULL;
	return(m_Instance != NULL);
}

bool CFFInstance::Destroy()
{
	bool	retc;
	if (m_Instance != NULL) {
		retc = ((int)CallFF(FF_DEINSTANTIATE, 0) != FF_FAIL);
		m_Instance = NULL;
	} else
		retc = FALSE;
	m_pff = NULL;
	return(retc);
}

bool CFFInstance::Attach(const CFFInstance& Instance)
{
	if (!Instance.IsCreated())
		return(FALSE);
	m_Instance = Instance.m_Instance;
	m_pff = Instance.m_pff;
	return(TRUE);
}

void CFFInstance::Detach()
{
	m_Instance = NULL;
	m_pff = NULL;
}

