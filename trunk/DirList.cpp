// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05jan07	initial version
		01		05oct07	add secondary name sort
		02		23nov07	support Unicode
		03		29jan08	in List, create CDirItem explicitly to fix warning
 		04		24feb09	GetLength is 64-bit from MFC7 on
		05		06jan10	W64: in List, cast filter array size to 32-bit

		directory listing
 
*/

#include "stdafx.h"
#include "DirList.h"
#include "PathStr.h"

const CDirList::COMPARE_FUNC  CDirList::m_CompareFunc[SORT_PROPS][SORT_ORDERS] = {
	{&CmpAscName,		&CmpDscName},
	{&CmpAscLength,		&CmpDscLength},	
	{&CmpAscFileType,	&CmpDscFileType},
	{&CmpAscLastWrite,	&CmpDscLastWrite}
};

CDirItem::CDirItem(int ObjType, const CString& Name, ULONGLONG Length, CTime LastWrite)
{
	m_ObjType = ObjType;
	m_Name = Name;
	m_Length = Length;
	m_LastWrite = LastWrite;
}

CDirItem::CDirItem(const CFileFind& Find)
{
	m_ObjType = Find.IsDirectory();
	m_Name = Find.GetFileName();
#if _MFC_VER >= 0x0700
	m_Length = Find.GetLength();	// MFC7 gets it right
#else
	m_Length = Find.GetLength64();	// MFC6 kludge
#endif
	Find.GetLastWriteTime(m_LastWrite);
}

void CDirItem::Copy(const CDirItem& Item)
{
	m_ObjType = Item.m_ObjType;
	m_Name = Item.m_Name;
	m_Length = Item.m_Length;
	m_LastWrite = Item.m_LastWrite;
}

LPCTSTR CDirItem::GetExtension() const
{
	return(PathFindExtension(m_Name));
}

CString CDirItem::GetPath(LPCTSTR Folder) const
{
	CPathStr	Path(Folder);
	Path.Append(m_Name);
	return(Path);
}

CDirList::CDirList()
{
	m_ShowParent = TRUE;
}

CString CDirList::GetItemPath(int Idx) const
{
	CPathStr	Path(m_Folder);
	Path.Append(GetItem(Idx).GetName());
	return(Path);
}

bool CDirList::List(const CString& Path, const CStringArray *Filter)
{
	m_Item.RemoveAll();
	CPathStr	FindStr(Path);
	FindStr.Append(_T("*.*"));
	CFileFind	ff;
	BOOL bWorking = ff.FindFile(FindStr);
	if (!bWorking)
		return(FALSE);
	if (m_ShowParent) {
		CDirItem	item(TRUE, "..", 0, 0);
		m_Item.Add(item);	// add link to parent
	}
	int	FiltCount = INT64TO32(Filter != NULL && Filter->GetSize() ? Filter->GetSize() : 0);
	while (bWorking) {
		bWorking = ff.FindNextFile();
		if (!ff.IsDots()) {
			CDirItem	item(ff);
			if (FiltCount && !item.IsDir()) {
				for (int i = 0; i < FiltCount; i++) {
					if (!_tcsicmp(PathFindExtension(item.GetName()), (*Filter)[i]))
						m_Item.Add(item);
				}
			} else
				m_Item.Add(item);
		}
	}
	CPathStr	AbsPath = ff.GetFilePath();
	AbsPath.RemoveFileSpec();
	m_Folder = AbsPath;	// absolute path of current folder
	MakeItemPtrs();
	return(TRUE);
}

bool CDirList::ListDrives()
{
	enum {
		DRIVE_LEN = 4,
		MAX_DRIVES = 26
	};
	TCHAR	DriveBuf[DRIVE_LEN * MAX_DRIVES + 1];
	if (!GetLogicalDriveStrings(sizeof(DriveBuf), DriveBuf))
		return(FALSE);
	LPCTSTR	p = DriveBuf;
	m_Item.RemoveAll();
	CDirItem	Item;
	Item.m_ObjType = CDirItem::OT_DRIVE;
	Item.m_Length = 0;
	Item.m_LastWrite = 0;
	while (*p) {
		Item.m_Name = p;
		m_Item.Add(Item);
		p += DRIVE_LEN;
	}
	MakeItemPtrs();
	m_Folder.Empty();	// no current folder
	return(TRUE);
}

void CDirList::MakeItemPtrs()
{
	int	items = GetCount();
	m_ItemPtr.SetSize(items);
	for (int i = 0; i < items; i++)
		m_ItemPtr[i] = &m_Item[i];
}

#define CDIRLIST_CMP_OBJ_TYPE(arg1, arg2)		\
	CDirItem	*item1 = *(CDirItem **)arg1;	\
	CDirItem	*item2 = *(CDirItem **)arg2;	\
	if (item1->m_ObjType < item2->m_ObjType)	\
		return(1);								\
	if (item1->m_ObjType > item2->m_ObjType)	\
		return(-1);

int CDirList::CmpAscName(const void *arg1, const void *arg2)
{
	CDIRLIST_CMP_OBJ_TYPE(arg1, arg2);
	return(_tcsicmp(item1->m_Name, item2->m_Name));
}

int CDirList::CmpDscName(const void *arg1, const void *arg2)
{
	return(-CmpAscName(arg1, arg2));
}

int CDirList::CmpAscLength(const void *arg1, const void *arg2)
{
	CDIRLIST_CMP_OBJ_TYPE(arg1, arg2);
	int	retc = SortCmp(item1->m_Length, item2->m_Length);
	return(retc ? retc : _tcsicmp(item1->m_Name, item2->m_Name));
}

int CDirList::CmpDscLength(const void *arg1, const void *arg2)
{
	return(-CmpAscLength(arg1, arg2));
}

int CDirList::CmpAscFileType(const void *arg1, const void *arg2)
{
	CDIRLIST_CMP_OBJ_TYPE(arg1, arg2);
	// ideally we should sort on the file's type name from SHGetFileInfo, but
	// that complicates things considerably, so instead we sort on extension
	int	retc = _tcsicmp(PathFindExtension(item1->m_Name), 
		PathFindExtension(item2->m_Name));
	return(retc ? retc : _tcsicmp(item1->m_Name, item2->m_Name));
}

int CDirList::CmpDscFileType(const void *arg1, const void *arg2)
{
	return(-CmpAscFileType(arg1, arg2));
}

int CDirList::CmpAscLastWrite(const void *arg1, const void *arg2)
{
	CDIRLIST_CMP_OBJ_TYPE(arg1, arg2);
	int	retc = SortCmp(item1->m_LastWrite, item2->m_LastWrite);
	return(retc ? retc : _tcsicmp(item1->m_Name, item2->m_Name));
}

int CDirList::CmpDscLastWrite(const void *arg1, const void *arg2)
{
	return(-CmpAscLastWrite(arg1, arg2));
}

void CDirList::Sort(int Property, int Order)
{
	ASSERT(Property >= 0 && Property < SORT_PROPS);
	ASSERT(Order >= 0 && Order < SORT_ORDERS);
	int	items = GetCount();
	if (items) {
		CDirItem**	data = m_ItemPtr.GetData();
		if (m_ShowParent) {
			// exclude parent link from sort, so it's always at the top
			data++;
			items--;
		}
		qsort(data, items, sizeof(void *), m_CompareFunc[Property][Order]);
	}
}
