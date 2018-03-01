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
		02		28apr10	in ResizeCtrls, use defer erase to reduce flicker
		03		30apr11	increase list border by one
		04		11nov11	fix keyboard-triggered context menu

		control bar for browsing files

*/

// FileBrowserControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FileBrowserBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserControlBar dialog

IMPLEMENT_DYNAMIC(CFileBrowserControlBar, CMySizingControlBar);

CFileBrowserControlBar::CPane::CPane()
{
	m_List = NULL;
	m_FolderDirty = FALSE;
	m_ListSized = FALSE;
}

CFileBrowserControlBar::CPane::~CPane()
{
	if (m_List != NULL) {
		m_List->DestroyWindow();
		delete m_List;
	}
}

void CFileBrowserControlBar::CPane::Copy(const CPane& Pane)
{
	m_List = Pane.m_List;
	m_Folder = Pane.m_Folder;
	m_FolderDirty = Pane.m_FolderDirty;
	m_ListSized = Pane.m_ListSized;
}

CFileBrowserControlBar::CFileBrowserControlBar()
{
	//{{AFX_DATA_INIT(CFileBrowserControlBar)
	//}}AFX_DATA_INIT
	m_CurSel = -1;
	m_CtrlsSized = FALSE;
	m_CacheThumbs = TRUE;
}

bool CFileBrowserControlBar::InsertPane(int PaneIdx, LPCTSTR Name, UINT Style)
{
	ASSERT(PaneIdx >= 0 && PaneIdx <= GetPaneCount());
	CFileBrowserListCtrl	*lp = new CFileBrowserListCtrl;
	CRect	r(0, 0, 100, 100);	// arbitrary initial size
	if (!lp->Create(Style, r, this, IDC_FB_LIST)) {
		delete lp;
		return(FALSE);
	}
	lp->ModifyStyleEx(0, WS_EX_CLIENTEDGE);	// required for 3D border
	lp->BringWindowToTop();
	lp->SetThumbCache(m_CacheThumbs ? &m_ThumbCache : NULL);
	m_TabCtrl.InsertItem(PaneIdx, Name, 0);
	CPane	Pane;
	m_Pane.InsertAt(PaneIdx, Pane);
	m_Pane[PaneIdx].m_List = lp;
	if (PaneIdx <= m_CurSel)		// if inserting at or below current selection
		m_CurSel++;						// move current pane up one
	else if (GetPaneCount() == 1)	// else if inserting first pane
		SetCurPane(PaneIdx);			// make inserted pane current
	return(TRUE);
}

void CFileBrowserControlBar::RemovePane(int PaneIdx)
{
	ASSERT(PaneIdx >= 0 && PaneIdx < GetPaneCount());
	m_Pane.RemoveAt(PaneIdx);
	m_TabCtrl.DeleteItem(PaneIdx);
	if (PaneIdx == m_CurSel) {		// if we deleted at current pane
		m_CurSel = -1;					// current pane is now invalid
		int	count = GetPaneCount();
		if (count > 0) {				// if there's at least one pane left
			if (PaneIdx == count)			// if we deleted at end of list
				PaneIdx--;						// move current pane down one
			SetCurPane(PaneIdx);			// set current pane
		}
	} else if (PaneIdx < m_CurSel)	// else if we deleted below current pane
		m_CurSel--;						// move current pane down one
}

void CFileBrowserControlBar::SetFolder(int PaneIdx, LPCTSTR Path)
{
	ASSERT(PaneIdx >= 0 && PaneIdx < GetPaneCount());
	if (PaneIdx == m_CurSel)	// if current selected pane
		GetList(PaneIdx).SetFolder(Path);
	else {	// don't update hidden list control, just store folder path
		CPane& pane = m_Pane[PaneIdx];
		pane.m_Folder = Path;
		pane.m_FolderDirty = TRUE;
	}
}

void CFileBrowserControlBar::SetCurPane(int PaneIdx)
{
	ASSERT(PaneIdx >= 0 && PaneIdx < GetPaneCount());
	if (PaneIdx == m_CurSel)
		return;	// nothing to do
	bool	ListHasFocus = FALSE;
	CFileBrowserListCtrl	*CurList = GetCurList();
	if (CurList != NULL) {	// if current selection is valid
		if (::GetFocus() == CurList->m_hWnd)
			ListHasFocus = TRUE;
		// to avoid flicker and save time, hide currently selected list without
		// redrawing; it will be completely overwritten by newly selected list
		CurList->SetWindowPos(0, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOREDRAW 
			| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);	// no other changes
	}
	m_TabCtrl.SetCurSel(PaneIdx);
	CPane& pn = m_Pane[PaneIdx];	// reference new list
	if (pn.m_FolderDirty) {	// if list's folder was changed
		pn.m_List->SetFolder(pn.m_Folder);	// update list's folder
		pn.m_FolderDirty = FALSE;	// reset folder changed flag
	}
	m_CurSel = PaneIdx;	// order matters; ResizeCtrls uses m_CurSel
	if (m_CtrlsSized && !pn.m_ListSized) {	// if list needs sizing
		ResizeCtrls();			// resize it
		pn.m_ListSized = TRUE;	// mark it done
	}
	if (ListHasFocus)	// if previous list had focus
		pn.m_List->SetFocus();	// avoid hidden list with focus
	pn.m_List->ShowWindow(SW_SHOW);	// show new list
}

void CFileBrowserControlBar::ResizeCtrls()
{
	enum {
		LIST_BORDER = 3,	// border around list control
		TAB_BORDER = 2		// space between tabs and list
	};
	CRect	cr;
	GetClientRect(cr);
	if (m_CurSel >= 0) {
		CRect	tir, lr(cr);	// tab item rect, list rect
		m_TabCtrl.GetItemRect(0, tir);
		lr.DeflateRect(LIST_BORDER, LIST_BORDER);
		lr.top += tir.Height() + TAB_BORDER;
			GetCurList()->MoveWindow(lr);
	}
	// to reduce flicker, resize tab control after list and use defer erase
	m_TabCtrl.SetWindowPos(NULL, cr.left, cr.top,
		cr.Width(), cr.Height(), SWP_NOZORDER | SWP_DEFERERASE);
}

void CFileBrowserControlBar::GetPaneState(int PaneIdx, PANE_STATE& State) const
{
	const CFileBrowserListCtrl&	list = GetList(PaneIdx);
	State.SortCol = list.GetSortCol();
	State.SortDir = list.GetSortDir();
	State.ViewType = list.GetViewType();
	for (int i = 0; i < COLUMNS; i++)
		State.ColWidth[i] = list.GetColumnWidth(i);
	ListView_GetColumnOrderArray(list.m_hWnd, COLUMNS, State.ColOrder);
}

void CFileBrowserControlBar::SetPaneState(int PaneIdx, const PANE_STATE& State)
{
	CFileBrowserListCtrl&	list = GetList(PaneIdx);
	list.SetSort(State.SortCol, State.SortDir);
	list.SetViewType(State.ViewType);
	for (int i = 0; i < COLUMNS; i++) {
		int	width = State.ColWidth[i];
		if (width)	// only set non-zero width; else restore default
			list.SetColumnWidth(i, width);
	}
	ListView_SetColumnOrderArray(list.m_hWnd, COLUMNS, State.ColOrder);
}

void CFileBrowserControlBar::SetThumbSize(CSize Size)
{
	if (Size == GetThumbSize())
		return;	// nothing to do
	int	panes = GetPaneCount();
	for (int i = 0; i < panes; i++)
		GetList(i).SetThumbSize(Size);
}

void CFileBrowserControlBar::SetCacheThumbs(bool Enable)
{
	if (Enable == m_CacheThumbs)
		return;	// nothing to do
	CThumbCache	*tc = Enable ? &m_ThumbCache : NULL;
	int	panes = GetPaneCount();
	for (int i = 0; i < panes; i++)
		GetList(i).SetThumbCache(tc);
	m_CacheThumbs = Enable;
}

BEGIN_MESSAGE_MAP(CFileBrowserControlBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CFileBrowserControlBar)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_FB_TAB, OnSelchangeTab)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_RETURN, IDC_FB_LIST, OnReturnList)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserControlBar message handlers

LRESULT CFileBrowserControlBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	CRect	r(0, 0, 100, 100);	// arbitrary initial size of controls
	m_TabCtrl.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, r, this, IDC_FB_TAB);
	m_TabCtrl.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	if (IsFloating())
		PostMessage(WM_SIZE);	// defer resize
	return TRUE;
}

void CFileBrowserControlBar::OnDestroy() 
{
	m_Pane.RemoveAll();
	CMySizingControlBar::OnDestroy();
}

void CFileBrowserControlBar::OnSize(UINT nType, int cx, int cy) 
{
	if (m_TabCtrl.m_hWnd) {
		ResizeCtrls();
		// set sized flag for current pane; for all other panes, clear it
		int	panes = GetPaneCount();
		for (int i = 0; i < panes; i++)
			m_Pane[i].m_ListSized = (i == m_CurSel);
		m_CtrlsSized = TRUE;
	}
	CMySizingControlBar::OnSize(nType, cx, cy);
}

void CFileBrowserControlBar::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	sel = m_TabCtrl.GetCurSel();
	if (sel >= 0)
		SetCurPane(sel);
	*pResult = 0;
}

void CFileBrowserControlBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (m_CurSel >= 0) {
		CFileBrowserListCtrl	*lp = GetCurList();
		if (point.x == -1 && point.y == -1) {	// if triggered via keyboard
			CRect	r;
			GetWindowRect(r);
			point = r.TopLeft();
			point += CSize(10, 10);	// offset looks better
		}
		NMLISTVIEW	nmh;
		ZeroMemory(&nmh, sizeof(nmh));
		nmh.hdr.hwndFrom = lp->GetSafeHwnd();	// set common header
		nmh.hdr.idFrom = lp->GetDlgCtrlID();
		nmh.hdr.code = FBLCN_CONTEXTMENU;
		nmh.iItem = -1;
		nmh.ptAction = point;
		lp->ScreenToClient(&nmh.ptAction);
		SendMessage(WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
	}
}

void CFileBrowserControlBar::OnReturnList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_CurSel >= 0)
		GetCurList()->OpenSelectedItems();
	*pResult = 0;
}
