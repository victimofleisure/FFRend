// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26sep07	initial version
		
		thumbnail container
 
*/

#ifndef CTHUMB_INCLUDED
#define CTHUMB_INCLUDED

#include "BitmapInfo.h"

class CThumb : public CBitmapInfo {
public:
	DECLARE_SERIAL(CThumb);
// Construction
	CThumb();
	CThumb(const CThumb& Thumb);
	CThumb& operator=(const CThumb& Thumb);

// Attributes
	CTime	GetLastWrite() const;
	void	SetLastWrite(CTime LastWrite);

// Operations
	void	Serialize(CArchive& ar);

protected:
// Member data
	CTime	m_LastWrite;	// when thumbnail's source file was last written

// Helpers
	void	Copy(const CThumb& Thumb);
};

inline CThumb::CThumb()
{
	m_LastWrite = 0;
}

inline void CThumb::Copy(const CThumb& Thumb)
{
	CBitmapInfo::Copy(Thumb);
	m_LastWrite = Thumb.m_LastWrite;
}

inline CThumb::CThumb(const CThumb& Thumb)
{
	Copy(Thumb);
}

inline CThumb& CThumb::operator=(const CThumb& Thumb)
{
	Copy(Thumb);
	return(*this);
}

inline CTime CThumb::GetLastWrite() const
{
	return(m_LastWrite);
}

inline void CThumb::SetLastWrite(CTime LastWrite)
{
	m_LastWrite = LastWrite;
}

inline void CThumb::Serialize(CArchive& ar)
{
	CBitmapInfo::Serialize(ar);	// serialize base class
	if (ar.IsStoring()) {
		ar << m_LastWrite;
	} else {
		ar >> m_LastWrite;
	}
}

template<> inline void AFXAPI 
SerializeElements<CThumb>(CArchive& ar, CThumb *pNewItem, int nCount)
{
	for (int i = 0; i < nCount; i++, pNewItem++)
		pNewItem->Serialize(ar);
}

#endif
