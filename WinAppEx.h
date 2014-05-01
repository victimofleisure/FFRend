// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
        01      13jan12	add GetTempPath and GetAppPath

        enhanced application
 
*/

#if !defined(AFX_WINAPPEX_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_)
#define AFX_WINAPPEX_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// CWinAppEx:
// See WinAppEx.cpp for the implementation of this class
//

#include "Persist.h"

class CWinAppEx : public CWinApp
{
public:
// Attributes
	bool	GetTempPath(CString& Path);
	bool	GetTempFileName(CString& Path, LPCTSTR Prefix = NULL, UINT Unique = 0);
	bool	GetAppDataFolder(CString& Folder);
	static	CString GetAppPath();
	static	CString GetAppFolder();
	static	bool	GetIconSize(HICON Icon, CSize& Size);
	static	bool	GetComputerName(CString& Name);

// Operations
	void	UpdateAllViews(CView* pSender, LPARAM lHint = 0L, CObject* pHint = NULL);
	static	bool	DeleteDirectory(LPCTSTR lpszDir, bool bAllowUndo = FALSE);
	static	void	EnableChildWindows(CWnd& Wnd, bool Enable);
	static	CString	GetTitleFromPath(LPCTSTR Path);
	bool	CreateFolder(LPCTSTR Path);

// Persistence
	int		RdRegInt(LPCTSTR Key, int Default = 0);
	void	WrRegInt(LPCTSTR Key, int Value);
	bool	RdRegBool(LPCTSTR Key, bool Default = FALSE);
	void	WrRegBool(LPCTSTR Key, bool Value);
	CString	RdRegString(LPCTSTR Key, LPCTSTR Default = NULL);
	void	WrRegString(LPCTSTR Key, CString Value);
	float	RdRegFloat(LPCTSTR Key, float Default = 0);
	void	WrRegFloat(LPCTSTR Key, float Value);
	double	RdRegDouble(LPCTSTR Key, double Default = 0);
	void	WrRegDouble(LPCTSTR Key, double Value);
	// templates for reading/writing structs
	template<class T>
	static	T		RdRegStruct(LPCTSTR Key, T& Value, T Default) {
		DWORD	Size = sizeof(T);
		if (CPersist::GetBinary(REG_SETTINGS, Key, &Value, &Size))
			return(Value);
		return(Default);
	}
	template<class T>	
	static	BOOL	WrRegStruct(LPCTSTR Key, T Value) {
		return(CPersist::WriteBinary(REG_SETTINGS, Key, &Value, sizeof(T)));
	}
	// for these reads, value defaults to itself
	void	RdReg2Int(LPCTSTR Key, int& Value);
	void	RdReg2UInt(LPCTSTR Key, UINT& Value);
	void	RdReg2Long(LPCTSTR Key, long& Value);
	void	RdReg2ULong(LPCTSTR Key, ULONG& Value);
	void	RdReg2Bool(LPCTSTR Key, bool& Value);
	void	RdReg2String(LPCTSTR Key, CString& Value);
	void	RdReg2Float(LPCTSTR Key, float& Value);
	void	RdReg2Double(LPCTSTR Key, double& Value);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinAppEx)
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWinAppEx)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
// Data members

// Helpers
	static	void	FixMFCDotBitmap();
	static	HBITMAP	GetMFCDotBitmap();
};

inline int CWinAppEx::RdRegInt(LPCTSTR Key, int Default)
{
	return(GetProfileInt(REG_SETTINGS, Key, Default));
}

inline void CWinAppEx::WrRegInt(LPCTSTR Key, int Value)
{
	WriteProfileInt(REG_SETTINGS, Key, Value);
}

inline bool CWinAppEx::RdRegBool(LPCTSTR Key, bool Default)
{
	return(GetProfileInt(REG_SETTINGS, Key, Default) != 0);
}

inline void CWinAppEx::WrRegBool(LPCTSTR Key, bool Value)
{
	WriteProfileInt(REG_SETTINGS, Key, Value);
}

inline CString CWinAppEx::RdRegString(LPCTSTR Key, LPCTSTR Default)
{
	return(GetProfileString(REG_SETTINGS, Key, Default));
}

inline void CWinAppEx::WrRegString(LPCTSTR Key, CString Value)
{
	WriteProfileString(REG_SETTINGS, Key, Value);
}

inline float CWinAppEx::RdRegFloat(LPCTSTR Key, float Default)
{
	return(CPersist::GetFloat(REG_SETTINGS, Key, Default));
}

inline void CWinAppEx::WrRegFloat(LPCTSTR Key, float Value)
{
	CPersist::WriteFloat(REG_SETTINGS, Key, Value);
}

inline double CWinAppEx::RdRegDouble(LPCTSTR Key, double Default)
{
	return(CPersist::GetDouble(REG_SETTINGS, Key, Default));
}

inline void CWinAppEx::WrRegDouble(LPCTSTR Key, double Value)
{
	CPersist::WriteDouble(REG_SETTINGS, Key, Value);
}

inline void CWinAppEx::RdReg2Int(LPCTSTR Key, int& Value)
{
	Value = RdRegInt(Key, Value);
}

inline void CWinAppEx::RdReg2UInt(LPCTSTR Key, UINT& Value)
{
	Value = RdRegInt(Key, Value);
}

inline void CWinAppEx::RdReg2Long(LPCTSTR Key, long& Value)
{
	Value = RdRegInt(Key, Value);
}

inline void CWinAppEx::RdReg2ULong(LPCTSTR Key, ULONG& Value)
{
	Value = RdRegInt(Key, Value);
}

inline void CWinAppEx::RdReg2Bool(LPCTSTR Key, bool& Value)
{
	Value = RdRegBool(Key, Value);
}

inline void CWinAppEx::RdReg2String(LPCTSTR Key, CString& Value)
{
	Value = RdRegString(Key, Value);
}

inline void CWinAppEx::RdReg2Float(LPCTSTR Key, float& Value)
{
	Value = RdRegFloat(Key, Value);
}

inline void CWinAppEx::RdReg2Double(LPCTSTR Key, double& Value)
{
	Value = RdRegDouble(Key, Value);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINAPPEX_H__ED0ED84D_AA43_442C_85CD_A7FA518EBF90__INCLUDED_)
