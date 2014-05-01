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

        metaparameter array
 
*/

#ifndef CMETAPARMARRAY_INCLUDED
#define CMETAPARMARRAY_INCLUDED

#include "Metaparm.h"

class CMetaparmArray : public WObject {
public:
// Attributes
	int		GetSize() const;
	void	SetSize(int NewSize);
	int		GetMaster(int ParmIdx) const;
	void	GetGroup(int ParmIdx, CMetaparmGroup& Group) const;
	void	SetGroup(const CMetaparmGroup& Group);
	void	GetGroups(CMetaparmGroupList& GroupList) const;
	void	SetGroups(const CMetaparmGroupList& GroupList);
	LPCTSTR	GetGroupName(int ParmIdx) const;

// Operations
	const	CMetaparm& GetAt(int ParmIdx) const;
	CMetaparm&	GetAt(int ParmIdx);
	void	Reset(int ParmIdx);
	void	InsertAt(int ParmIdx, CMetaparm& Metaparm);
	void	RemoveAt(int ParmIdx);
	void	Move(int Src, int Dst);
	void	Copy(const CMetaparmArray& src);
	void	Serialize(CArchive& ar);
	void	RemoveAllGroups();
	void	DumpGroups() const;
	void	Group(int MasterIdx, int SlaveIdx);
	void	Ungroup(int ParmIdx);
	void	Unlink(int ParmIdx);
	int		Find(const CMetaparm& Metaparm) const;

// Operators
	const CMetaparm&	operator[](int ParmIdx) const;
	CMetaparm&	operator[](int ParmIdx);

// Nested classes
	class CGroupLinkIter : public WObject {
	public:
		CGroupLinkIter(CMetaparmArray& Metaparm);
		int		*Next();

	private:
		CMetaparmArray& m_Metaparm;	// metaparameter array to iterate
		int		m_ParmIdx;	// index of current metaparameter 
		int		m_SlaveIdx;	// index of current slave, or -1 for master
	};

protected:
// Member data
	CArrayEx<CMetaparm, CMetaparm&>	m_Metaparm;	// array of metaparameters

// Helpers
	void	OffsetLinks(int ParmIdx, int Offset);
};

inline int CMetaparmArray::GetSize() const
{
	return(m_Metaparm.GetSize());
}

inline void CMetaparmArray::SetSize(int NewSize)
{
	m_Metaparm.SetSize(NewSize);
}

inline const CMetaparm& CMetaparmArray::GetAt(int ParmIdx) const
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	return(m_Metaparm.GetData()[ParmIdx]);
}

inline CMetaparm& CMetaparmArray::GetAt(int ParmIdx)
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	return(m_Metaparm[ParmIdx]);
}

inline const CMetaparm& CMetaparmArray::operator[](int ParmIdx) const
{
	return(GetAt(ParmIdx));
}

inline CMetaparm& CMetaparmArray::operator[](int ParmIdx)
{
	return(GetAt(ParmIdx));
}

inline void	CMetaparmArray::Copy(const CMetaparmArray& src)
{
	m_Metaparm.Copy(src.m_Metaparm);
}

inline void CMetaparmArray::Serialize(CArchive& ar)
{
	m_Metaparm.Serialize(ar);
}

#endif
