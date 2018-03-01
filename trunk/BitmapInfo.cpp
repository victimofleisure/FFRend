// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17sep07	initial version

		bitmap container with serialization

*/

#include "stdafx.h"
#include "BitmapInfo.h"

IMPLEMENT_SERIAL(CBitmapInfo, CObject, 1);

void CBitmapInfo::Copy(const CBitmapInfo& Bmp)
{
	m_Bmp = Bmp.m_Bmp;
	m_Bits.Copy(Bmp.m_Bits);
	m_Bmp.bmBits = m_Bits.GetData();
}

inline void CBitmapInfo::SetBitmapInfo(BITMAPINFO& bi) const
{
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = m_Bmp.bmWidth;
	bi.bmiHeader.biHeight = m_Bmp.bmHeight;
	bi.bmiHeader.biPlanes = m_Bmp.bmPlanes;
	bi.bmiHeader.biBitCount = m_Bmp.bmBitsPixel;
	bi.bmiHeader.biCompression = BI_RGB;
}

bool CBitmapInfo::StoreBitmap(CBitmap& Bmp)
{
	if (!Bmp.GetBitmap(&m_Bmp))
		return(FALSE);
	m_Bits.SetSize(m_Bmp.bmWidthBytes * m_Bmp.bmHeight);
	m_Bmp.bmBits = m_Bits.GetData();
	bool	retc = FALSE;
	HDC	hDC = GetDC(NULL);
	BITMAPINFO	bi;
	SetBitmapInfo(bi);
	if (GetDIBits(hDC, Bmp, 0, m_Bmp.bmHeight, m_Bmp.bmBits, &bi, DIB_RGB_COLORS))
		retc = TRUE;
	ReleaseDC(NULL, hDC);
	return(retc);
}

bool CBitmapInfo::LoadBitmap(CBitmap& Bmp)
{
	ASSERT(Bmp.m_hObject == NULL);	// must be uninitialized, else handle leaks
	bool	retc = FALSE;
	HDC	hDC = GetDC(NULL);
	Bmp.m_hObject = CreateCompatibleBitmap(hDC, m_Bmp.bmWidth, m_Bmp.bmHeight);
	BITMAPINFO	bi;
	SetBitmapInfo(bi);
	if (SetDIBits(hDC, Bmp, 0, m_Bmp.bmHeight, m_Bmp.bmBits, &bi, DIB_RGB_COLORS))
		retc = TRUE;
	ReleaseDC(NULL, hDC);
	return(retc);
}

void CBitmapInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ar.Write(&m_Bmp, sizeof(m_Bmp));
		m_Bits.Serialize(ar);
	} else {
		ar.Read(&m_Bmp, sizeof(m_Bmp));
		m_Bits.Serialize(ar);
		m_Bmp.bmBits = m_Bits.GetData();
	}
}
