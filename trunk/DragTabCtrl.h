// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		28jul06	initial version
		01		23nov07	support Unicode
		02		28nov07	add IsDragging
		03		07dec07	add get/set image
		04		17may12	add scrolling during drag
		05		18may12	derive from extended tab control

		tab control with drag reordering

*/

#if !defined(AFX_DRAGTABCTRL_H__6F4A61F8_D590_426E_AD60_A826FDF20554__INCLUDED_)
#define AFX_DRAGTABCTRL_H__6F4A61F8_D590_426E_AD60_A826FDF20554__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragTabCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDragTabCtrl window

#include "TabCtrlEx.h"

class CDragTabCtrl : public CTabCtrlEx
{
	DECLARE_DYNAMIC(CDragTabCtrl);
// Construction
public:
	CDragTabCtrl();

// Attributes
public:
	bool	IsDragging() const;

// Operations
public:
	void	CancelDrag();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDragTabCtrl();

// Generated message map functions
protected:
	//{{AFX_MSG(CDragTabCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		DRAG_THRESHOLD = 2,	// minimum cursor movement to start drag, in client coords
		TIMER_ID = 1201,	// scroll timer identifier
		TIMER_PERIOD = 100,	// scroll timer period, in milliseconds
	};
	enum {	// states
		DTS_NONE,	// left button is up
		DTS_TRACK,	// left button is down, but movement is less than drag threshold
		DTS_DRAG	// left button is down, and drag is in progress
	};

// Data members
	int		m_State;	// see state enum above
	CPoint	m_Origin;	// position at which left button was pressed, in client coords
	int		m_TabIdx;	// index of tab over which left button was pressed
	int		m_ScrollDelta;	// scroll delta while dragging

// Helpers
	void	UpdateCursor(CPoint point);
};

inline bool CDragTabCtrl::IsDragging() const
{
	return(m_State == DTS_DRAG);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGTABCTRL_H__6F4A61F8_D590_426E_AD60_A826FDF20554__INCLUDED_)
