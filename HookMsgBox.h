// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09aug07	initial version

        intercept all message boxes
 
*/

#ifndef CHOOKMSGBOX_INCLUDED
#define CHOOKMSGBOX_INCLUDED

#include <afxtempl.h>

class CHookMsgBox {
public:
// Construction
	CHookMsgBox();
	~CHookMsgBox();

// Types
	typedef bool (*MSGBOX_CALLBACK)(PVOID Cookie, LPCTSTR Caption, LPCTSTR MsgText);

// Attributes
	bool	IsInstalled() const;

// Operations
	bool	Install(MSGBOX_CALLBACK Callback, PVOID Cookie);
	bool	Remove();

protected:
// Types
	typedef struct tagCHILD_STATS {	// statistics about child controls
		int		Icons;		// number of icons
		int		Statics;	// number of non-icon statics
		int		Buttons;	// number of buttons
		int		Others;		// number of other child controls
	} CHILD_STATS;
	typedef CMap<HWND, HWND, LONG, LONG> CWndProcMap;

// Member data
	bool	m_IsInstalled;	// true if this instance has a hook installed

// Static members
	static	HHOOK	m_hHook;	// per-process CBT window hook
	static	MSGBOX_CALLBACK	m_MsgBoxCallback;	// user's callback function
	static	PVOID	m_Cookie;	// user-defined value passed to Install
	static	CWndProcMap	m_WndProcMap;	// original window procedures

// Helpers
	static	BOOL	CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
	static	bool	IsMessageBox(HWND hWnd);
	static	bool	IsDialogBox(HWND hWnd);
	static	void	GetText(HWND hWnd, CString& str);
	static	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static	LRESULT CALLBACK CbtProc(int nCode, WPARAM wParam, LPARAM lParam);
};

inline bool CHookMsgBox::IsInstalled() const
{
	return(m_IsInstalled);
}

#endif
