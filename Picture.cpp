// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21aug06	initial version
		01		23nov07	support Unicode
		02		27mar10	overload Open for CFile
		03		12apr10	in Read, cast file length to 32-bit

        IPicture wrapper for loading image files
 
*/

#include "stdafx.h"
#include "Picture.h"

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

CPicture::CPicture()
{
	m_p = NULL;
}

CPicture::~CPicture()
{
	Close();
}

bool CPicture::Read(CFile& File)
{
	UINT	len = static_cast<UINT>(File.GetLength());	// limits pics to 4GB
	HGLOBAL	hg = GlobalAlloc(GPTR, len);
	if (hg) {
		File.Read(hg, len);
		IStream	*s;
		CreateStreamOnHGlobal(hg, FALSE, &s);
		if (s) {
			OleLoadPicture(s, 0, FALSE, IID_IPicture, (void **)&m_p);
			s->Release();
		}
		GlobalFree(hg);
	}
	return(IsOpen());
}

bool CPicture::Open(LPCTSTR Path)
{
	Close();
	CFile	fi;
	if (!fi.Open(Path, CFile::modeRead | CFile::shareDenyWrite))
		return(FALSE);
	return(Read(fi));
}

void CPicture::Close()
{
	SAFE_RELEASE(m_p);
}
