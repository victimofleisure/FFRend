// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05jan07	initial version
		01		17sep07	support large icons
		02		05oct07	rename and don't derive from CDirList
		03		23nov07	support Unicode
		04		06jan10	W64: in GetFileInfo, SHGetFileInfo result can be 64-bit

		retrieve and cache shell file info
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "FileInfoCache.h"
#include "PathStr.h"

CFileInfo::CFileInfo(int IconIdx, const CString& TypeName)
{
	m_IconIdx = IconIdx;
	m_TypeName = TypeName;
}

void CFileInfo::Copy(const CFileInfo& Item)
{
	m_IconIdx = Item.m_IconIdx;
	m_TypeName = Item.m_TypeName;
}

CFileInfoCache::CFileInfoCache()
{
	HIMAGELIST	hil;
	SHFILEINFO	shfi;
	hil = (HIMAGELIST)SHGetFileInfo(_T(""), 0, &shfi, sizeof(shfi), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON);	// small icons
	m_ImgList[SMALL_ICON].Attach(hil);	// attach our CImageList to system image list
	hil = (HIMAGELIST)SHGetFileInfo(_T(""), 0, &shfi, sizeof(shfi), 
		SHGFI_SYSICONINDEX);	// large icons
	m_ImgList[LARGE_ICON].Attach(hil);	// attach our CImageList to system image list
}

CFileInfoCache::~CFileInfoCache()
{
	m_ImgList[SMALL_ICON].Detach();	// so CImageList doesn't delete system image list
	m_ImgList[LARGE_ICON].Detach();	// so CImageList doesn't delete system image list
}

bool CFileInfoCache::GetFileInfo(LPCTSTR Folder, const CDirItem& Item, CFileInfo& Info)
{
	// SHGetFileInfo is expensive, so only call it once per file extension,
	// except for drives, apps and shortcuts, because they have custom icons
	const CString& Name = Item.GetName();
	CString	Ext;
	if (Item.IsDir()) {
		if (Item.IsDrive())
			Ext = Name;	// drive icons are potentially unique
		else
			Ext = ":";	// this string is reserved for folders
	} else {
		Ext = PathFindExtension(Name);	// get file extension
		Ext.MakeUpper();	// extension lookup is case-insensitive
		if (Ext == ".EXE" || Ext == ".LNK")	// if app or shortcut
			Ext = Name;	// app icons are potentially unique
	}
	if (!m_ExtInfo.Lookup(Ext, Info)) {	// if extension not mapped yet
		CPathStr	Path;
		if (*Name != '.') {	// skip parent link, else Append trims it
			Path = Folder;
			Path.Append(Name);
		} else
			Path = Name;
		SHFILEINFO	shfi;
		W64UINT	il = SHGetFileInfo(Path, 0, &shfi, sizeof(shfi), 
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME);
		if (!il)
			return(FALSE);
		Info.m_IconIdx = shfi.iIcon;
		if (*shfi.szTypeName)	// if type name was found, use it
			Info.m_TypeName = shfi.szTypeName;
		else {
			if (*Ext == '.')		// if extension is valid, use it instead
				Info.m_TypeName = Ext.Mid(1) + " " + LDS(IDS_FB_FILE_SUFFIX);
			else
				Info.m_TypeName = LDS(IDS_FB_FILE_SUFFIX);	// last resort
		}
		m_ExtInfo.SetAt(Ext, Info);		// map extension to file info
	}
	return(TRUE);
}
