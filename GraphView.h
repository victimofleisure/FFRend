// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		26mar10	initial version

        graph view
 
*/

#if !defined(AFX_GRAPHVIEW_H__43DEB2E9_C6E8_4DCA_8387_15C0A9C6F932__INCLUDED_)
#define AFX_GRAPHVIEW_H__43DEB2E9_C6E8_4DCA_8387_15C0A9C6F932__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GraphView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGraphView view

#include "Picture.h"
#include "WndTimer.h"

class CEngine;

class CGraphView : public CScrollView
{
	DECLARE_DYNCREATE(CGraphView)
public:
	CGraphView();

// Attributes
public:

// Operations
public:
	bool	UpdateView();
	void	DelayedUpdate();
	void	OnGraphDone(LPTSTR pPicPath);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CGraphView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CGraphView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		RESIZE_TIMER_ID	= 260,		// resize timer ID
		RESIZE_DELAY = 500,			// resize update delay, in milliseconds
	};

// Member data
	CEngine&	m_Engine;			// reference to engine
	CPicture	m_Graph;			// graph image
	CSize		m_GraphSize;		// size of graph
	CWndTimer	m_ResizeTimer;		// resize timer
	CSize		m_PrevClientSize;	// size of client window at last update

// Helpers
	bool	MakeGraphData(LPCTSTR Path, double Width, double Height, double DPI);
	static	UINT	GraphThread(LPVOID pParam);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRAPHVIEW_H__43DEB2E9_C6E8_4DCA_8387_15C0A9C6F932__INCLUDED_)
