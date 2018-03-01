// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jan07	initial version
        01      05oct07	update for reorganized base class
		02		05jan10	standardize OnInitDialog prototype
		03		19jul11	add delete to context menu
	
		customize file browser control bar for FFRend
 
*/

#if !defined(AFX_FILEBROWSERFF_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
#define AFX_FILEBROWSERFF_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileBrowserFF.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserFF dialog

#include "FileBrowserBar.h"

class CMainFrame;

class CFileBrowserFF : public CFileBrowserControlBar
{
	DECLARE_DYNAMIC(CFileBrowserFF);
// Construction
public:
	CFileBrowserFF();
	~CFileBrowserFF();

// Constants
	enum {	// define browser panes
		PANE_PROJECTS,
		PANE_PLUGINS,
		PANE_CLIPS,
		PANES
	};

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBrowserFF)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CFileBrowserFF)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CFileBrowserFF)
	afx_msg void OnDestroy();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
	//}}AFX_MSG
	afx_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFBOpenItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBRenameItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBDragMove(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBDragEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBContextMenu(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFBViewType(UINT nID);
	afx_msg void OnUpdateFBViewType(CCmdUI* pCmdUI);
	afx_msg void OnFBSortType(UINT nID);
	afx_msg void OnUpdateFBSortType(CCmdUI* pCmdUI);
	afx_msg void OnFBParentFolder();
	afx_msg void OnUpdateFBParentFolder(CCmdUI* pCmdUI);
	afx_msg void OnFBRename();
	afx_msg void OnFBDelete();
	afx_msg void OnUpdateFBRename(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFBFileProps(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagPANE_INFO {
		int		NameId;				// string resource ID of pane name
		int		Style;				// list control style bits
		const	LPCTSTR	*ExtFilter;	// array of filter extension strings
		int		ExtCount;			// number of filter extensions
	} PANE_INFO;

// Constants
	static const PANE_INFO PaneInfo[PANES];
	static const int m_ViewTypeMap[];
	static const int m_SortTypeMap[];
	static const PANE_STATE	m_DefaultPaneState;

// Member data
	CMainFrame	*m_Frm;				// pointer to main frame
	int		m_CurPane;				// currently selected pane
	PANE_STATE	m_PaneState[PANES];	// array of pane states
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEBROWSERFF_H__DF8CAC05_A18B_4898_93FC_8FD9D24B6984__INCLUDED_)
