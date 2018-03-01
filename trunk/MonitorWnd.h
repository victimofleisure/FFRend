// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan07	initial version
		01		02may10	refactor for engine
		02		10may11	add SetDefaultSurface
		03		30mar12	add Clear
		
		monitor window
 
*/

#if !defined(AFX_MONITORWND_H__C03D95E0_4477_4D61_B20E_5F8780A8C155__INCLUDED_)
#define AFX_MONITORWND_H__C03D95E0_4477_4D61_B20E_5F8780A8C155__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MonitorWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMonitorWnd window

#include "Dib.h"
#include <ddraw.h>

class CRenderer;

class CMonitorWnd : public CWnd
{
	DECLARE_DYNAMIC(CMonitorWnd);
// Construction
public:
	CMonitorWnd();

// Attributes
	bool	GetQuality() const;
	void	SetQuality(bool Quality);

// Operations
	bool	CreateFrame(CSize FrameSize);
	void	DestroyFrame();
	void	SetDefaultSurface();
	void	UpdateView();
	void	TimerHook();
	void	Clear();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMonitorWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMonitorWnd();

// Generated message map functions
protected:
	//{{AFX_MSG(CMonitorWnd)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CRenderer	*m_Renderer;	// pointer to output renderer
	CSize	m_ClientSize;		// size of client rectangle
	CSize	m_FrameSize;		// current frame size in pixels
	CBitmap	m_OutBmp;			// bitmap of stretched output frame
	CDC		m_OutDC;			// device context for output bitmap
	HGDIOBJ	m_PrevBmp;			// device context's previous bitmap
	IDirectDrawSurface7	*m_FrameSurf;	// off-screen surface for input frame
	DDSURFACEDESC2	m_SurfDesc;	// description of input frame surface
	bool	m_Quality;			// if true, halftone stretch blit

// Helpers
	bool	CreateBackBuffer(int Width, int Height);
	void	ApplyQuality(bool Quality);
};

inline bool CMonitorWnd::GetQuality() const
{
	return(m_Quality);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MONITORWND_H__C03D95E0_4477_4D61_B20E_5F8780A8C155__INCLUDED_)
