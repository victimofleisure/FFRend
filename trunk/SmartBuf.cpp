// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      22feb04	initial version
        01      25sep07	add Serialize

        smart buffer
 
*/

#include "stdafx.h"
#include "SmartBuf.h"

void CSmartBuf::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar << m_Len;
		ar.Write(m_Data, m_Len);
	} else {
		DWORD	Len;
		ar >> Len;
		SetSize(Len);
		ar.Read(m_Data, Len);
	}
}
