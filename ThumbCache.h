// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26sep07	initial version
		01		24apr10	use CPair for map positions
		02		22jun11	fix SetCacheLimit name
		03		28jun11	bump archive version for .NET, CTime is 64-bit

		thumbnail cache
 
*/

#ifndef CTHUMBCACHE_INCLUDED
#define CTHUMBCACHE_INCLUDED

#include "Thumb.h"
#include "MapEx.h"

class CThumbCache : public WObject {
public:
// Construction
	CThumbCache();
	~CThumbCache();

// Types
	typedef CMapEx<CString, LPCTSTR, CThumb, CThumb&> CThumbMap;
	typedef	CThumbMap::CPair *CThumbPos;

// Attributes
	CSize	GetThumbSize() const;
	DWORD	GetColorDepth() const;
	void	SetResolution(CSize ThumbSize, DWORD ColorDepth);
	void	SetFolder(LPCTSTR Folder);
	CThumbPos	GetThumb(LPCTSTR FileName) const;
	bool	IsEmpty() const;
	DWORD	GetCacheLimit() const;
	void	SetCacheLimit(DWORD Size);

// Operations
	BOOL	Lookup(LPCTSTR FileName, CThumb& Thumb) const;
	void	SetAt(LPCTSTR FileName, CThumb& Thumb);
	void	Empty();
	void	Flush();

protected:
// Types
	typedef struct tagFOLDER_INFO {
		CThumbMap *ThumbMap;	// pointer to folder's thumbnail map
		clock_t	LastAccess;		// when we last accessed this folder
	} FOLDER_INFO;
	typedef CMapEx<CString, LPCTSTR, FOLDER_INFO, FOLDER_INFO&> CFolderMap;

// Constants
	enum {
		ARCHIVE_SIG = 0x68546b63,	// archive signature (ckTh)
#if _MFC_VER < 0x0700
		ARCHIVE_VERSION = 1			// archive version number
#else
		ARCHIVE_VERSION = 2			// archive version number
#endif
	};
	static const bool	m_CanShrink;	// true if databases can shrink
	static const LPCTSTR	THUMB_DB_NAME;	// database name format string

// Member data
	CString	m_Folder;			// path of current folder
	CThumbMap	*m_ThumbMap;	// thumbnail map for current folder
	CFolderMap	m_FolderMap;	// map folders to thumbnail maps
	CSize	m_ThumbSize;		// thumbnail size, in pixels
	DWORD	m_ColorDepth;		// thumbnail color depth, in bits per pixel
	bool	m_IsDirty;			// true if thumbnail map has been modified
	DWORD	m_CacheLimit;		// maximum size of memory cache, in bytes
	DWORD	m_NumCached;		// total number of thumbnails cached in memory,
								// excluding thumbnails for current folder

// Helpers
	CString	MakeDBPath(LPCTSTR Folder) const;
	void	Serialize(CArchive& ar, CThumbMap& ThumbMap);
	bool	Read(CFile& fp, CThumbMap& ThumbMap);
	bool	Write(CFile& fp, CThumbMap& ThumbMap);
	bool	DoIO(CFile& fp, DWORD ArchiveMode, CThumbMap& ThumbMap);
	bool	FindLRUFolder(CString& Folder, FOLDER_INFO& Info) const;
	void	MakeRoom(DWORD BytesNeeded);
};

inline CSize CThumbCache::GetThumbSize() const
{
	return(m_ThumbSize);
}

inline DWORD CThumbCache::GetColorDepth() const
{
	return(m_ColorDepth);
}

inline BOOL CThumbCache::Lookup(LPCTSTR FileName, CThumb& Thumb) const
{
	ASSERT(m_ThumbMap != NULL);
	return(m_ThumbMap->Lookup(FileName, Thumb));
}

inline CThumbCache::CThumbPos CThumbCache::GetThumb(LPCTSTR FileName) const
{
	ASSERT(m_ThumbMap != NULL);
	return(m_ThumbMap->PLookup(FileName));
}

inline bool CThumbCache::IsEmpty() const
{
	return(m_ThumbMap == NULL);
}

inline bool CThumbCache::Read(CFile& fp, CThumbMap& ThumbMap)
{
	return(DoIO(fp, CArchive::load, ThumbMap));
}

inline bool CThumbCache::Write(CFile& fp, CThumbMap& ThumbMap)
{
	return(DoIO(fp, CArchive::store, ThumbMap));
}

inline DWORD CThumbCache::GetCacheLimit() const
{
	return(m_CacheLimit);
}

inline void CThumbCache::SetCacheLimit(DWORD Size)
{
	m_CacheLimit = Size;
}

#endif
