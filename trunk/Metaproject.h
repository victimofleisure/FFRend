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

#ifndef CMETAPROJECT_INCLUDED
#define CMETAPROJECT_INCLUDED

#include <afxtempl.h>
#include "FFProject.h"

class CMetaproject : public CFFProject {
public:
// Attributes
	void	GetPluginList(CStringArray& List);
	static	bool	IsMetaplugin(LPCTSTR Path);
	static	bool	GetAppDataFolder(CString& Folder);

// Operations
	bool	Export(LPCTSTR Path);
	bool	Import(LPCTSTR Path, bool UnpackEmbPlugs = TRUE);
	bool	CheckLinks();

protected:
// Types
	typedef struct tagMETAPLUGIN_HEADER {
		char	Id[10];		// must contain characters "MetaFFRend"
		WORD	Version;	// version number of metaplugin data format
		DWORD	DataOfs;	// offset of metaplugin data from start of DLL file
	} METAPLUGIN_HEADER;

// Constants
	enum {
		META_BASE_TAG_LEN = 16,		// length of base metaplugin's identifying tag
		META_DATA_VERSION = 1,		// version number of metaplugin data format
		COMPRESS_BUF_SIZE = 0x4000	// size of compress/uncompress buffer in bytes
	};
	static const LPCTSTR	META_BASE_NAME;	// filename of base metaplugin DLL
	static const LPCSTR		META_BASE_TAG;	// verifies DLL is base metaplugin
	static const LPCSTR		META_ID_STR;	// verifies DLL is exported metaplugin
	static const LPCTSTR	PLUGIN_FOLDER;	// embedded plugins are unpacked here

// Helpers
	bool	SetVideoCaps();
	bool	DeflatePlugs(CFile& OutFile);
	bool	InflatePlugs(CFile& InpFile);
	static	void	ReportException(CException *e);
	static	bool	GetModuleFolder(CString& Folder);
	static	void	ParsePathList(const CString& Path, CStringArray& PathList);
	static	bool	GetEnvironmentVar(LPCTSTR Name, CString& Value);
};

#endif
