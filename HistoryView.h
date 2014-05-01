// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29mar10	initial version
		01		15sep10	add plugin helpers
		02		30apr11	move thread iterator into engine

        history view
 
*/

#if !defined(AFX_HISTORYVIEW_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_HISTORYVIEW_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistoryWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHistoryView window

#include "FFEngine.h"

class CHistoryInfo;

class CHistoryView : public CScrollView
{
	DECLARE_DYNCREATE(CHistoryView)
// Construction
public:
	CHistoryView();

// Constants
	enum {	// update view flags
		UVF_NO_INVALIDATE	= 0x01,
		UVF_SET_SIZE_ONLY	= 0x02,
	};

// Attributes
public:
	CSize	GetZoom() const;
	void	SetZoom(CSize Zoom);
	void	GetInfo(CHistoryInfo& Info) const;
	void	SetInfo(const CHistoryInfo& Info);
	bool	IsRunning() const;
	bool	IsPaused() const;

// Operations
public:
	void	UpdateView(UINT Flags = 0);
	void	Run(bool Enable);
	void	Pause(bool Enable);
	void	TimerHook();
	void	Zoom(CPoint Origin, double ZoomX, double ZoomY);
	void	ZoomCursor(double ZoomX, double ZoomY);
	bool	Write(LPCTSTR Path);
	bool	Read(LPCTSTR Path);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryView)
	public:
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CHistoryView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CHistoryView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnViewZoomIn();
	afx_msg void OnViewZoomOut();
	afx_msg void OnViewResetZoom();
	afx_msg void OnHistoryPause();
	afx_msg void OnUpdateHistoryPause(CCmdUI* pCmdUI);
	afx_msg void OnHistoryShowTimes();
	afx_msg void OnUpdateHistoryShowTimes(CCmdUI* pCmdUI);
	afx_msg void OnHistorySave();
	afx_msg void OnHistoryOpen();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	class CPauseEngine : public CEngine {
	public:
		CPauseEngine(CRenderer& Renderer);
		void	SetPluginCount(int Plugins);
	};

// Constants
	static const CSize	m_DefaultZoom;
	static const CSize	m_MinZoom;
	static const CSize	m_MaxZoom;

// Member data
	CEngine	*m_Engine;			// pointer to engine, real or pause
	CBitmap	m_BackBuf;			// back buffer
	CDC		m_DC;				// back buffer device context
	HGDIOBJ	m_PrevBmp;			// previously selected bitmap
	CSize	m_ClientSize;		// size of client area
	CSize	m_ViewSize;			// size of view
	CSize	m_Zoom;				// zoom in pixels per second
	DWORD	m_StartTime;		// start time of history
	CPoint	m_PrevScrollPos;	// previous scroll position
	bool	m_Dragging;			// true while dragging
	CPoint	m_DragOrg;			// origin at start of drag
	CSize	m_DragZoom;			// zoom at start of drag
	bool	m_IsRunning;		// true if running
	int		m_TotalTimeSpan;	// time span of all rows; only valid while stopped
	int		m_TextHeight;		// text height in logical units
	CPoint	m_CtxMenuOrg;		// context menu origin, in client coords
	double	m_GridStep;			// grid step size, in seconds
	bool	m_ShowFrameTimes;	// true if showing frame times
	CPauseEngine	*m_PauseEngine;	// engine for containing history while paused
	CRenderer	m_PauseRenderer;	// renderer for containing history while paused
	DWORD	m_ResumeStartTime;	// start time before entering pause

// Helpers
	bool	CreateBackBuffer(int Width, int Height);
	void	ResetView();
	int		TimeToX(int t) const;
	int		XToTime(int x) const;
	void	UpdateGridStep();
	void	CalcTimeSpan();
	void	UpdateZoom(CPoint ScrollPos);
	CSize	GetMaxScrollPos() const;
	static	double	QuantizeStep(double StepSize);
};

inline CHistoryView::CPauseEngine::CPauseEngine(CRenderer& Renderer) :
	CEngine(Renderer)
{
}

inline void CHistoryView::TimerHook()
{
	if (m_IsRunning)
		m_StartTime = GetTickCount();
}

inline int CHistoryView::TimeToX(int t) const
{
	return(t * m_Zoom.cx / 1000);
}

inline int CHistoryView::XToTime(int x) const
{
	return(x * 1000 / m_Zoom.cx);
}

inline CSize CHistoryView::GetZoom() const
{
	return(m_Zoom);
}

inline CSize CHistoryView::GetMaxScrollPos() const
{
	return(CSize(max(m_ViewSize.cx - m_ClientSize.cx, 0), 
		max(m_ViewSize.cy - m_ClientSize.cy, 0)));
}

inline bool CHistoryView::IsRunning() const
{
	return(m_IsRunning);
}

inline bool CHistoryView::IsPaused() const
{
	return(m_PauseEngine != NULL);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTORYVIEW_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
