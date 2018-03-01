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
 
		retrieve and cache shell file info
 
*/

#ifndef CFILEINFOCACHE_INCLUDED
#define CFILEINFOCACHE_INCLUDED

#include "DirList.h"

class CFileInfo : public WObject {
public:
// Construction
	CFileInfo();
	CFileInfo(int IconIdx, const CString& TypeName);
	CFileInfo(const CFileInfo& Info);
	CFileInfo& operator=(const CFileInfo& Info);

// Attributes
	int		GetIconIdx() const;
	const	CString& GetTypeName() const;

protected:
// Data members
	int		m_IconIdx;		// index of file's icon in system image list
	CString	m_TypeName;		// file's type name

// Helpers
	void	Copy(const CFileInfo& Info);

// Friends
	friend	class	CFileInfoCache;
};

class CFileInfoCache : public WObject {
public:
// Construction
	CFileInfoCache();
	~CFileInfoCache();

// Constants
	enum {	// image list icon types
		SMALL_ICON,
		LARGE_ICON,
		ICON_TYPES
	};

// Attributes
	CImageList&	GetImageList(int IconType);
	CImageList&	GetLargeImageList();
	bool	GetFileInfo(LPCTSTR Folder, const CDirItem& Item, CFileInfo& Info);

// Types
	typedef CMap<CString, LPCTSTR, CFileInfo, CFileInfo&> CExtInfoMap;

protected:
// Data members
	CImageList	m_ImgList[ICON_TYPES];	// attached to system image lists
	CExtInfoMap	m_ExtInfo;		// map extensions to file info
};

inline CFileInfo::CFileInfo()
{
	m_IconIdx = -1;	// no icon
}

inline CFileInfo::CFileInfo(const CFileInfo& Info)
{
	Copy(Info);
}

inline CFileInfo& CFileInfo::operator=(const CFileInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline int CFileInfo::GetIconIdx() const
{
	return(m_IconIdx);
}

inline const CString& CFileInfo::GetTypeName() const
{
	return(m_TypeName);
}

inline CImageList& CFileInfoCache::GetImageList(int IconType)
{
	ASSERT(IconType >= 0 && IconType < ICON_TYPES);
	return(m_ImgList[IconType]);
}

#endif
