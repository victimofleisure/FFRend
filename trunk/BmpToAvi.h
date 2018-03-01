// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      18nov08	initial version

        DLL wrapper for DirectShow BMP to AVI converter

*/

#ifndef CBMPTOAVIEXP_INCLUDED
#define CBMPTOAVIEXP_INCLUDED

#include "VideoComprState.h"

#ifdef MAKEDLL		// if this symbol is defined, export the object
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport) 	
#endif

#ifndef MAKEDLL
typedef struct tagBMPTOAVI_PARMS {
	int		Width;		// frame width, in pixels
	int		Height;		// frame height, in pixels
	int		BitCount;	// number of bits per pixel
	float	FrameRate;	// frame rate, in frames per second
} BMPTOAVI_PARMS;
#else
#include "BmpToAviCo.h"
#endif

DLLEXPORT PVOID	BmpToAviCreate();
DLLEXPORT void	BmpToAviDestroy(PVOID Inst);
DLLEXPORT bool	BmpToAviOpen(PVOID Inst, const BMPTOAVI_PARMS& Parms, LPCTSTR Path, bool ShowComprDlg);
DLLEXPORT bool	BmpToAviClose(PVOID Inst);
DLLEXPORT bool	BmpToAviAddFrame(PVOID Inst, HBITMAP hBitmap);
DLLEXPORT bool	BmpToAviIsCompressed(PVOID Inst);
DLLEXPORT int	BmpToAviGetLastError(PVOID Inst, HRESULT *hr);
DLLEXPORT void	BmpToAviGetLastErrorString(PVOID Inst, LPTSTR pErr, LPTSTR pDSErr, DWORD MaxErrLen);
DLLEXPORT void	BmpToAviGetComprState(PVOID Inst, LPTSTR Name, DWORD MaxNameLen, CVideoComprState::PARMS& Parms, CByteArray& DlgState);
DLLEXPORT void	BmpToAviSetComprState(PVOID Inst, LPCTSTR Name, const CVideoComprState::PARMS& Parms, const CByteArray& DlgState);

class CBmpToAvi : public WObject {
public:
// Construction
	CBmpToAvi();
	~CBmpToAvi();

// Operations
	bool	Open(const BMPTOAVI_PARMS& Parms, LPCTSTR Path, bool ShowComprDlg);
	bool	Close();
	bool	AddFrame(HBITMAP hBitmap);

// Attributes
	bool	IsCompressed() const;
	int		GetLastError(HRESULT *hr = NULL) const;
	void	GetLastErrorString(CString& Err, CString& DSErr) const;
	void	GetComprState(CVideoComprState& State) const;
	void	SetComprState(const CVideoComprState& State);

protected:
// Data members
	PVOID	m_Inst;	// pointer to CBmpToAvi instance
};

inline CBmpToAvi::CBmpToAvi()
{
	m_Inst = BmpToAviCreate();
}

inline CBmpToAvi::~CBmpToAvi()
{
	BmpToAviDestroy(m_Inst);
}

inline bool CBmpToAvi::Open(const BMPTOAVI_PARMS& Parms, LPCTSTR Path, bool ShowComprDlg)
{
	return(BmpToAviOpen(m_Inst, Parms, Path, ShowComprDlg));
}

inline bool	CBmpToAvi::Close()
{
	return(BmpToAviClose(m_Inst));
}

inline bool	CBmpToAvi::AddFrame(HBITMAP hBitmap)
{
	return(BmpToAviAddFrame(m_Inst, hBitmap));
}

inline bool CBmpToAvi::IsCompressed() const
{
	return(BmpToAviIsCompressed(m_Inst));
}

inline int CBmpToAvi::GetLastError(HRESULT *hr) const
{
	return(BmpToAviGetLastError(m_Inst, hr));
}

inline void CBmpToAvi::GetLastErrorString(CString& Err, CString& DSErr) const
{
	static const int	MAX_ERROR_TEXT = 160;	// from VFW's Errors.h
	LPTSTR	pErr = Err.GetBuffer(MAX_ERROR_TEXT);
	LPTSTR	pDSErr = DSErr.GetBuffer(MAX_ERROR_TEXT);
	BmpToAviGetLastErrorString(m_Inst, pErr, pDSErr, MAX_ERROR_TEXT);
	Err.ReleaseBuffer();
	DSErr.ReleaseBuffer();
}

inline void CBmpToAvi::GetComprState(CVideoComprState& State) const
{
	static const int	MAX_COMPR_NAME = 256;	// generous guess
	LPTSTR	pName = State.m_Name.GetBuffer(MAX_COMPR_NAME);
	BmpToAviGetComprState(m_Inst, pName, MAX_COMPR_NAME, 
		State.m_Parms, State.m_DlgState);
	State.m_Name.ReleaseBuffer();
}

inline void CBmpToAvi::SetComprState(const CVideoComprState& State)
{
	BmpToAviSetComprState(m_Inst, State.m_Name, State.m_Parms, State.m_DlgState);
}

#endif
