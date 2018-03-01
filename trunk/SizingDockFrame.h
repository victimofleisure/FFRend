// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jan07	initial version
		01		23feb09	add non-blocking mode
		02		21mar09	add system menu item to enable/disable docking
		03		29apr10	derive from SCB miniframe for floating dynamic resize

		sizing bar dock frame that doesn't require idle time
 
*/

#if !defined(AFX_SIZINGDOCKFRAME_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_SIZINGDOCKFRAME_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SizingDockFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSizingDockFrame dialog

#include <afxpriv.h>

class CMySizingControlBar;

class CSizingDockFrame : public CSCBMiniDockFrameWnd
{
	DECLARE_DYNCREATE(CSizingDockFrame);
// Construction
public:
	CSizingDockFrame();

// Attributes
	bool	GetNonBlockingMode() const;
	void	SetNonBlockingMode(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSizingDockFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CSizingDockFrame)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcRButtonDown(UINT nHitTest, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	//}}AFX_MSG
	LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		// user system menu command IDs must avoid using unreliable low nibble
		USC_DOCKABLE = 0x10
	};

// Member data
	bool	m_IsNCMoving;		// if true, we're being moved by non-client drag
	bool	m_IsNonBlocking;	// if true, we shouldn't block the message loop
	CPoint	m_NCLBDownPos;		// cursor position at non-client left button down

// Helpers
	CMySizingControlBar	*GetSizingBar();
};

inline bool CSizingDockFrame::GetNonBlockingMode() const
{
	return(m_IsNonBlocking);
}

inline void CSizingDockFrame::SetNonBlockingMode(bool Enable)
{
	m_IsNonBlocking = Enable;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIZINGDOCKFRAME_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
