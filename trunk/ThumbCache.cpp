// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      26sep07	initial version
		01		24apr10	W64: in SetFolder, case thumb file length to 32-bit
		02		24apr10	use CPair for map positions
		03		22jun11	W64: cast thumb map's GetCount to 32-bit
		04		22jun11	W64: in MakeRoom, cast GetLength to 32-bit
		
		thumbnail cache
 
*/

#include "stdafx.h"
#include "ThumbCache.h"
#include "PathStr.h"

// Setting this option causes Flush to remove thumbnails that no longer have
// source files, due the source files being deleted or renamed. Without this
// option, Flush runs faster, but thumbnail databases can't shrink, and will
// become bloated if source files are frequently deleted or renamed.
const bool CThumbCache::m_CanShrink = FALSE;

const LPCTSTR CThumbCache::THUMB_DB_NAME = _T("ckThumbs%dx%dx%d.db");

CThumbCache::CThumbCache()
{
	m_ThumbMap = NULL;
	m_ThumbSize = CSize(0, 0);
	m_ColorDepth = 0;
	m_IsDirty = FALSE;
	m_CacheLimit = 30000000;
	m_NumCached = 0;
}

CThumbCache::~CThumbCache()
{
	if (!IsEmpty())
		Empty();
}

void CThumbCache::SetResolution(CSize ThumbSize, DWORD ColorDepth)
{
	if (ThumbSize == m_ThumbSize && ColorDepth == m_ColorDepth)
		return;	// nothing to do
	if (!IsEmpty())
		Empty();
	m_ThumbSize = ThumbSize;
	m_ColorDepth = ColorDepth;
}

void CThumbCache::SetAt(LPCTSTR FileName, CThumb& Thumb)
{
	ASSERT(m_ThumbMap != NULL);
	m_ThumbMap->SetAt(FileName, Thumb);
	m_IsDirty = TRUE;
}

void CThumbCache::Empty()
{
	Flush();
	POSITION	pos = m_FolderMap.GetStartPosition();
	while (pos != NULL) {
		CString	Folder;
		FOLDER_INFO	Info;
		m_FolderMap.GetNextAssoc(pos, Folder, Info);
		delete Info.ThumbMap;
	}
	m_FolderMap.RemoveAll();
	m_Folder.Empty();
	m_ThumbMap = NULL;
	m_NumCached = 0;
}

void CThumbCache::Flush()
{
	if (m_CanShrink) {	// if thumbnail databases can shrink
		if (m_ThumbMap != NULL) {
			CThumbPos	pos = m_ThumbMap->PGetFirstAssoc();
			while (pos != NULL) {
				const CString&	FileName = pos->key;
				CPathStr	Path(m_Folder);
				Path.Append(FileName);
				if (!PathFileExists(Path)) {	// if source file doesn't exist
					m_ThumbMap->RemoveKey(FileName);	// delete thumbnail
					m_IsDirty = TRUE;	// thumbnail map was modified
				}
				pos = m_ThumbMap->PGetNextAssoc(pos);
			}
		}
	}
	if (m_IsDirty) {	// if thumbnail map was modified
		CString	Path = MakeDBPath(m_Folder);
		CFile	fp;
		ASSERT(m_ThumbMap != NULL);
		if (fp.Open(Path, CFile::modeCreate | CFile::modeWrite))
			Write(fp, *m_ThumbMap);	// write thumbnail database
		m_IsDirty = FALSE;	// ignore errors
	}
}

bool CThumbCache::FindLRUFolder(CString& Folder, FOLDER_INFO& Info) const
{
	clock_t	LastAccess = LONG_MAX;
	const CFolderMap::CPair	*LRUPos = NULL;
	const CFolderMap::CPair	*pos = m_FolderMap.PGetFirstAssoc();
	while (pos != NULL) {	// sequence is indeterminate, so examine all folders
		const FOLDER_INFO&	info = pos->value;
		if (info.ThumbMap == m_ThumbMap)
			continue;	// exclude current folder
		if (info.LastAccess < LastAccess) {	// if folder used less recently
			LastAccess = info.LastAccess;	// make it the new candidate
			LRUPos = pos;	// save folder's position
		}
		pos = m_FolderMap.PGetNextAssoc(pos);
	}
	if (LRUPos == NULL)
		return(FALSE);
	Folder = LRUPos->key;
	Info = LRUPos->value;
	return(TRUE);
}

void CThumbCache::MakeRoom(DWORD BytesNeeded)
{
	DWORD	m_ThumbBytes = m_ThumbSize.cx * m_ThumbSize.cy * (m_ColorDepth >> 3);
	DWORD	Request = BytesNeeded / m_ThumbBytes;	// number of thumbs requested
	DWORD	Limit = m_CacheLimit / m_ThumbBytes;	// maximum number of thumbs
	CString	Folder;
	FOLDER_INFO	Info;
	// remove least recently used folders until cache has enough room
	while (m_NumCached + Request > Limit && FindLRUFolder(Folder, Info)) {
		m_NumCached -= INT64TO32(Info.ThumbMap->GetCount());
		m_FolderMap.RemoveKey(Folder);
		delete Info.ThumbMap;
	}
}

void CThumbCache::SetFolder(LPCTSTR Folder)
{
	if (Folder == m_Folder)
		return;	// nothing to do
	Flush();
	FOLDER_INFO	Info;
	CFolderMap::CPair	*pos = m_FolderMap.PLookup(Folder);
	if (m_ThumbMap != NULL)	// add previous folder to total
		m_NumCached += INT64TO32(m_ThumbMap->GetCount());
	if (pos != NULL) {	// if folder is cached
		Info = pos->value;	// get folder info
		Info.LastAccess = clock();			// update folder's last access time
		pos->value = Info;	// set folder info
		// exclude this folder from total
		m_NumCached -= INT64TO32(Info.ThumbMap->GetCount());
		m_ThumbMap = Info.ThumbMap;	// make this folder current
		MakeRoom(0);	// if thumbs were added, cache may be too big
	} else {	// folder isn't cached
		m_ThumbMap = new CThumbMap;	// allocate new thumb map
		Info.ThumbMap = m_ThumbMap;
		Info.LastAccess = clock();
		CFile	fp;
		CString	Path = MakeDBPath(Folder);
		if (fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite)) {
			// file size is always bigger than bytes needed for thumbs because
			// it includes file names and other overhead, but it's close enough
			MakeRoom(static_cast<DWORD>(fp.GetLength()));	// make room in cache
			Read(fp, *Info.ThumbMap);	// read this folder's thumbnail database
		}
		m_FolderMap.SetAt(Folder, Info);	// add new folder to folder map
	}
	m_Folder = Folder;
}

CString	CThumbCache::MakeDBPath(LPCTSTR Folder) const
{
	CPathStr	Path(Folder);
	CString	Name;
	Name.Format(THUMB_DB_NAME, m_ThumbSize.cx, m_ThumbSize.cy, m_ColorDepth);
	Path.Append(Name);
	return(Path);
}

void CThumbCache::Serialize(CArchive& ar, CThumbMap& ThumbMap)
{
	if (ar.IsStoring()) {
		ar << ARCHIVE_SIG;
		ar << ARCHIVE_VERSION;
		ar << m_ThumbSize;
		ar << m_ColorDepth;
	} else {
		int	Sig, Version;
		ar >> Sig;
		ar >> Version;
		if (Sig != ARCHIVE_SIG || Version > ARCHIVE_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
		CSize	ThumbSize;
		DWORD	ColorDepth;
		ar >> ThumbSize;
		ar >> ColorDepth;
		if (ThumbSize != m_ThumbSize || ColorDepth != m_ColorDepth)
			AfxThrowArchiveException(CArchiveException::badIndex, ar.m_strFileName);
	}
	ThumbMap.Serialize(ar);
}

bool CThumbCache::DoIO(CFile& fp, DWORD ArchiveMode, CThumbMap& ThumbMap)
{
	TRY {
		CArchive	ar(&fp, ArchiveMode);
		Serialize(ar, ThumbMap);
	}
	CATCH(CArchiveException, e)
	{
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}
