// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan07	initial version
        01      19jan07	add OnDestroy to clear m_IsVisible
        02      01dec07	add context menu
		03		02may10	refactor for engine
		04		10may11	add SetDefaultSurface
		05		26mar12 restore monitor source context menu
		06		30mar12	add Clear
		07		12apr12	make CalcMonitorRect static

		monitor control bar
 
*/

#if !defined(AFX_MONITORBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_MONITORBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MonitorBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMonitorBar dialog

#include "MySizingControlBar.h"
#include "MonitorWnd.h"

class CMonitorBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CMonitorBar);
// Construction
public:
	CMonitorBar();

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
	//{{AFX_VIRTUAL(CMonitorBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CMonitorBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types

// Constants

// Member data
	CMonitorWnd	m_Monitor;		// our video is displayed here
	CSize	m_FrameSize;		// current frame size

// Helpers
	static	void	CalcMonitorRect(CSize FrmSz, CRect& r);
};

inline void CMonitorBar::TimerHook()
{
	m_Monitor.TimerHook();
}

inline void CMonitorBar::DestroyFrame()
{
	m_Monitor.DestroyFrame();
}

inline bool CMonitorBar::GetQuality() const
{
	return(m_Monitor.GetQuality());
}

inline void CMonitorBar::SetQuality(bool Quality)
{
	m_Monitor.SetQuality(Quality);
}

inline void CMonitorBar::SetDefaultSurface()
{
	m_Monitor.SetDefaultSurface();
}

inline void CMonitorBar::UpdateView()
{
	m_Monitor.UpdateView();
}

inline void CMonitorBar::Clear()
{
	m_Monitor.Clear();
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MONITORBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
