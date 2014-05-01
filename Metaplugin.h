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
		02		25dec07	add metaparameter groups
		03		27apr10	rename plugin index to slot index

        metaplugin data
 
*/

#ifndef CMETAPLUGIN_INCLUDED
#define CMETAPLUGIN_INCLUDED

#include <afxtempl.h>
#include "EmbDllInfo.h"
#include "MetaparmArray.h"

class CMetaplugin : public WObject {
public:
// Construction
	CMetaplugin();
	CMetaplugin(const CMetaplugin& Plug);
	CMetaplugin& operator=(const CMetaplugin& Plug);

// Types
	typedef struct tagCAPS {
		bool	Video16Bit;		// true if plugin supports 16-bit color
		bool	Video24Bit;		// true if plugin supports 24-bit color
		bool	Video32Bit;		// true if plugin supports 32-bit color
		bool	ProcFrameCopy;	// true if plugin supports ProcessFrameCopy
		BYTE	MinInpFrames;	// minimum number of input frames plugin supports
		BYTE	MaxInpFrames;	// maximum number of input frames plugin supports
		BYTE	CopyOrInPlace;	// copy or in-place preference
		BYTE	Reserved;		// for future enhancements
	} CAPS;

// Public data
	// don't forget to add new members to ctor, Copy, and Serialize
	CString m_PluginName;		// plugin name, 16 characters maximum
	CString	m_UniqueID;			// plugin's unique ID, 4 characters
	CString m_Description;		// a description of the plugin
	CString m_AboutText;		// author and license information
	UINT	m_PluginType;		// 0 = effect, 1 = source
	CAPS	m_Caps; 			// plugin capabilities
	UINT	m_PluginMajorVersion;	// number before decimal point
	UINT	m_PluginMinorVersion;	// number after decimal point
	CDWordArray m_InpTargetIdx; // for each input frame, index of target plugin
	BOOL	m_IsEmbedded;		// true if plugins are embedded in metaplugin
	DWORD	m_ComprLength;		// compressed length of all embedded DLLs
	CEmbDllInfoArray	m_EmbDllInfo;	// array of info about each embedded DLL
	CMetaparmArray	m_Metaparm;	// array of info about each metaparameter
	
// Operations
	void	Serialize(CArchive& ar, int Version);
	void	Write(CStdioFile& fp) const;
	bool	Read(CStdioFile& fp);

protected:
// Helpers
	void	Copy(const CMetaplugin& Plug);
};

inline CMetaplugin::CMetaplugin(const CMetaplugin& Plug)
{
	Copy(Plug);
}

inline CMetaplugin& CMetaplugin::operator=(const CMetaplugin& Plug)
{
	Copy(Plug);
	return(*this);
}

#endif
