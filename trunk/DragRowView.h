// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      31jan07	initial version
        01      07mar07	make FindRow const
		02		23nov07	support Unicode
		03		28nov07	add IsDragging

        drag reorderable row view
 
*/

#if !defined(AFX_DRAGROWVIEW_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
#define AFX_DRAGROWVIEW_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragRowView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDragRowView dialog

#include "RowView.h"

class CDragRowView : public CRowView
{
	DECLARE_DYNCREATE(CDragRowView);
// Construction
public:
	CDragRowView();

// Attributes
	bool	IsDragging() const;

// Operations
	void	BeginDrag(int RowIdx);
	int		FindRow(CPoint pt) const;
	void	CancelDrag();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragRowView)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CDragRowView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		SCROLL_TIMER_ID = 1000,	// scroll timer's ID
		SCROLL_DELAY = 50,	// scrolling delay, in milliseconds
		DRAG_THRESHOLD = 2	// minimum cursor movement to start drag, in client coords
	};
	enum {	// states
		DTS_NONE,	// left button is up
		DTS_TRACK,	// left button is down, but movement is less than drag threshold
		DTS_DRAG	// left button is down, and drag is in progress
	};

// Member data
	int		m_DragSrcRow;		// row index of drag source
	int		m_ScrollDelta;		// amount to scroll, in view coords
	UINT	m_ScrollTimer;		// during drag, scroll timer instance
	int		m_ContextSlot;		// slot index of context menu's target
	int		m_State;			// see state enum above
	CPoint	m_Origin;			// position at which left button was pressed, in client coords

// Overridables
	virtual	void	OnDrop(int SrcRow, int DstRow);

// Helpers
	void	EndDrag(CPoint Cursor);
	void	AutoScroll(CPoint Cursor);
	void	UpdateCursor(CPoint point);
};

inline bool CDragRowView::IsDragging() const
{
	return(m_State == DTS_DRAG);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGROWVIEW_H__53C410DA_1109_40AF_B567_7D7918C63980__INCLUDED_)
