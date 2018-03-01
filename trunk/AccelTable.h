// Copyleft 2004 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04feb05 initial version
        01      15feb09 add copy ctor and array support

		wrapper for accelerator table
 
*/

#ifndef ACCELTABLE_INCLUDED
#define ACCELTABLE_INCLUDED

#include "ArrayEx.h"

class CAccelTable;

typedef CArrayEx<ACCEL, ACCEL&> CAccelArray;

class CAccelTable : public CObject {
public:
// Construction
	CAccelTable();
	CAccelTable(const CAccelTable& Table);
	CAccelTable(const CAccelArray& AccArr);
	CAccelTable& operator=(const CAccelTable& Table);
	~CAccelTable();
	bool	LoadFromRes(int ResID);
	bool	LoadFromHandle(HACCEL hAccel);
	bool	LoadFromTable(const ACCEL *Table, int Entries);
	bool	LoadFromArray(const CAccelArray& AccArr);
	void	Destroy();

// Attributes
	operator bool() const;
	operator HACCEL() const;
	int		GetEntries() const;
	int		GetTable(ACCEL *Table, int Entries) const;
	int		GetArray(CAccelArray& AccArr) const;
	static	int		GetArray(HACCEL hAccel, CAccelArray& AccArr);
	static	bool	IsExtended(DWORD Key);
	static	void	GetKeyName(const ACCEL& Accel, CString& KeyName);
	static	void	GetCmdName(const ACCEL& Accel, CString& CmdName);
	static	void	GetCmdHelp(const ACCEL& Accel, CString& CmdHelp);

protected:
// Member data
	HACCEL	m_hAccel;	// if non-NULL, handle to an accelerator table
	bool	m_bDestroy;	// true if we're responsible for destroying table

// Helpers
	void	Copy(const CAccelTable& Table);
	static	CString	GetKeyNameText(LONG lParam);
};

inline CAccelTable::CAccelTable(const CAccelTable& Table)
{
	Copy(Table);
}

inline CAccelTable& CAccelTable::operator=(const CAccelTable& Table)
{
	if (&Table != this)	// avoid self-assignment
		Copy(Table);
	return(*this);
}

inline CAccelTable::operator bool() const
{
	return(m_hAccel != NULL);
}

inline CAccelTable::operator HACCEL() const
{
	return(m_hAccel);
}

inline bool CAccelTable::LoadFromArray(const CAccelArray& AccArr)
{
	return(LoadFromTable(AccArr.GetData(), AccArr.GetSize()));
}

inline int CAccelTable::GetArray(CAccelArray& AccArr) const
{
	return(GetArray(m_hAccel, AccArr));
}

#endif
