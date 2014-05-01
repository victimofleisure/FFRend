// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jan07	initial version
		01		21jan07	replace AfxGetMainWnd with GetThis
        02      05oct07	update for reorganized base class
		03		23nov07	support Unicode
		04		17dec07	add mpeg extension
		05		05jan10	standardize OnInitDialog prototype
		06		19jul11	replace context menu checks with radios
		07		19jul11	add delete to context menu
		
		customize file browser control bar for FFRend
 
*/

// FileBrowserFF.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "FileBrowserFF.h"
#include "Persist.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserFF dialog

IMPLEMENT_DYNAMIC(CFileBrowserFF, CFileBrowserControlBar);

#define RK_FB_CUR_PANE		_T("FBCurPane")
#define RK_FB_PANE_STATE	_T("FBPaneState")

static const LPCTSTR ProjectExt[] = {_T(".ffp")};
static const LPCTSTR PluginExt[] = {_T(".dll")};
static const LPCTSTR ClipExt[] = {
	_T(".avi"), 
	_T(".mpg"),
	_T(".bmp"),
	_T(".jpg"),
	_T(".gif"),
	_T(".avs"),
};

const CFileBrowserFF::PANE_INFO CFileBrowserFF::PaneInfo[PANES] = {
	{IDS_FB_PROJECTS,	LVS_SINGLESEL,	ProjectExt,	sizeof(ProjectExt) / sizeof(LPCTSTR)},
	{IDS_FB_PLUGINS,	0,				PluginExt,	sizeof(PluginExt) / sizeof(LPCTSTR)}, 
	{IDS_FB_CLIPS,		LVS_SINGLESEL,	ClipExt,	sizeof(ClipExt) / sizeof(LPCTSTR)}
};

const CFileBrowserFF::PANE_STATE CFileBrowserFF::m_DefaultPaneState = {
	CFileBrowserListCtrl::VTP_REPORT, CDirList::SORT_NAME, CDirList::ORDER_ASC, {0, 1, 2, 3}
};

// alphabetical order by ID name; must match Resource.h and message map ranges
const int CFileBrowserFF::m_ViewTypeMap[] = {
	CFileBrowserListCtrl::VTP_ICON,			// ID_FB_VIEW_ICON
	CFileBrowserListCtrl::VTP_LIST,			// ID_FB_VIEW_LIST
	CFileBrowserListCtrl::VTP_REPORT,		// ID_FB_VIEW_REPORT
	CFileBrowserListCtrl::VTP_SMALLICON,	// ID_FB_VIEW_SMALL_ICON
	CFileBrowserListCtrl::VTP_THUMBNAIL		// ID_FB_VIEW_THUMBNAIL
};

// alphabetical order by ID name; must match Resource.h and message map ranges
const int CFileBrowserFF::m_SortTypeMap[] = {
	CDirList::SORT_LAST_WRITE,	// ID_FB_SORT_DATE
	CDirList::SORT_NAME,		// ID_FB_SORT_NAME
	CDirList::SORT_LENGTH,		// ID_FB_SORT_SIZE
	CDirList::SORT_FILE_TYPE	// ID_FB_SORT_TYPE
};

CFileBrowserFF::CFileBrowserFF()
{
	//{{AFX_DATA_INIT(CFileBrowserFF)
	//}}AFX_DATA_INIT
	m_CurPane = CPersist::GetInt(REG_SETTINGS, RK_FB_CUR_PANE, 0);
	for (int i = 0; i < PANES; i++)
		m_PaneState[i] = m_DefaultPaneState;
	DWORD	sz = sizeof(m_PaneState);
	CPersist::GetBinary(REG_SETTINGS, RK_FB_PANE_STATE, &m_PaneState, &sz);
	m_Frm = NULL;
}

CFileBrowserFF::~CFileBrowserFF()
{
	CPersist::WriteInt(REG_SETTINGS, RK_FB_CUR_PANE, m_CurPane);
	CPersist::WriteBinary(REG_SETTINGS, RK_FB_PANE_STATE, m_PaneState, sizeof(m_PaneState));
}

void CFileBrowserFF::DoDataExchange(CDataExchange* pDX)
{
	CFileBrowserControlBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileBrowserFF)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFileBrowserFF, CFileBrowserControlBar)
	//{{AFX_MSG_MAP(CFileBrowserFF)
	ON_WM_DESTROY()
	ON_WM_MENUSELECT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
	ON_NOTIFY(FBLCN_OPENITEM, IDC_FB_LIST, OnFBOpenItem)
	ON_NOTIFY(FBLCN_RENAMEITEM, IDC_FB_LIST, OnFBRenameItem)
	ON_NOTIFY(FBLCN_DRAGMOVE, IDC_FB_LIST, OnFBDragMove)
	ON_NOTIFY(FBLCN_DRAGEND, IDC_FB_LIST, OnFBDragEnd)
	ON_NOTIFY(FBLCN_CONTEXTMENU, IDC_FB_LIST, OnFBContextMenu)
	ON_COMMAND_RANGE(ID_FB_VIEW_ICON, ID_FB_VIEW_THUMBNAIL, OnFBViewType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FB_VIEW_ICON, ID_FB_VIEW_THUMBNAIL, OnUpdateFBViewType)
	ON_COMMAND_RANGE(ID_FB_SORT_DATE, ID_FB_SORT_TYPE, OnFBSortType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FB_SORT_DATE, ID_FB_SORT_TYPE, OnUpdateFBSortType)
	ON_COMMAND(ID_FB_PARENT_FOLDER, OnFBParentFolder)
	ON_UPDATE_COMMAND_UI(ID_FB_PARENT_FOLDER, OnUpdateFBParentFolder)
	ON_COMMAND(ID_FB_RENAME, OnFBRename)
	ON_UPDATE_COMMAND_UI(ID_FB_RENAME, OnUpdateFBRename)
	ON_COMMAND(ID_FB_DELETE, OnFBDelete)
	ON_UPDATE_COMMAND_UI(ID_FB_FILE_PROPS, OnUpdateFBFileProps)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserFF message handlers

LRESULT CFileBrowserFF::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CFileBrowserControlBar::OnInitDialog(wParam, lParam);

	m_Frm = theApp.GetMain();
	for (int i = 0; i < PANES; i++) {
		const PANE_INFO&	info = PaneInfo[i];
		CString	PaneName;
		PaneName.LoadString(info.NameId);
		InsertPane(i, PaneName, DEFAULT_LIST_STYLE | info.Style);
		CStringArray	ExtFilter;
		ExtFilter.SetSize(info.ExtCount);
		for (int j = 0; j < info.ExtCount; j++)
			ExtFilter[j] = info.ExtFilter[j];
		CFileBrowserListCtrl&	list = GetList(i);
		list.SetExtFilter(ExtFilter);
		ListView_SetExtendedListViewStyleEx(list.m_hWnd, 
			LVS_EX_HEADERDRAGDROP, LVS_EX_HEADERDRAGDROP);
		SetPaneState(i, m_PaneState[i]);
	}
	SetCurPane(m_CurPane);

	return TRUE;
}

void CFileBrowserFF::OnDestroy() 
{
	m_CurPane = GetCurPane();
	for (int i = 0; i < PANES; i++)
		GetPaneState(i, m_PaneState[i]);
	CFileBrowserControlBar::OnDestroy();
}

void CFileBrowserFF::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	// allow our context menu to display hints in the status bar
	m_Frm->SendMessage(WM_SETMESSAGESTRING, nItemID);	// show hint for this menu item
}

void CFileBrowserFF::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	// prevent main's frame counter from overwriting our menu hints
	m_Frm->SendMessage(WM_ENTERMENULOOP, bIsTrackPopupMenu);
}

void CFileBrowserFF::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	// restore main's frame counter behavior
	m_Frm->SendMessage(WM_EXITMENULOOP, bIsTrackPopupMenu);
}

void CFileBrowserFF::OnFBOpenItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_Frm->SendMessage(WM_NOTIFY, pNMHDR->idFrom, (LPARAM)pNMHDR);
}

void CFileBrowserFF::OnFBRenameItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_Frm->SendMessage(WM_NOTIFY, pNMHDR->idFrom, (LPARAM)pNMHDR);
}

void CFileBrowserFF::OnFBDragMove(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW	pnmlv = (LPNMLISTVIEW)pNMHDR;
	CPoint	pt(pnmlv->ptAction);
	CRect	mr, br;
	m_Frm->GetWindowRect(mr);
	GetWindowRect(br);
	// if cursor is within main frame but not within file browser
	if (mr.PtInRect(pt) && !br.PtInRect(pt) && GetCurPane() >= 0) {
		SetCursor(AfxGetApp()->LoadCursor(
			GetCurList()->GetSelectedCount() > 1 ?
			IDC_DRAG_MULTI : IDC_DRAG_SINGLE));
	} else
		SetCursor(LoadCursor(NULL, IDC_NO));
}

void CFileBrowserFF::OnFBDragEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	m_Frm->SendMessage(WM_NOTIFY, pNMHDR->idFrom, (LPARAM)pNMHDR);
}

void CFileBrowserFF::OnFBContextMenu(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW	pnmlv = (LPNMLISTVIEW)pNMHDR;
	int	MenuId;
	if (pnmlv->iItem >= 0)
		MenuId = IDR_FB_CTX_ITEM;
	else
		MenuId = IDR_FB_CTX_BAR;
	CMenu	menu;
	menu.LoadMenu(MenuId);
	CMenu	*mp = menu.GetSubMenu(0);
	theApp.UpdateMenu(this, &menu);
	CPoint	pt(pnmlv->ptAction);
	if (GetCurPane() >= 0) {
		GetCurList()->ClientToScreen(&pt);
		mp->TrackPopupMenu(0, pt.x, pt.y, this);
	}
}

void CFileBrowserFF::OnFBViewType(UINT nID)
{
	UINT	idx = nID -= ID_FB_VIEW_ICON;
	ASSERT(idx >= 0 && idx < CFileBrowserListCtrl::VIEW_TYPES);
	if (GetCurPane() >= 0)
		GetCurList()->SetViewType(m_ViewTypeMap[idx]);
}

void CFileBrowserFF::OnUpdateFBViewType(CCmdUI* pCmdUI) 
{
	UINT	idx = pCmdUI->m_nID - ID_FB_VIEW_ICON;
	ASSERT(idx >= 0 && idx < CFileBrowserListCtrl::VIEW_TYPES);
	if (GetCurPane() >= 0)
		pCmdUI->SetRadio(GetCurList()->GetViewType() == m_ViewTypeMap[idx]);
}

void CFileBrowserFF::OnFBSortType(UINT nID)
{
	UINT	idx = nID -= ID_FB_SORT_DATE;
	ASSERT(idx >= 0 && idx < CDirList::SORT_PROPS);
	if (GetCurPane() >= 0)
		GetCurList()->SetSort(m_SortTypeMap[idx], FALSE);
}

void CFileBrowserFF::OnUpdateFBSortType(CCmdUI* pCmdUI) 
{
	UINT	idx = pCmdUI->m_nID - ID_FB_SORT_DATE;
	ASSERT(idx >= 0 && idx < CDirList::SORT_PROPS);
	if (GetCurPane() >= 0)
		pCmdUI->SetRadio(GetCurList()->GetSortCol() == m_SortTypeMap[idx]);
}

void CFileBrowserFF::OnFBParentFolder() 
{
	if (GetCurPane() >= 0)
		GetCurList()->OpenParentFolder();
}

void CFileBrowserFF::OnUpdateFBParentFolder(CCmdUI* pCmdUI) 
{
	if (GetCurPane() >= 0)
		pCmdUI->Enable(GetCurList()->HasParentFolder());
}

void CFileBrowserFF::OnFBRename() 
{
	if (GetCurPane() >= 0)
		GetCurList()->Rename();
}

void CFileBrowserFF::OnUpdateFBRename(CCmdUI* pCmdUI)
{
	if (GetCurPane() >= 0)
		pCmdUI->Enable(GetCurList()->CanRename());
}

void CFileBrowserFF::OnFBDelete()
{
	CFileBrowserListCtrl	*pList = GetCurList();
	if (pList != NULL) {
		CStringArray	SelPath;
		pList->GetSelectedItems(SelPath);
		CFileBrowserListCtrl::DeleteFileList(m_hWnd, SelPath, FOF_ALLOWUNDO);
		pList->Refresh();
	}
}

void CFileBrowserFF::OnUpdateFBFileProps(CCmdUI *pCmdUI)
{
	bool	Enable = FALSE;
	switch (GetCurPane()) {
	case PANE_PLUGINS:
	case PANE_CLIPS:
		if (GetCurPane() >= 0) {
			CDirItemArray	DirItem;
			GetCurList()->GetSelectedItems(DirItem);
			int	items = DirItem.GetSize();
			for (int i = 0; i < items; i++) {
				if (!DirItem[i].IsDir()) {
					Enable = TRUE;	// at least one non-folder item
					break;
				}
			}
		}
		break;
	}
	pCmdUI->Enable(Enable);
}
