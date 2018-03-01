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

#ifndef CBITMAPINFO_INCLUDED
#define CBITMAPINFO_INCLUDED

#include "SmartBuf.h"
#include <afxtempl.h>

class CBitmapInfo : public CObject {
public:
	DECLARE_SERIAL(CBitmapInfo);
// Construction
	CBitmapInfo();
	CBitmapInfo(const CBitmapInfo& Bmp);
	CBitmapInfo& operator=(const CBitmapInfo& Bmp);

// Operations
	bool	StoreBitmap(CBitmap& Bmp);
	bool	LoadBitmap(CBitmap& Bmp);
	void	Serialize(CArchive& ar);

protected:
// Member data
	BITMAP	m_Bmp;
	CSmartBuf	m_Bits;

// Helpers
	void	Copy(const CBitmapInfo& Bmp);
	void	SetBitmapInfo(BITMAPINFO& bi) const;
};

inline CBitmapInfo::CBitmapInfo()
{
	ZeroMemory(&m_Bmp, sizeof(m_Bmp));
}

inline CBitmapInfo::CBitmapInfo(const CBitmapInfo& Bmp)
{
	Copy(Bmp);
}

inline CBitmapInfo& CBitmapInfo::operator=(const CBitmapInfo& Bmp)
{
	Copy(Bmp);
	return(*this);
}

template<> inline void AFXAPI 
SerializeElements<CBitmapInfo>(CArchive& ar, CBitmapInfo *pNewItem, int nCount)
{
	for (int i = 0; i < nCount; i++, pNewItem++)
		pNewItem->Serialize(ar);
}

#endif
