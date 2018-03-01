// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25dec07	initial version

        metaparameter group
 
*/

#include "stdafx.h"
#include "MetaparmGroup.h"

IMPLEMENT_SERIAL(CMetaparmGroup, CObject, 1);

CMetaparmGroup::CMetaparmGroup()
{
	m_Master = -1;
}

void CMetaparmGroup::Copy(const CMetaparmGroup& Info)
{
	m_Master = Info.m_Master;
	m_Slave.Copy(Info.m_Slave);
}

void CMetaparmGroup::Empty()
{
	m_Master = -1;
	m_Slave.RemoveAll();
}

void CMetaparmGroup::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Master;
	} else {
		ar >> m_Master;
	}
	m_Slave.Serialize(ar);
}
