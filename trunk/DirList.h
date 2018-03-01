// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05jan07	initial version
		01		05oct07	add user-defined item data
		02		24feb09	make file size unsigned
		03		05jan10	use CArrayEx

		directory listing
 
*/

#ifndef CDIRLIST_INCLUDED
#define CDIRLIST_INCLUDED

#include "ArrayEx.h"

class CDirItem : public WObject {
public:
// Constants
	enum {	// object types
		OT_FILE,
		OT_FOLDER,
		OT_DRIVE,
		OBJECT_TYPES
	};

// Construction
	CDirItem();
	CDirItem(int ObjType, const CString& Name, ULONGLONG Length, CTime LastWrite);
	CDirItem(const CFileFind& Find);
	CDirItem(const CDirItem& Item);
	CDirItem& operator=(const CDirItem& Item);

// Attributes
	int		GetObjType() const;
	bool	IsDir() const;
	bool	IsDrive() const;
	bool	IsDots() const;
	const	CString& GetName() const;
	void	SetName(const CString& Name);
	LPCTSTR	GetExtension() const;
	ULONGLONG	GetLength() const;
	CTime	GetLastWrite() const;
	CString	GetPath(LPCTSTR Folder) const;
	DWORD	GetData() const;
	void	SetData(DWORD Data);

protected:
// Data members
	int		m_ObjType;		// object type; see enum above
	CString	m_Name;			// item name, including extension
	CTime	m_LastWrite;	// when item was last modified
	DWORD	m_Data;			// user-defined data
	ULONGLONG	m_Length;	// item size, in bytes

// Helpers
	void	Copy(const CDirItem& Item);

// Friends
	friend	class	CDirList;
};

typedef CArrayEx<CDirItem, CDirItem&> CDirItemArray;

class CDirList : public WObject {
public:
// Construction
	CDirList();

// Constants
	enum {	// sortable properties
		SORT_NAME,
		SORT_LENGTH,
		SORT_FILE_TYPE,
		SORT_LAST_WRITE,
		SORT_PROPS
	};
	enum {	// sort orders
		ORDER_ASC,
		ORDER_DESC,
		SORT_ORDERS
	};

// Attributes
	const	CString& GetFolder() const;
	int		GetCount() const;
	const	CDirItem& GetItem(int Idx) const;
	CDirItem&	GetItem(int Idx);
	CString	GetItemPath(int Idx) const;
	void	ShowParent(bool Enable);

// Operations
	bool	List(const CString& Path, const CStringArray *Filter = NULL);
	bool	ListDrives();
	void	Sort(int Property, int Order);

protected:
// Types
	typedef CArrayEx<CDirItem*, CDirItem*&> CItemPtrArray;
	typedef	int	(*COMPARE_FUNC)(const void *arg1, const void *arg2);

// Constants
	static const COMPARE_FUNC	m_CompareFunc[SORT_PROPS][SORT_ORDERS];

// Data members
	CDirItemArray	m_Item;		// items in current folder
	CItemPtrArray	m_ItemPtr;	// sorted pointers to items
	CString	m_Folder;			// path of current folder
	bool	m_ShowParent;		// if true, show parent link

// Helpers
	void	MakeItemPtrs();
	template<class T> static int SortCmp(const T& a, const T& b)
		{ return(a == b ? 0 : (a < b ? -1 : 1)); }
	static	int		CmpAscName(const void *arg1, const void *arg2);
	static	int		CmpDscName(const void *arg1, const void *arg2);
	static	int		CmpAscLength(const void *arg1, const void *arg2);
	static	int		CmpDscLength(const void *arg1, const void *arg2);
	static	int		CmpAscFileType(const void *arg1, const void *arg2);
	static	int		CmpDscFileType(const void *arg1, const void *arg2);
	static	int		CmpAscLastWrite(const void *arg1, const void *arg2);
	static	int		CmpDscLastWrite(const void *arg1, const void *arg2);
};

inline CDirItem::CDirItem()
{
}

inline CDirItem::CDirItem(const CDirItem& Item)
{
	Copy(Item);
}

inline CDirItem& CDirItem::operator=(const CDirItem& Item)
{
	Copy(Item);
	return(*this);
}

inline int CDirItem::GetObjType() const
{
	return(m_ObjType);
}

inline bool CDirItem::IsDir() const
{
	return(m_ObjType != OT_FILE);	// folder or drive
}

inline bool CDirItem::IsDrive() const
{
	return(m_ObjType == OT_DRIVE);
}

inline bool CDirItem::IsDots() const
{
	return(m_Name == "..");
}

inline const CString& CDirItem::GetName() const
{
	return(m_Name);
}

inline void CDirItem::SetName(const CString& Name)
{
	m_Name = Name;
}

inline ULONGLONG CDirItem::GetLength() const
{
	return(m_Length);
}

inline CTime CDirItem::GetLastWrite() const
{
	return(m_LastWrite);
}

inline DWORD CDirItem::GetData() const
{
	return(m_Data);
}

inline void CDirItem::SetData(DWORD Data)
{
	m_Data = Data;
}

inline const CString& CDirList::GetFolder() const
{
	return(m_Folder);
}

inline int CDirList::GetCount() const
{
	return(m_Item.GetSize());
}

inline const CDirItem& CDirList::GetItem(int Idx) const
{
	return(*m_ItemPtr[Idx]);	// sort adds a level of indirection
}

inline CDirItem& CDirList::GetItem(int Idx)
{
	return(*m_ItemPtr[Idx]);	// sort adds a level of indirection
}

#endif
