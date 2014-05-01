// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06jan10	W64: adapt for 64-bit
		
		enhanced array with copy ctor, assignment, and fast const access
 
*/

#ifndef CARRAYEX_INCLUDED
#define CARRAYEX_INCLUDED

#include <afxtempl.h>

template<class TYPE, class ARG_TYPE>
class CArrayEx : public CArray<TYPE, ARG_TYPE> {
public:
// Construction
	CArrayEx() {}	// inline body here to avoid internal compiler error
	CArrayEx(const CArrayEx& arr);
	CArrayEx& operator=(const CArrayEx& arr);

// Attributes
	int	GetSize() const;

// Operations
	TYPE& ElementAt(int nIndex);
	const TYPE& ElementAt(int nIndex) const;
	TYPE& operator[](int nIndex);
	const TYPE& operator[](int nIndex) const;
	void Detach(TYPE*& pData, int& Size);
	void Attach(TYPE *pData, int Size);
	void Swap(CArrayEx& src);
};

template<class TYPE, class ARG_TYPE>
AFX_INLINE CArrayEx<TYPE, ARG_TYPE>::CArrayEx(const CArrayEx& arr)
{
	*this = arr;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE CArrayEx<TYPE, ARG_TYPE>& 
CArrayEx<TYPE, ARG_TYPE>::operator=(const CArrayEx& arr)
{
	if (this != &arr)
		Copy(arr);
	return *this;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE TYPE& CArrayEx<TYPE, ARG_TYPE>::ElementAt(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

template<class TYPE, class ARG_TYPE>
const AFX_INLINE TYPE& CArrayEx<TYPE, ARG_TYPE>::ElementAt(int nIndex) const
{
	ASSERT(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE TYPE& CArrayEx<TYPE, ARG_TYPE>::operator[](int nIndex)
{
	return ElementAt(nIndex);
}

template<class TYPE, class ARG_TYPE>
const AFX_INLINE TYPE& CArrayEx<TYPE, ARG_TYPE>::operator[](int nIndex) const
{
	return ElementAt(nIndex);	// base class uses GetAt which is too slow
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::Detach(TYPE*& pData, int& nSize)
{
	ASSERT_VALID(this);
	pData = m_pData;
	nSize = INT64TO32(m_nSize);	// W64: force to 32-bit
	m_pData = NULL;
	m_nSize = 0;
	m_nMaxSize = 0;
	m_nGrowBy = -1;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::Attach(TYPE *pData, int nSize)
{
	ASSERT_VALID(this);
	RemoveAll();
	m_pData = pData;
	m_nSize = nSize;
	m_nMaxSize = nSize;
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE void CArrayEx<TYPE, ARG_TYPE>::Swap(CArrayEx& src)
{
	ASSERT_VALID(this);
	TYPE	*pOurData, *pSrcData;
	int	nOurSize, nSrcSize;
	Detach(pOurData, nOurSize);	// detach our data
	src.Detach(pSrcData, nSrcSize);	// detach source data
	src.Attach(pOurData, nOurSize);	// attach our data to source
	Attach(pSrcData, nSrcSize);	// attach source data to us
}

template<class TYPE, class ARG_TYPE>
AFX_INLINE int CArrayEx<TYPE, ARG_TYPE>::GetSize() const
{
	return(INT64TO32(m_nSize));	// W64: force to 32-bit
}

#endif
