// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25dec07	initial version
		01		24apr10	use ArrayEx

        metaparameter group
 
*/

#ifndef CMETAPARMGROUP_INCLUDED
#define CMETAPARMGROUP_INCLUDED

#include "ArrayEx.h"

class CMetaparmGroup : public CObject {
	DECLARE_SERIAL(CMetaparmGroup);
public:
// Types
	typedef CArrayEx<int, int&> CSlaveArray;

// Construction
	CMetaparmGroup();
	CMetaparmGroup(const CMetaparmGroup& Group);
	CMetaparmGroup& operator=(const CMetaparmGroup& Group);

// Public data
	// don't forget to add new members to ctor, Copy, and Serialize
	int		m_Master;		// if slave, index of master, else -1
	CSlaveArray	m_Slave;	// if master, array of slave indices

// Attributes
	int		GetSlaveCount() const;

// Operations
	void	Empty();
	void	Serialize(CArchive& ar);

protected:
// Helpers
	void	Copy(const CMetaparmGroup& Group);
};

typedef CArrayEx<CMetaparmGroup, CMetaparmGroup&> CMetaparmGroupList;

template<> inline void AFXAPI 
SerializeElements<CMetaparmGroup>(CArchive& ar, CMetaparmGroup *pNewGroup, int nCount)
{
	for (int i = 0; i < nCount; i++, pNewGroup++)
		pNewGroup->Serialize(ar);
}

inline CMetaparmGroup::CMetaparmGroup(const CMetaparmGroup& Group)
{
	Copy(Group);
}

inline CMetaparmGroup& CMetaparmGroup::operator=(const CMetaparmGroup& Group)
{
	Copy(Group);
	return(*this);
}

inline int CMetaparmGroup::GetSlaveCount() const
{
	return(m_Slave.GetSize());
}

#endif
