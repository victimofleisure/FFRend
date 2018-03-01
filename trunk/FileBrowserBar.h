// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05oct07	initial version
		01		05jan10	standardize OnInitDialog prototype
		02		05jan10	use CArrayEx
		03		30apr11	remove border from default list style
		04		18may12	use extended tab control for wheel scrolling

		control bar for browsing files

*/

#if !defined(AFX_FILEBROWSERCONTROLBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_FILEBROWSERCONTROLBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileBrowserControlBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserControlBar dialog

#include "MySizingControlBar.h"
#include "FileBrowserListCtrl.h"
#include "TabCtrlEx.h"

// "ControlBar" must be included in our class name, else the Class Wizard won't
// show virtual functions for any classes derived from us; hence the long names

class CFileBrowserControlBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CFileBrowserControlBar);
// Construction
public:
	CFileBrowserControlBar();

// Constants
	enum {
		COLUMNS = CFileBrowserListCtrl::COLUMNS,
		DEFAULT_LIST_STYLE = WS_CHILD | LVS_REPORT 
			| LVS_OWNERDATA | LVS_EDITLABELS | LVS_SHOWSELALWAYS
	};

// Types
	typedef struct tagPANE_STATE {
		int		ViewType;			// see enum in CFileBrowserListCtrl
		int		SortCol;			// sort column; see enum in CDirList
		int		SortDir;			// 0 = ascending, 1 = descending
		int		ColOrder[COLUMNS];	// column order
		int		ColWidth[COLUMNS];	// column width
	} PANE_STATE;

// Attributes
	int		GetPaneCount() const;
	void	SetCurPane(int PaneIdx);
	int		GetCurPane() const;
	CFileBrowserListCtrl&	GetList(int PaneIdx);
	const CFileBrowserListCtrl&	GetList(int PaneIdx) const;
	CFileBrowserListCtrl	*GetCurList();
	const CFileBrowserListCtrl	*GetCurList() const;
	void	GetPaneState(int PaneIdx, PANE_STATE& State) const;
	void	SetPaneState(int PaneIdx, const PANE_STATE& State);
	CSize	GetThumbSize() const;
	void	SetThumbSize(CSize Size);
	bool	GetCacheThumbs() const;
	void	SetCacheThumbs(bool Enable);
	void	SetFolder(int PaneIdx, LPCTSTR Path);
	LPCTSTR GetFolder(int PaneIdx) const;

// Operations
	bool	InsertPane(int PaneIdx, LPCTSTR Name, UINT Style = DEFAULT_LIST_STYLE);
	void	RemovePane(int PaneIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBrowserControlBar)
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CFileBrowserControlBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnReturnList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	class CPane : public WObject {
	public:
		CPane();
		CPane(const CPane& Pane);
		~CPane();
		CPane& operator=(const CPane& Pane);
		CFileBrowserListCtrl	*m_List;	// pointer to pane's list control
		CString	m_Folder;		// stores folder changes while pane is hidden
		bool	m_FolderDirty;	// if true, folder changed since pane was hidden
		bool	m_ListSized;	// if true, pane's list control is correct size
		void	Copy(const CPane& Pane);
	};
	typedef CArrayEx<CPane, CPane&> CPaneArray;

protected:
// Member data
	CTabCtrlEx	m_TabCtrl;		// tab control for selecting panes
	CPaneArray	m_Pane;			// array of file browser panes
	CThumbCache	m_ThumbCache;	// thumbnail cache, shared by all panes
	int			m_CurSel;		// currently selected pane
	bool		m_CtrlsSized;	// true if child controls have been sized
	bool		m_CacheThumbs;	// true if we're caching thumbnails

// Helpers
	void	ResizeCtrls();
};

inline CFileBrowserControlBar::CPane::CPane(const CPane& Pane)
{
	Copy(Pane);
}

inline CFileBrowserControlBar::CPane& CFileBrowserControlBar::CPane::operator=(const CPane& Pane)
{
	Copy(Pane);
	return(*this);
}

inline int CFileBrowserControlBar::GetPaneCount() const
{
	return(m_Pane.GetSize());
}

inline int CFileBrowserControlBar::GetCurPane() const
{
	return(m_CurSel);
}

inline CFileBrowserListCtrl& CFileBrowserControlBar::GetList(int PaneIdx)
{
	ASSERT(PaneIdx >= 0 && PaneIdx < m_Pane.GetSize());
	return(*m_Pane[PaneIdx].m_List);
}

inline const CFileBrowserListCtrl& CFileBrowserControlBar::GetList(int PaneIdx) const
{
	ASSERT(PaneIdx >= 0 && PaneIdx < m_Pane.GetSize());
	return(*m_Pane.GetData()[PaneIdx].m_List);	// use GetData to avoid temp object
}

inline CFileBrowserListCtrl *CFileBrowserControlBar::GetCurList()
{
	ASSERT(m_CurSel < m_Pane.GetSize());
	return(m_CurSel >= 0 ? m_Pane[m_CurSel].m_List : NULL);
}

inline const CFileBrowserListCtrl *CFileBrowserControlBar::GetCurList() const
{
	ASSERT(m_CurSel < m_Pane.GetSize());
	return(m_CurSel >= 0 ? m_Pane.GetData()[m_CurSel].m_List : NULL);
}

inline CSize CFileBrowserControlBar::GetThumbSize() const
{
	return(m_ThumbCache.GetThumbSize());
}

inline bool CFileBrowserControlBar::GetCacheThumbs() const
{
	return(m_CacheThumbs);
}

inline LPCTSTR CFileBrowserControlBar::GetFolder(int PaneIdx) const
{
	return(GetList(PaneIdx).GetFolder());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEBROWSERCONTROLBAR_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
