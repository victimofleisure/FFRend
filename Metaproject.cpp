// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version
		01		01feb07	add metaparameters
		02		09feb07	add CheckLinks
		03		23nov07	support Unicode
		04		07jun10	update for engine

        export/import project as a metaplugin DLL
 
*/

#include "stdafx.h"
#include "FFRend.h"
#include "Metaproject.h"
#include "ZInflate.h"
#include "PathStr.h"
#include "MainFrm.h"	// for GetAppPath
#include "imagehlp.h"	// for MakeSureDirectoryPathExists

#ifndef METAFFREND		// MetaFFRend DLL never zips, only unzips
#include "ZDeflate.h"
#endif

const LPCTSTR CMetaproject::META_BASE_NAME	= _T("MetaFFRend.dll");
const LPCSTR CMetaproject::META_BASE_TAG	= "BASEMETAFFREND01";
const LPCSTR CMetaproject::META_ID_STR		= "MetaFFRend";
const LPCTSTR CMetaproject::PLUGIN_FOLDER	= _T("FFRend\\Plugins\\");
// trailing backslash is required for MakeSureDirectoryPathExists

#define RK_META_BASE_FOLDER	_T("MetaBaseFolder")

bool CMetaproject::GetAppDataFolder(CString& Folder)
{
	LPTSTR	p = Folder.GetBuffer(MAX_PATH);
	bool	retc = SUCCEEDED(SHGetSpecialFolderPath(NULL, p, CSIDL_APPDATA, 0));
	Folder.ReleaseBuffer();
	return(retc);
}

void CMetaproject::ReportException(CException *e)
{
#ifdef METAFFREND	// MetaFFRend DLL can't show dialogs, so it logs errors instead
	TCHAR	msg[255];
	e->GetErrorMessage(msg, sizeof(msg), NULL);
	AfxMessageBox(msg);	// AfxMessageBox is a macro that appends msg to a log file
#else
	e->ReportError();	// display error dialog
#endif
}

bool CMetaproject::IsMetaplugin(LPCTSTR Path)
{
	CFile	fp;
	if (!fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite))
		return(FALSE);
	METAPLUGIN_HEADER	hdr;
	TRY {
		fp.Seek(-int(sizeof(hdr)), CFile::end);
		fp.Read(&hdr, sizeof(hdr));
		if (memcmp(hdr.Id, META_ID_STR, sizeof(hdr.Id)))
			return(FALSE);
	}
	CATCH(CFileException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

void CMetaproject::GetPluginList(CStringArray& List)
{
	CMap<CString, LPCTSTR, int, int&> DllKey;	// for eliminating duplicates
	int	dummy = 0;	// we only use key, not value
	int	plugs = m_PlugInfo.GetSize();
	for (int i = 0; i < plugs; i++) {
		CFFPlugInfo&	info = m_PlugInfo[i];
		if (!info.m_Path.IsEmpty()) {	// skip empty slots
			LPCTSTR	DllName = PathFindFileName(info.m_Path);
			if (!DllKey.Lookup(DllName, dummy)) {	// skip duplicates
				DllKey.SetAt(DllName, dummy);	// add plugin name to hash
				List.Add(DllName);
			}
		}
	}
}

#ifndef METAFFREND	// MetaFFRend DLL never exports, only imports

bool CMetaproject::SetVideoCaps()
{
	CMetaplugin::CAPS&	Caps = m_Metaplugin.m_Caps;
	Caps.Video16Bit = TRUE;	// assume success
	Caps.Video24Bit = TRUE;
	Caps.Video32Bit = TRUE;
	CFFEngine&	eng = theApp.GetEngine();
	// stop engine before accessing FreeFrame instance directly via GetFFPlugin
	STOP_ENGINE(eng);
	int	plugs = eng.GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	plex = eng.GetPlugin(PlugIdx);
		const CFFPlugin& pl = plex.GetFFPlugin();
		if (pl.GetPluginCaps(FF_CAP_16BITVIDEO) != FF_TRUE)
			Caps.Video16Bit = FALSE;
		if (pl.GetPluginCaps(FF_CAP_24BITVIDEO) != FF_TRUE)
			Caps.Video24Bit = FALSE;
		if (pl.GetPluginCaps(FF_CAP_32BITVIDEO) != FF_TRUE)
			Caps.Video32Bit = FALSE;
	}
	return(TRUE);
}

bool CMetaproject::Export(LPCTSTR Path)
{
	CPathStr	MetaBasePath(AfxGetApp()->GetProfileString(
		REG_SETTINGS, RK_META_BASE_FOLDER, NULL));
	if (MetaBasePath.IsEmpty())	// // if registry doesn't specify a folder
		MetaBasePath = theApp.GetAppFolder();	 // use application folder
	MetaBasePath.Append(META_BASE_NAME);
	CString	msg;
	if (!PathFileExists(MetaBasePath)) {
		AfxFormatString1(msg, IDS_META_BASE_NOT_FOUND, META_BASE_NAME);
		AfxMessageBox(msg);
		return(FALSE);
	}
	if (!CopyFile(MetaBasePath, Path, FALSE)) {
		AfxFormatString1(msg, IDS_META_CANT_CREATE_PLUGIN, Path);
		AfxMessageBox(msg);
		return(FALSE);
	}
	CFile	fp;
	if (!fp.Open(Path, CFile::modeReadWrite)) {
		AfxFormatString1(msg, IDS_META_CANT_OPEN, Path);
		AfxMessageBox(msg);
		return(FALSE);
	}
	if (!SetVideoCaps())	// specify which video mode(s) our plugins have in common
		return(FALSE);
	TRY {
		// verify base metaplugin; look for identification tag at end of DLL
		fp.Seek(-META_BASE_TAG_LEN, CFile::end);
		DWORD	DllEnd = static_cast<DWORD>(fp.GetPosition());	// limit to 4GB
		char	BaseTag[META_BASE_TAG_LEN];
		fp.Read(BaseTag, META_BASE_TAG_LEN);
		if (memcmp(BaseTag, META_BASE_TAG, META_BASE_TAG_LEN)) {
			AfxFormatString1(msg, IDS_META_BAD_FORMAT, Path);
			AfxMessageBox(msg);
			return(FALSE);
		}
		fp.Seek(-META_BASE_TAG_LEN, CFile::end);	// overwrite base tag
		if (m_Metaplugin.m_IsEmbedded) {
			if (!DeflatePlugs(fp)) {	// compress plugins into metaplugin
				AfxMessageBox(IDS_META_CANT_DEFLATE);
				return(FALSE);
			}
		} else {	// don't assume we're freshly constructed
			m_Metaplugin.m_EmbDllInfo.RemoveAll();
			m_Metaplugin.m_ComprLength = 0;
		}
		CArchive	ar(&fp, CArchive::store);
		ar.m_strFileName = fp.GetFileName();
		m_Version = 5;	// use V1 project format for backwards compatibility
		Serialize(ar);	// store project data in metaplugin
		ar.Flush();
		// add header to end of metaplugin
		METAPLUGIN_HEADER	hdr;
		memcpy(hdr.Id, META_ID_STR, sizeof(hdr.Id));
		hdr.Version = META_DATA_VERSION;
		hdr.DataOfs = DllEnd + m_Metaplugin.m_ComprLength;
		fp.Write(&hdr, sizeof(hdr));
	}
	CATCH(CArchiveException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	CATCH(CFileException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

bool CMetaproject::DeflatePlugs(CFile& OutFile)
{
	CZDeflate	zs(OutFile, COMPRESS_BUF_SIZE);
	if (!zs.Init(Z_DEFAULT_COMPRESSION))	// initialize compressor
		return(FALSE);
	m_Metaplugin.m_EmbDllInfo.RemoveAll();
	CMap<CString, LPCTSTR, int, int&> DllKey;	// for eliminating duplicate DLLs
	int	dummy = 0;	// we only use key, not value
	CByteArray	InpBuf;
	int	plugs = m_PlugInfo.GetSize();
	for (int i = 0; i < plugs; i++) {
		CFFPlugInfo&	info = m_PlugInfo[i];
		if (!info.m_Path.IsEmpty()) {	// skip empty slots
			LPCTSTR	DllName = PathFindFileName(info.m_Path);
			if (!DllKey.Lookup(DllName, dummy)) {	// skip duplicate DLLs
				DllKey.SetAt(DllName, dummy);	// add DLL's name to hash
				CFile	dll;
				if (!dll.Open(info.m_Path, CFile::modeRead | CFile::shareDenyWrite))
					return(FALSE);
				DWORD	len = static_cast<DWORD>(dll.GetLength());	// limit to 4GB
				CEmbDllInfo	edi;	// embedded DLL info
				edi.m_Name = DllName;
				edi.m_Length = len;
				CFileStatus	stat;
				if (dll.GetStatus(stat)) {	// get DLL's file times
					edi.m_Created = stat.m_ctime;
					edi.m_LastWrite = stat.m_mtime;
				}
				m_Metaplugin.m_EmbDllInfo.Add(edi);	// add info to list
				InpBuf.SetSize(len);	// allocate input buffer
				dll.Read(InpBuf.GetData(), len);	// read DLL into input buffer
				if (!zs.Write(InpBuf.GetData(), len)) {	// compress from buffer to OutFile
					CString	msg;
					msg.Format(IDS_META_ZLIB_ERROR, zs.GetLastError());
					AfxMessageBox(msg);
					return(FALSE);
				}
			}
		}
	}
	if (!zs.End())
		return(FALSE);
	m_Metaplugin.m_ComprLength = zs.GetComprLen();
	return(TRUE);
}

#endif	// !METAFFREND

bool CMetaproject::Import(LPCTSTR Path, bool UnpackEmbPlugs)
{
	CString	msg;
	CFile	fp;
	if (!fp.Open(Path, CFile::modeRead | CFile::shareDenyWrite)) {
		AfxFormatString1(msg, IDS_META_CANT_OPEN, Path);
		AfxMessageBox(msg);
		return(FALSE);
	}
	METAPLUGIN_HEADER	hdr;
	TRY {
		fp.Seek(-int(sizeof(hdr)), CFile::end);
		fp.Read(&hdr, sizeof(hdr));
		if (memcmp(hdr.Id, META_ID_STR, sizeof(hdr.Id))) {	// verify metaplugin header
			AfxFormatString1(msg, IDS_META_BAD_FORMAT, Path);
			AfxMessageBox(msg);
			return(FALSE);
		}
		fp.Seek(hdr.DataOfs, CFile::begin);
		CArchive	ar(&fp, CArchive::load);
		ar.m_strFileName = fp.GetFileName();
		Serialize(ar);	// load project data from metaplugin
		if (m_Metaplugin.m_IsEmbedded && UnpackEmbPlugs) {
			fp.Seek(hdr.DataOfs - m_Metaplugin.m_ComprLength, CFile::begin);
			if (!InflatePlugs(fp)) {	// uncompress plugins from metaplugin
				AfxMessageBox(IDS_META_CANT_INFLATE);
				return(FALSE);
			}
		}
	}
	CATCH(CArchiveException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	CATCH(CFileException, e)
	{
		ReportException(e);
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
}

// MakeSureDirectoryPathExists doesn't support Unicode; SHCreateDirectoryEx
// is a reasonable substitute, but our version of the SDK doesn't define it
#if defined(UNICODE) && !defined(SHCreateDirectoryEx)
int WINAPI SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa)
{
	int	retc = ERROR_INVALID_FUNCTION;
	typedef int (WINAPI* lpfnSHCreateDirectoryExW)(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa);
	HMODULE hShell = LoadLibrary(_T("shell32.dll"));
	lpfnSHCreateDirectoryExW lpfn = NULL;
	if (hShell) {
		lpfn = (lpfnSHCreateDirectoryExW)GetProcAddress(hShell, "SHCreateDirectoryExW");
		if (lpfn)
			retc = lpfn(hwnd, pszPath, psa);
		FreeLibrary(hShell);
	}
	return(retc);
}
#define SHCreateDirectoryEx SHCreateDirectoryExW
#endif

bool CMetaproject::InflatePlugs(CFile& InpFile)
{
	CPathStr	PluginFolder;
	if (!GetAppDataFolder(PluginFolder)) {
		AfxMessageBox(IDS_META_NO_APP_FOLDER);
		return(FALSE);
	}
	PluginFolder.Append(PLUGIN_FOLDER);
	if (!PathFileExists(PluginFolder)) {	// test using shlwapi, it's much faster
#ifdef UNICODE
		if (!SHCreateDirectoryEx(NULL, PluginFolder, NULL) != ERROR_SUCCESS) {
#else
		if (!MakeSureDirectoryPathExists(PluginFolder)) {	// this function is SLOW
#endif
			CString	msg;
			AfxFormatString1(msg, IDS_META_CANT_CREATE_DIR, PluginFolder);
			AfxMessageBox(msg);
			return(FALSE);
		}
	}
	CZInflate	zs(InpFile, COMPRESS_BUF_SIZE);
	if (!zs.Init())	// initialize compressor
		return(FALSE);
	CByteArray	OutBuf;
	int	dlls = m_Metaplugin.m_EmbDllInfo.GetSize();
	// find out which DLLs have been unpacked already
	bool	AllUnpacked = TRUE;
	CDWordArray	IsUnpacked;
	CStringArray	DllPath;
	IsUnpacked.SetSize(dlls);
	int	i;
	for (i = 0; i < dlls; i++) {
		CPathStr	Path(PluginFolder);
		Path.Append(m_Metaplugin.m_EmbDllInfo[i].m_Name);
		BOOL	found = PathFileExists(Path);
		IsUnpacked[i] = found;
		if (!found)
			AllUnpacked = FALSE;
	}
	if (!AllUnpacked) {	// if all DLLs are unpacked, we can skip inflate entirely
		for (i = 0; i < dlls; i++) {
			const CEmbDllInfo&	edi = m_Metaplugin.m_EmbDllInfo[i];
			int	len = edi.m_Length;
			OutBuf.SetSize(len);	// allocate output buffer
			if (!zs.Read(OutBuf.GetData(), len)) {	// uncompress from InpFile to buffer
				CString	msg;
				msg.Format(IDS_META_ZLIB_ERROR, zs.GetLastError());
				AfxMessageBox(msg);
				return(FALSE);
			}
			if (!IsUnpacked[i]) {	// if this DLL hasn't been unpacked yet
				CFile	dll;
				CPathStr	Path(PluginFolder);
				Path.Append(edi.m_Name);
				if (!dll.Open(Path, CFile::modeWrite | CFile::modeCreate))	// create DLL
					return(FALSE);
				dll.Write(OutBuf.GetData(), len);	// write output buffer to DLL
				dll.Close();	// must close file before calling SetStatus
				if (edi.m_Created > 0) {
					CFileStatus	stat;
					if (dll.GetStatus(stat)) {	// get DLL's current status
						stat.m_ctime = edi.m_Created;
						stat.m_mtime = edi.m_LastWrite;
						dll.SetStatus(Path, stat);	// set DLL's file times
					}
				}
			}
		}
	}
	// redirect project paths to unpack folder
	int	plugs = m_PlugInfo.GetSize();
	for (i = 0; i < plugs; i++) {
		CFFPlugInfo&	info = m_PlugInfo[i];
		if (!info.m_Path.IsEmpty()) {
			CPathStr	Path(PluginFolder);
			Path.Append(PathFindFileName(info.m_Path));
			info.m_Path = Path;
		}
	}
	return(TRUE);
}

bool CMetaproject::GetModuleFolder(CString& Folder)
{
	LPTSTR	p = Folder.GetBuffer(MAX_PATH);
	bool	retc = GetModuleFileName(AfxGetApp()->m_hInstance, p, MAX_PATH) != 0;
	if (retc)
		PathRemoveFileSpec(p);
	Folder.ReleaseBuffer();
	return(retc);
}

bool CMetaproject::GetEnvironmentVar(LPCTSTR Name, CString& Value)
{
	DWORD	len = GetEnvironmentVariable(Name, NULL, 0);	// get length first
	if (!len)	// if environment variable was not found
		return(FALSE);
	LPTSTR	p = Value.GetBuffer(len);
	GetEnvironmentVariable(Name, p, len);	// now get value
	Value.ReleaseBuffer();
	return(TRUE);
}

void CMetaproject::ParsePathList(const CString& Path, CStringArray& PathList)
{
	int	idx = 0;
	PathList.RemoveAll();
	while (idx < Path.GetLength()) {
		CString	s = Path.Mid(idx).SpanExcluding(_T(";"));
		PathList.Add(s);
		idx += s.GetLength() + 1;
	}
}

bool CMetaproject::CheckLinks()
{
	static const LPCTSTR	METAFFREND_PATH = _T("METAFFREND_PATH");
	CString	BadLinks, DllFolder, AppDataFolder, EnvVarPath;
	CStringArray	EnvVarPathList;
	bool	TryDllFolder		= TRUE;
	bool	TryAppDataFolder	= TRUE;
	bool	TryEnvVarPath		= TRUE;
	int	plugs = m_PlugInfo.GetSize();
	for (int i = 0; i < plugs; i++) {
		CFFPlugInfo&	info = m_PlugInfo[i];
		if (!info.m_Path.IsEmpty()) {	// skip empty slots
			//
			// try the following paths in order:
			// 1. absolute path as specified in project data
			// 2. folder that metaplugin DLL was loaded from
			// 3. profile's Application Data + PLUGIN_FOLDER
			// 4. METAFFREND_PATH environment variable paths
			//
			if (!PathFileExists(info.m_Path)) {	// if absolute path doesn't work
				LPCTSTR	PluginName = PathFindFileName(info.m_Path);
				CPathStr	Path;
				bool	Fixed = FALSE;	// assume failure
				//
				// try folder that metaplugin DLL was loaded from
				//
				if (TryDllFolder) {	// unless we gave up on DLL folder
					if (DllFolder.IsEmpty()) {	// if we didn't get folder yet
						if (!GetModuleFolder(DllFolder))	// if we can't get it
							TryDllFolder = FALSE;	// don't try anymore
					}
					Path = DllFolder;
					Path.Append(PluginName);
					Fixed = PathFileExists(Path) != 0;	// if plugin exists, we're done
				}
				if (!Fixed) {	// if DLL folder didn't work
					//
					// try profile's Application Data + PLUGIN_FOLDER
					//
					if (TryAppDataFolder) {	// unless we gave up on App Data folder
						if (AppDataFolder.IsEmpty()) {	// if we didn't get folder yet
							if (!GetAppDataFolder(AppDataFolder))	// if we can't get it
								TryAppDataFolder = FALSE;	// don't try anymore
						}
						Path = AppDataFolder;
						Path.Append(PLUGIN_FOLDER);
						Path.Append(PluginName);
						Fixed = PathFileExists(Path) != 0;	// if plugin exists, we're done
					}
					if (!Fixed) {	// if App Data folder didn't work
						//
						// try METAFFREND_PATH environment variable paths
						//
						if (TryEnvVarPath) {	// unless we gave up on environment var
							if (EnvVarPath.IsEmpty()) {	// if we didn't get path yet
								if (GetEnvironmentVar(METAFFREND_PATH, EnvVarPath))
									ParsePathList(EnvVarPath, EnvVarPathList);	// parse it
								else	// environment variable not found
									TryEnvVarPath = FALSE;	// don't try anymore
							}
							int	PathCount = EnvVarPathList.GetSize();
							for (int i = 0; i < PathCount; i++) {	// for each path
								Path = EnvVarPathList[i];
								Path.Append(PluginName);
								if (PathFileExists(Path)) {	// if plugin exists, we're done
									Fixed = TRUE;
									break;
								}
							}
						}
					}
				}
				if (Fixed)	// if link was repaired
					info.m_Path = Path;	// update path in project data
#ifdef METAFFREND		// MetaFFRend DLL only; app displays missing files dialog instead
				else {
					if (BadLinks.Find(PluginName) < 0) {	// eliminate duplicate names
						if (!BadLinks.IsEmpty())
							BadLinks += ", ";	// add separator
						BadLinks += PluginName;	// add name to broken links
						info.m_Path.Empty();	// prevent further error messages
					}
				}
#endif
			}
		}
	}
#ifdef METAFFREND		// MetaFFRend DLL only; app displays missing files dialog instead
	if (!BadLinks.IsEmpty()) {	// if there were broken links
		CString	msg;
		AfxFormatString1(msg, IDS_META_MISSING_PLUGINS, BadLinks);
		AfxMessageBox(msg);
		return(FALSE);
	}
#endif
	return(TRUE);
}
