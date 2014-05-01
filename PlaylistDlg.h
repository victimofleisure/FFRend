// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		19nov11	initial version
		01		30nov11	add buttons
		02		15dec11 remove options dialog, add save check

		playlist dialog

*/

#if !defined(AFX_PLAYLISTDLG_H__E5E20D81_F442_4431_8725_F12372F19D36__INCLUDED_)
#define AFX_PLAYLISTDLG_H__E5E20D81_F442_4431_8725_F12372F19D36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlaylistDlg.h : header file
//

#include "ToolDlg.h"
#include "CtrlResize.h"
#include "DragVirtualListCtrl.h"
#include "RandList.h"
#include "IconButton.h"
#include "NumSpin.h"

/////////////////////////////////////////////////////////////////////////////
// CPlaylistDlg dialog

class CPlaylistDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CPlaylistDlg);
// Construction
public:
	CPlaylistDlg(CWnd* pParent = NULL);   // standard constructor
	~CPlaylistDlg();

// Attributes
	int		GetItemCount() const;
	bool	IsPlaying() const;
	void	SetPlay(bool Enable);
	bool	IsLooping() const;
	void	SetLoop(bool Enable);
	bool	IsShuffling() const;
	void	SetShuffle(bool Enable);
	int		GetPeriod() const;
	void	SetPeriod(int Period);
	int		GetPeriodSecs() const;
	void	SetPeriodSecs(int PeriodSecs);
	bool	IsModified() const;

// Operations
	bool	NewPlaylist();
	bool	Open(LPCTSTR Path);
	bool	Save();
	bool	SaveAs();
	bool	DropProject(LPCTSTR Path);
	bool	SaveCheck();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaylistDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CPlaylistDlg)
	enum { IDD = IDD_PLAYLIST };
	CNumSpin	m_PeriodSpin;
	CNumEdit	m_PeriodEdit;
	CIconButton	m_ShuffleBtn;
	CIconButton	m_LoopBtn;
	CIconButton	m_PlayBtn;
	CDragVirtualListCtrl	m_List;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CPlaylistDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnEditDelete();
	afx_msg void OnEditInsert();
	afx_msg void OnAutoPlay();
	afx_msg void OnAutoLoop();
	afx_msg void OnAutoShuffle();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditAdd();
	afx_msg void OnUpdateAutoPlay(CCmdUI* pCmdUI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateAutoLoop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAutoShuffle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnEditOpenProject();
	afx_msg void OnSysColorChange();
	afx_msg void OnFileNew();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLoop();
	afx_msg void OnPlay();
	afx_msg void OnShuffle();
	//}}AFX_MSG
	afx_msg void OnReorderList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileMru(UINT nID);
	afx_msg void OnUpdateFileMru(CCmdUI* pCmdUI);
	afx_msg	void OnChangedPeriodEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct {
		int		TitleID;
		int		Align;
		int		Width;
	} LIST_COL;

// Constants
	enum {	// define list columns; must match m_ListCol array init
		COL_PROJECT,
		COLS
	};
	enum {	// item states
		IS_NORMAL,	// item is normal
		IS_STOPPED,	// item is current and automation is stopped
		IS_PLAYING,	// item is current and automation is playing
		ITEM_STATES	// number of item states
	};
	enum {
		AUTO_TIMER_ID = 1000,		// automation timer identifier
		MAX_RECENT_PLAYLISTS = 4,	// maximum recently used playlists
		PLAYLIST_VERSION = 1,		// playlist file format version
		DEFAULT_PERIOD = 60000,		// default period, in milliseconds
	};
	static const LIST_COL	m_ListCol[COLS];
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];	// zero terminated
	static const int	m_StateIcon[ITEM_STATES];

// Member data
	CCtrlResize	m_Resize;		// control resizing object
	CRect	m_InitRect;			// initial window rectangle
	CStringArray	m_Project;	// list of projects
	CString	m_DocPath;			// path of current playlist
	CImageList	m_StateImg;		// image list of state icons
	CRecentFileList	m_RecentPlaylist;	// recently used playlists
	CRandList	m_RandList;		// list randomizer for shuffle
	int		m_CurItem;			// index of current item
	bool	m_InTimer;			// true if handling timer message
	bool	m_Playing;			// true if automation is playing
	bool	m_Looping;			// true if automation is looping
	bool	m_Shuffling;		// true if automation is randomized
	bool	m_Modified;			// true if playlist was modified
	int		m_Period;			// automation period, in milliseconds

// Helpers
	void	UpdateList();
	void	GetSelection(CDWordArray& Sel);
	bool	ReadPlaylist(LPCTSTR Path);
	bool	WritePlaylist(LPCTSTR Path) const;
	void	DeleteSelectedItems(int& CurPos);
	void	InsertItems(int ItemIdx, const CStringArray& Item);
	bool	PromptForProjects(CStringArray& Project) const;
	void	SelectItem(int ItemIdx, bool Enable);
	void	ClearSelection();
	void	SetStateIcon(int ItemIdx, int State);
	void	UpdateStateIcon();
	bool	OpenRecent(int FileIdx);
	bool	OpenProject(int ItemIdx);
};

inline int CPlaylistDlg::GetItemCount() const
{
	return(m_Project.GetSize());
}

inline bool CPlaylistDlg::IsPlaying() const
{
	return(m_Playing);
}

inline bool CPlaylistDlg::IsLooping() const
{
	return(m_Looping);
}

inline bool CPlaylistDlg::IsShuffling() const
{
	return(m_Shuffling);
}

inline int CPlaylistDlg::GetPeriod() const
{
	return(m_Period);
}

inline bool CPlaylistDlg::IsModified() const
{
	return(m_Modified);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYLISTDLG_H__E5E20D81_F442_4431_8725_F12372F19D36__INCLUDED_)
