// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		08mar10	initial version
        01		21nov11	add playlist
		02		23jan12	add app update info message
		03		13apr12	add engine uses COM flag

		global definitions and inlines

*/

#pragma once

#pragma warning(disable : 4100)	// unreferenced formal parameter

// minimal base for non-CObject classes
#include "WObject.h"

// file extensions
#define PROJECT_EXT		_T(".ffp")
#define PLUGIN_EXT		_T(".dll")
#define PLAYLIST_EXT	_T(".ffl")
#define HISTORY_EXT		_T(".hst")
#define BITMAP_EXT		_T(".bmp")
#define AVI_EXT			_T(".avi")
#define AVS_EXT			_T(".avs")	// AviSynth script
#define MPG_EXT			_T(".mpg")

// registry strings
#define REG_SETTINGS		_T("Settings")

// key status bits for GetAsyncKeyState
#define GKS_TOGGLED			0x0001
#define GKS_DOWN			0x8000

// trig macros
#define PI 3.141592653589793
#define DTR(x) (x * PI / 180)	// degrees to radians
#define RTD(x) (x * 180 / PI)	// radians to degrees

// clamp a value to a range
#define CLAMP(x, lo, hi) (min(max((x), (lo)), (hi)))

// trap bogus default case in switch statement
#define NODEFAULTCASE	ASSERT(0)

// load string from resource via temporary object
#define LDS(x) CString((LPCTSTR)x)

// optimized FPU rounding
inline int round(double x)
{
#ifdef _WIN64
	return(int(x > 0 ? x + 0.5 : x - 0.5));
#else
	int		temp;
	__asm {
		fld		x		// load real
		fistp	temp	// store integer and pop stack
	}
	return(temp);
#endif
}

// optimized FPU truncation
inline int trunc(double x)
{
#ifdef _WIN64
	return(int(x));
#else
	int		temp;
	short	cw, chop;
	__asm {
		fstcw	cw		// save control word
		mov		ax, cw
		or		ax, 0c00h	// set rounding mode to chop
		mov		chop, ax
		fldcw	chop	// load chop control word
		fld		x		// load real
		fistp	temp	// store integer and pop stack
		fldcw	cw		// restore control word
	}
	return(temp);
#endif
}

enum {	// user windows messages
	UWM_FIRST = WM_APP,
	UWM_ENGINESTALL,	// wParam: CEngineThread ptr, lParam: none
	UWM_GRAPHDONE,		// wParam: LPTSTR picture path, lParam: none
	UWM_DLGBARUPDATE,	// wParam: none, lParam: none
	UWM_MULTIFILESEL,	// wParam: CMultiFileDlg pointer, lParam: none
	UWM_FFROWEDIT,		// wParam: row index, lParam: control ID
	UWM_TABCTRLDRAG,	// wParam: tab index, lParam: control ID
	UWM_EXTRACTTHUMB,	// wParam: thumbnail HBITMAP, lParam: THUMB_RESULT*
	UWM_HANDLEDLGKEY,	// wParam: MSG pointer, lParam: none
	UWM_MIDIROWEDIT,	// wParam: row index, lParam: control ID
	UWM_MIDIROWSEL,		// wParam: row index, lParam: control ID
	UWM_MIDIIN,			// wParam: incoming MIDI message, lParam: none
	UWM_FOLDERCHANGE,	// wParam: pane index, lParam: folder index
	UWM_ENDRECORD,		// wParam: none, lParam: none
	UWM_APPUPDATEINFO,	// wParam: CAppUpdateInfo ptr, lParam: none
};

// atof's generic-text wrapper is missing in MFC 6
#ifndef _tstof
#ifdef UNICODE
#define _tstof(x) _tcstod(x, NULL)
#else
#define _tstof(x) atof(x)
#endif
#endif

#if _MFC_VER < 0x0800
#define genericException generic	// generic was deprecated in .NET 2005
#endif

inline void StoreBool(CArchive& ar, bool flag)
{
#if _MFC_VER >= 0x0700
	ar << flag;
#else	// MFC 6 CArchive doesn't support bool
	BYTE	byte = flag;
	ar << byte;
#endif
}

inline void LoadBool(CArchive& ar, bool& flag)
{
#if _MFC_VER >= 0x0700
	ar >> flag;
#else	// MFC 6 CArchive doesn't support bool
	BYTE	byte;
	ar >> byte;
	flag = byte != 0;
#endif
}

#if _MSC_VER < 1300
#define ACTIVATEAPPTASK HTASK
#else
#define ACTIVATEAPPTASK DWORD
#endif

#ifdef _WIN64
typedef INT_PTR W64INT;
typedef UINT_PTR W64UINT;
#define INT64TO32(x) static_cast<int>(x)
#define UINT64TO32(x) static_cast<UINT>(x)
#define GCL_HBRBACKGROUND GCLP_HBRBACKGROUND
#else
typedef int W64INT;
typedef UINT W64UINT;
#define INT64TO32(x) x
#define UINT64TO32(x) x
#endif

#ifdef _DEBUG
// console natter settings
//#define ENGINE_NATTER
//#define SPIDER_NATTER
//#define RENDERER_NATTER
//#define MIDI_NATTER
//#define METAGROUP_NATTER
//#define UNDO_NATTER TRUE
//#define ENGINE_TAP_NATTER
#endif

#define DISABLE_PRINTF 0
#if DISABLE_PRINTF
#define printf sizeof
#undef _tprintf
#define _tprintf sizeof
#endif

#define ENGINE_USES_COM TRUE
