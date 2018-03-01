// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05oct07	initial version
		01		05jan10	use CArrayEx
		02		24apr10	use CPair for map positions
		03		19jul11	add delete file list

		file browser list control
 
*/

#if !defined(AFX_FILEBROWSERLISTCTRL_H__F73C724F_C135_493B_AD99_AB3D686CA0C5__INCLUDED_)
#define AFX_FILEBROWSERLISTCTRL_H__F73C724F_C135_493B_AD99_AB3D686CA0C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileBrowserListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserListCtrl window

#include "FileInfoCache.h"
#include "ThumbCache.h"
#include "ThumbThread.h"

// custom notifications
#define FBLCN_FIRST			(0U - 1600U)

#define FBLCN_OPENITEM		(FBLCN_FIRST - 1)	// lParam: NMFBOPENITEM*
#define FBLCN_RENAMEITEM	(FBLCN_FIRST - 2)	// lParam: NMFBRENAMEITEM*
#define FBLCN_DRAGBEGIN		(FBLCN_FIRST - 3)	// lParam: NMLISTVIEW*
#define FBLCN_DRAGMOVE		(FBLCN_FIRST - 4)	// lParam: NMLISTVIEW*
#define FBLCN_DRAGEND		(FBLCN_FIRST - 5)	// lParam: NMLISTVIEW*
#define FBLCN_CONTEXTMENU	(FBLCN_FIRST - 6)	// lParam: NMLISTVIEW*

typedef struct tagFBOPENITEM {	// passed in FBLCN_OPENITEM lParam
	NMHDR	nmhdr;		// common notification header
	LPCTSTR	pszPath;	// path of item to open, or folder path
	bool	bIsDir;		// true if pszPath is a folder path
} NMFBOPENITEM, *LPNMFBOPENITEM;

typedef struct tagFBRENAMEITEM {	// passed in FBLCN_RENAMEITEM lParam
	NMHDR	nmhdr;		// common notification header
	LPCTSTR	pszOldPath;	// item's old path
	LPCTSTR	pszNewPath;	// item's new path
} NMFBRENAMEITEM, *LPNMFBRENAMEITEM;

class CFileBrowserListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CFileBrowserListCtrl);
// Construction
public:
	CFileBrowserListCtrl();

// Constants
	enum {	// view types; order must match list control view types
		VTP_ICON,		// LVS_ICON
		VTP_REPORT,		// LVS_REPORT
		VTP_SMALLICON,	// LVS_SMALLICON
		VTP_LIST,		// LVS_LIST
		VTP_THUMBNAIL,
		VIEW_TYPES
	};
	enum {	// report columns
		COL_NAME,
		COL_SIZE,
		COL_TYPE,
		COL_MODIFIED,
		COLUMNS
	};

// Attributes
public:
	void	SetExtFilter(const CStringArray& Filter);
	void	GetExtFilter(CStringArray& Filter) const;
	void	SetFolder(LPCTSTR Path);
	LPCTSTR GetFolder() const;
	void	SetViewType(int Type);
	int		GetViewType() const;
	void	SetSort(int Col, int Dir);
	int		GetSortCol() const;
	int		GetSortDir() const;
	void	GetSelectedItems(CStringArray& Path) const;
	void	GetSelectedItems(CDirItemArray& DirItem) const;
	void	GetSelection(CDWordArray& SelIdx) const;
	const	CDirItem& GetDirItem(int ItemIdx) const;
	CString	GetItemPath(int ItemIdx) const;
	void	SetThumbCache(CThumbCache *Cache);
	CThumbCache	*GetThumbCache() const;
	void	SetThumbSize(CSize ThumbSize);
	CSize	GetThumbSize() const;
	void	SetColorDepth(DWORD ColorDepth);
	DWORD	GetColorDepth() const;
	bool	HasParentFolder() const;
	bool	CanRename();
	bool	IsDragging() const;

// Operations
public:
	void	SelectItem(int ItemIdx, bool Enable);
	void	SelectAll();
	void	ClearSelection();
	void	OpenItem(int ItemIdx);
	void	OpenSelectedItems();
	void	OpenParentFolder();
	void	CancelDrag();
	void	Refresh();
	bool	Rename();
	static	bool	RenameFile(HWND hWnd, LPCTSTR OldName, LPCTSTR NewName, FILEOP_FLAGS Flags);
	static	bool	DeleteFileList(HWND hWnd, const CStringArray& List, FILEOP_FLAGS Flags);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBrowserListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFileBrowserListCtrl();

protected:
// Generated message map functions
	//{{AFX_MSG(CFileBrowserListCtrl)
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnOdfinditem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg void OnDividerdblclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnExtractThumb(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagCOL_INFO {	// list column info
		int		Title;			// string resource ID of title
		int		Align;			// alignment
		int		Width;			// initial width
	} COL_INFO;
	typedef CThumbCache::CThumbPos CThumbPos;
	typedef CArrayEx<CThumbPos, CThumbPos> CPosArray;

// Constants
	enum {
		DEFAULT_SORT_COL = CDirList::SORT_NAME,
		DEFAULT_SORT_DIR = CDirList::ORDER_ASC
	};
	static const COL_INFO m_ColInfo[COLUMNS];	// column definitions

// Member data
	CDirList	m_DirList;		// directory listing object
	CStringArray	m_FiltExt;	// extensions to filter files with
	CImageList	m_HdrImgList;	// header image list for sort arrows
	CFileInfo	m_MRUFileInfo;	// most recently used shell file info
	int		m_CachedItemIdx;	// item index of cached file info
	int		m_ViewType;			// current view type
	int		m_SortCol;			// index of column we're sorted by, or -1 if none
	int		m_SortDir;			// sort direction: 0 = ascending, 1 = descending
	int		m_DragItem;			// index of item being dragged
	bool	m_IsDragging;		// if true, a drag is in progress
	bool	m_ThumbsValid;		// if true, thumbnails are valid
	bool	m_LabelEdit;		// if true, a label is being edited
	CSize	m_ThumbSize;		// thumbnail size in pixels
	DWORD	m_ColorDepth;		// thumbnail bits per pixel
	CImageList	m_ThumbImgList;	// list of thumbnail images, one per item
	CByteArray	m_GotThumb;		// for each item, true if thumbnail is in image list
	CPosArray	m_ThumbPos;		// for each item, thumb map position, or null
	CThumbCache	*m_ThumbCache;	// pointer to thumbnail persistent storage
	CThumbThread	m_ThumbThread;	// worker thread for extracting thumbnails
	static	DWORD	m_DisplayBitsPerPel;	// display resolution in bits per pixel
	static	CFileInfoCache	m_FileInfoCache;	// cached file info; not thread-safe

// Helpers
	void	Init();
	void	DrawSortArrow(int Col, int Dir);
	void	Notify(UINT code, int nItem, CPoint pt) const;
	void	Notify(UINT code, PVOID pNotification) const;
	int		GetFirstSelectedItem() const;
	void	UpdateThumbs(bool Rebuild = TRUE);
	void	UpdateThumbResolution();
	bool	MakeThumbFromIcon(int ItemIdx);
	int		CalcMinColumnWidth(int Col);
	static	bool	FormatTime(CTime Time, CString& Str);
	static	bool	FormatSize(LONGLONG Size, CString& Str);
};

inline void CFileBrowserListCtrl::SetExtFilter(const CStringArray& Filter)
{
	m_FiltExt.Copy(Filter);
}

inline void CFileBrowserListCtrl::GetExtFilter(CStringArray& Filter) const
{
	Filter.Copy(m_FiltExt);
}

inline LPCTSTR CFileBrowserListCtrl::GetFolder() const
{
	return(m_DirList.GetFolder());
}

inline int CFileBrowserListCtrl::GetViewType() const
{
	return(m_ViewType);
}

inline int CFileBrowserListCtrl::GetSortCol() const
{
	return(m_SortCol);
}

inline int CFileBrowserListCtrl::GetSortDir() const
{
	return(m_SortDir);
}

inline const CDirItem& CFileBrowserListCtrl::GetDirItem(int ItemIdx) const
{
	return(m_DirList.GetItem(ItemIdx));
}

inline CString CFileBrowserListCtrl::GetItemPath(int ItemIdx) const
{
	return(m_DirList.GetItemPath(ItemIdx));
}

inline CThumbCache *CFileBrowserListCtrl::GetThumbCache() const
{
	return(m_ThumbCache);
}

inline CSize CFileBrowserListCtrl::GetThumbSize() const
{
	return(m_ThumbSize);
}

inline DWORD CFileBrowserListCtrl::GetColorDepth() const
{
	return(m_ColorDepth);
}

inline bool CFileBrowserListCtrl::HasParentFolder() const
{
	return(!m_DirList.GetFolder().IsEmpty());
}

inline bool CFileBrowserListCtrl::IsDragging() const
{
	return(m_IsDragging);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEBROWSERLISTCTRL_H__F73C724F_C135_493B_AD99_AB3D686CA0C5__INCLUDED_)
