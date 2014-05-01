// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26sep07	initial version
        01      24apr10	refactor to support .NET CPair interface
		
		map that can access keys and values by reference
 
*/

#pragma once

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMapEx : public CMap<KEY, ARG_KEY, VALUE, ARG_VALUE> {
public:
#if _MFC_VER < 0x0700	// MFC 6 doesn't support CPair
	// can't inherit from CAssoc because it's protected, so copy it and cast
	struct CPair
	{
		CAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		KEY key;
		VALUE value;
	};
	// CPair functions
	const CPair* PLookup(ARG_KEY key) const;
	CPair* PLookup(ARG_KEY key);
	const CPair* PGetFirstAssoc() const;
	CPair* PGetFirstAssoc();
	const CPair *PGetNextAssoc(const CPair* pPairRet) const;
	CPair *PGetNextAssoc(const CPair* pPairRet);
#endif	// _MFC_VER < 0x0700
};

#if _MFC_VER < 0x0700	// MFC 6 doesn't support CPair
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
const CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::CPair* CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::PLookup(ARG_KEY key) const
{
	UINT nHash;
	return (CPair *)GetAssocAt(key, nHash);
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::CPair* CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::PLookup(ARG_KEY key)
{
	UINT nHash;
	return (CPair *)GetAssocAt(key, nHash);
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
const CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::CPair* CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::PGetFirstAssoc() const
{
	ASSERT_VALID(this);
	if(m_nCount == 0) return NULL;

	ASSERT(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)BEFORE_START_POSITION;

	// find the first association
	for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
		if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
			break;
	ASSERT(pAssocRet != NULL);  // must find something

	return (CPair *)pAssocRet;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::CPair* CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::PGetFirstAssoc()
{
	ASSERT_VALID(this);
	if(m_nCount == 0) return NULL;

	ASSERT(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)BEFORE_START_POSITION;

	// find the first association
	for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
		if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
			break;
	ASSERT(pAssocRet != NULL);  // must find something

	return (CPair *)pAssocRet;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
const CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::CPair *CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::PGetNextAssoc(const CPair* pPairRet) const
{
	ASSERT_VALID(this);

	CAssoc* pAssocRet = (CAssoc*)pPairRet;

	ASSERT(m_pHashTable != NULL);  // never call on empty map
	ASSERT(pAssocRet != NULL);
	
	if(m_pHashTable == NULL || pAssocRet == NULL)
		return NULL;
		
	ASSERT(pAssocRet != (CAssoc*)BEFORE_START_POSITION);

	// find next association
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = (pAssocRet->nHashValue % m_nHashTableSize) + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	return (CPair *)pAssocNext;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::CPair *CMapEx<KEY, ARG_KEY, VALUE, ARG_VALUE>::PGetNextAssoc(const CPair* pPairRet)
{
	ASSERT_VALID(this);

	CAssoc* pAssocRet = (CAssoc*)pPairRet;

	ASSERT(m_pHashTable != NULL);  // never call on empty map
	ASSERT(pAssocRet != NULL);
	
	if(m_pHashTable == NULL || pAssocRet == NULL)
		return NULL;
		
	ASSERT(pAssocRet != (CAssoc*)BEFORE_START_POSITION);

	// find next association
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = (pAssocRet->nHashValue % m_nHashTableSize) + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	return (CPair *)pAssocNext;
}
#endif	// _MFC_VER < 0x0700

