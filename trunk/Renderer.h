// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06mar10	initial version
        01      08mar10	add worker thread
		02		10may11	add OnDeleteFrames
		03		10jan12	add FlushMonitorQueue
		04		16jan12	add main frame pointer
		05		30mar12	make FlushMonitorQueue public
		06		13apr12	standardize thread function
		07		10may12	add m_InSetExclusive

        rendering window
 
*/

#if !defined(AFX_RENDERER_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_RENDERER_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Renderer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRenderer window

#include "EngineThread.h"
#include "Timer.h"
#include "BackBufDD.h"
#include <ddraw.h>
#include "Dib.h"
#include "FrameQueue.h"
#include "FFEngine.h"

class CMainFrame;

class CRenderer : public CWnd, public CEngineThread
{
// Construction
public:
	CRenderer(CFFEngine *Engine);

// Types
	class CRun : public WObject {
	public:
		CRun(CRenderer& Renderer, bool Enable = FALSE);
		~CRun();
		operator bool() const;

	protected:
		CRenderer&	m_Renderer;
		bool	m_PrevRun;
		bool	m_Succeeded;
	};

// Attributes
public:
	CFFEngine	*GetEngine();
	bool	IsPaused() const;
	bool	SetExclusive(bool Enable);
	bool	IsExclusive() const;
	bool	IsDualMonitor() const;
	bool	SetWindowSize(CSize Size);
	CSize	GetFrameSize() const;
	bool	SetFrameSize(CSize FrameSize);
	UINT	GetColorDepth() const;
	bool	SetColorDepth(UINT ColorDepth);
	bool	SetFrameProps(CSize FrameSize, UINT ColorDepth);
	bool	SetFramePeriod(UINT Period);
	UINT	GetFramePeriod() const;
	volatile PFRAME	GetCurFrame() const;
	CFrameQueue&	GetMonitorQueue();
	bool	IsMonitoring() const;
	static DDSURFACEDESC2	*GetDefaultSurface();
	CDib&	GetPauseFrame();
	bool	GetLockFrameRate() const;
	bool	SetLockFrameRate(bool Enable);
	bool	GetUseMMTimer() const;
	bool	SetUseMMTimer(bool Enable);

// Operations
public:
	bool	Pause(bool Enable);
	bool	CreateSurfaces();
	bool	CreateSurface(DDSURFACEDESC2& sd, IDirectDrawSurface7*& Surf);
	bool	Run(bool Enable);
	bool	LaunchRenderThread();
	bool	KillRenderThread();
	void	DumpState(FILE *fp);
	bool	ExportBitmap(LPCTSTR Path, int Resolution, CFileException *pError = NULL);
	bool 	Monitor(bool Enable);
	bool	SingleStep();
	void	OnDeleteFrames();
	bool	FlushMonitorQueue();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenderer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRenderer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRenderer)
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Member data
	CMainFrame	*m_Main;		// pointer to main frame window
	bool	m_WasShown;			// true if shown at least once
	bool	m_IsExclusive;		// true if we're in exclusive mode
	bool	m_IsDualMonitor;	// true if we're in dual-monitor mode
	bool	m_IsPaused;			// true if we're paused
	bool	m_InSetExclusive;	// true if we're changing exclusive mode
	CBackBufDD	m_dd;			// DirectDraw instance
	IDirectDrawSurface7	*m_BackBuf;	// off-screen back buffer
	DDSURFACEDESC2	m_SurfDesc;	// surface description of back buffer
	CSize	m_FrameSize;		// frame size in pixels
	UINT	m_ColorDepth;		// color depth in bits per pixel
	WTimer	m_FrameTimer;		// Win32 frame timer
	HANDLE	m_hFrameTimer;		// timer handle worker blocks on
	UINT	m_FramePeriod;		// frame period in milliseconds
	PFRAME	m_CurFrame;			// frame we're displaying
	CDib	m_PauseFrame;		// current frame while paused
	CBitmap	m_PauseBackBuf;		// GDI back buffer while paused
	bool	m_IsSingleStep;		// true if stepping one frame at a time
	bool	m_IsMonitoring;		// true if monitoring output
	bool	m_IsRateLocked;		// true if frame rate is locked
	bool	m_UseMMTimer;		// true if using multimedia timer
	UINT	m_MMFrameTimer;		// multimedia frame timer
	WEvent	m_MMTimerEvent;		// event set by multimedia timer
	CFrameQueue	m_MonitorQueue;	// frame queue for monitoring output
	static DWORD	m_DefSurfMem[];	// pointed to by m_DefSurf.lpSurface
	static DDSURFACEDESC2 m_DefSurf;	// default surface

// Helpers
	bool	Render();
	bool	Setup();
	void	Cleanup();
	static	bool	DualMonitorCheck(HWND hWnd);
	bool	Work();
	void	Idle();
	bool	SetFrameTimer(UINT Period);
	void	CancelTimer();
	bool	CreatePauseFrame();
	void	DestroyPauseFrame();
	BOOL	CreatePauseBackBuf(CSize Size);
	static	UINT	ThreadFunc(LPVOID Arg);
};

inline CRenderer::CRun::operator bool() const
{
	return(m_Succeeded);
}

inline CFFEngine *CRenderer::GetEngine()
{
	return((CFFEngine *)m_Engine);
}

inline bool CRenderer::IsPaused() const
{
	return(m_IsPaused);
}

inline bool CRenderer::IsExclusive() const
{
	return(m_IsExclusive);
}

inline bool CRenderer::IsDualMonitor() const
{
	return(m_IsDualMonitor);
}

inline UINT CRenderer::GetFramePeriod() const
{
	return(m_FramePeriod);
}

inline volatile PFRAME CRenderer::GetCurFrame() const
{
	return(m_CurFrame);
}

inline CFrameQueue& CRenderer::GetMonitorQueue()
{
	return(m_MonitorQueue);
}

inline bool CRenderer::IsMonitoring() const
{
	return(m_IsMonitoring);
}

inline DDSURFACEDESC2 *CRenderer::GetDefaultSurface()
{
	return(&m_DefSurf);
}

inline CSize CRenderer::GetFrameSize() const
{
	return(m_FrameSize);
}

inline UINT CRenderer::GetColorDepth() const
{
	return(m_ColorDepth);
}

inline CDib& CRenderer::GetPauseFrame()
{
	return(m_PauseFrame);
}

inline bool CRenderer::GetLockFrameRate() const
{
	return(m_IsRateLocked);
}

inline bool CRenderer::GetUseMMTimer() const
{
}

#define STOP_RENDER(Render)						\
	CRenderer::CRun	StopRenderObj(Render);		\
	if (!StopRenderObj)							\
		return(FALSE);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENDERER_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
