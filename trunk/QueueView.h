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
		02		16apr11	refactor UpdateView

        queue view
 
*/

#if !defined(AFX_QUEUEVIEW_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_QUEUEVIEW_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueueView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQueueView window

class CQueueView : public CScrollView
{
	DECLARE_DYNCREATE(CQueueView)
// Construction
public:
	CQueueView();

// Constants
	enum {	// frame styles
		FS_INDEX	= 0x01,
		FS_CONTENT	= 0x02,
		FS_REFS		= 0x04,
		FS_TIME		= 0x08,
	};

// Attributes
public:
	UINT	GetFrameStyle() const;
	void	SetFrameStyle(UINT Style);

// Operations
public:
	void	UpdateView(bool Repaint = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueueView)
	public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CQueueView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CQueueView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants

// Member data
	CEngine&	m_Engine;		// reference to engine
	CBitmap	m_BackBuf;			// back buffer
	CDC		m_DC;				// back buffer device context
	HGDIOBJ	m_PrevBmp;			// previously selected bitmap
	CSize	m_ClientSize;		// size of client area
	CSize	m_ViewSize;			// size of view
	CPoint	m_PrevScrollPos;	// previous scroll position
	int		m_MaxRowFrames;		// maximum number of frames in a row
	UINT	m_FrameStyle;		// which items are shown in frame
	CBrush	m_ActiveBrush;		// brush to indicate processing

// Helpers
	CSize	GetFrameArea(int Frames, int Rows);
	void	DrawFrame(CPoint pt, const PFRAME Frame);
	void	DrawPlugin(const CRect& rect, LPCTSTR Name, bool IsProcessing);
	void	DrawInputFrame(int InpIdx, CPoint pt, const PFRAME Frame);
	void	DrawOutputFrame(CPoint pt, const PFRAME Frame);
	void	DrawFrameArray(CPoint pt, const CFrameArray& Frame);
	void	DrawFrameCounter(CPoint pt, UINT FrameCounter, int RenderTime);
	bool	CreateBackBuffer(int Width, int Height);
};

inline UINT CQueueView::GetFrameStyle() const
{
	return(m_FrameStyle);
}

inline void CQueueView::SetFrameStyle(UINT Style)
{
	m_FrameStyle = Style;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUEUEVIEW_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
