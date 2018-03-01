// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      05oct07	initial version
		01		29jan08	comment out unused wizard-generated locals
		02		28feb08 in FormatTime, remove extra GetDateFormat call
		03		25feb09	check GetItemRect return value
		04		25feb09	in SetViewType, must set item count
        05		27feb09	in CalcMinColumnWidth, restore DC's previous font
		06		06jan10	W64: in OnOdfinditem, cast find string length to 32-bit
		07		24apr10	in FormatSize, use CString to fix Unicode warning
		08		24apr10	use CPair for map positions
		09		28apr11	disable main accelerators while editing label
		10		19jul11	add delete file list
		11		11nov11	fix keyboard-triggered context menu
		12		29may12	in SetThumbSize, make rebuild thumbs unconditional

		file browser list control
 
*/

// FileBrowserListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FileBrowserListCtrl.h"
#include "ThumbThread.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserListCtrl

IMPLEMENT_DYNAMIC(CFileBrowserListCtrl, CListCtrl);

const CFileBrowserListCtrl::COL_INFO CFileBrowserListCtrl::m_ColInfo[COLUMNS] = {
	{IDS_FB_COL_NAME,		LVCFMT_LEFT,	150},
	{IDS_FB_COL_SIZE,		LVCFMT_RIGHT,	80},
	{IDS_FB_COL_TYPE,		LVCFMT_LEFT,	120},
	{IDS_FB_COL_MODIFIED,	LVCFMT_LEFT,	120}
};

DWORD	CFileBrowserListCtrl::m_DisplayBitsPerPel;
CFileInfoCache	CFileBrowserListCtrl::m_FileInfoCache;	// assumes only one UI thread

CFileBrowserListCtrl::CFileBrowserListCtrl()
{
	m_CachedItemIdx = 0;
	m_ViewType = -1;
	m_SortCol = DEFAULT_SORT_COL;
	m_SortDir = DEFAULT_SORT_DIR;
	m_DragItem = 0;
	m_IsDragging = FALSE;
	m_LabelEdit = FALSE;
	m_ThumbsValid = FALSE;
	m_ThumbSize = CSize(96, 72);
	m_ColorDepth = 32;
	m_ThumbCache = NULL;
}

CFileBrowserListCtrl::~CFileBrowserListCtrl()
{
}

void CFileBrowserListCtrl::Init()
{
	for (int i = 0; i < COLUMNS; i++) {	// make columns
		const COL_INFO&	info = m_ColInfo[i];
		InsertColumn(i, LDS(info.Title), info.Align, info.Width);
	}
	SetImageList(&m_FileInfoCache.GetImageList(ICON_SMALL), LVSIL_SMALL);
	SetImageList(&m_FileInfoCache.GetImageList(ICON_BIG), LVSIL_NORMAL);
	// create header image list containing sort arrows
	m_HdrImgList.Create(8, 8, ILC_MASK, 1, 1);
	m_HdrImgList.Add(AfxGetApp()->LoadIcon(IDI_SORT_UP));
	m_HdrImgList.Add(AfxGetApp()->LoadIcon(IDI_SORT_DOWN));
	GetHeaderCtrl()->SetImageList(&m_HdrImgList);
	DrawSortArrow(DEFAULT_SORT_COL, DEFAULT_SORT_DIR);
	if (m_DisplayBitsPerPel)	// if static bit count is already set
		m_ColorDepth = m_DisplayBitsPerPel;	// no need to query display driver
	else {	// try to get bit count from display driver
		DEVMODE	dm;
		if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
			m_DisplayBitsPerPel = dm.dmBitsPerPel;	// set static bit count
			m_ColorDepth = m_DisplayBitsPerPel;
		}
	}
	m_ViewType = GetStyle() & LVS_TYPEMASK;
}

void CFileBrowserListCtrl::Notify(UINT code, int nItem, CPoint pt) const
{
	NMLISTVIEW	nmh;
	ZeroMemory(&nmh, sizeof(nmh));
	nmh.hdr.hwndFrom = GetSafeHwnd();	// set common header
	nmh.hdr.idFrom = GetDlgCtrlID();
	nmh.hdr.code = code;
	nmh.iItem = nItem;
	nmh.ptAction = pt;
	GetParent()->SendMessage(WM_NOTIFY, nmh.hdr.idFrom, (LPARAM)&nmh);
}

void CFileBrowserListCtrl::Notify(UINT code, PVOID pnm) const
{
	LPNMHDR	pnmhdr = (LPNMHDR)pnm;
	pnmhdr->hwndFrom = GetSafeHwnd();	// set common header
	pnmhdr->idFrom = GetDlgCtrlID();
	pnmhdr->code = code;
	GetParent()->SendMessage(WM_NOTIFY, pnmhdr->idFrom, (LPARAM)pnm);
}

void CFileBrowserListCtrl::DrawSortArrow(int Col, int Dir)
{
	HDITEM	hdi;
	hdi.mask = HDI_IMAGE | HDI_FORMAT;
	GetHeaderCtrl()->GetItem(Col, &hdi);
	if (Dir < 0) {	// erase arrow
		hdi.mask = HDI_FORMAT;
		hdi.fmt &= ~HDF_IMAGE;
	} else {
		hdi.mask = HDI_FORMAT | HDI_IMAGE;
		hdi.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
		hdi.iImage = Dir;
	}
	GetHeaderCtrl()->SetItem(Col, &hdi);
}

void CFileBrowserListCtrl::SetViewType(int Type)
{
	if (Type == m_ViewType)
		return;	// nothing to do
	int	PrevType = m_ViewType;
	m_ViewType = Type;	// order matters; UpdateThumbs uses m_ViewType
	DWORD	Style;
	CImageList	*ImgList;
	if (Type == VTP_THUMBNAIL) {
		Style = LVS_ICON;
		ImgList = &m_ThumbImgList;
		if (m_ThumbImgList.m_hImageList == NULL)	// if image list not created
			m_ThumbImgList.Create(m_ThumbSize.cx, m_ThumbSize.cy, ILC_COLORDDB, 1, 1);
		UpdateThumbs(FALSE);	// preserve existing thumbnails if possible
	} else {
		m_ThumbThread.Kill();	// tell thumbnail thread to exit
		Style = Type;
		ImgList = &m_FileInfoCache.GetImageList(ICON_BIG);
	}
	SetImageList(ImgList, LVSIL_NORMAL);
	if ((Type == VTP_ICON || Type == VTP_THUMBNAIL)
	&& (PrevType == VTP_ICON || PrevType == VTP_THUMBNAIL))
		SetItemCountEx(m_DirList.GetCount());	// else scroll bar doesn't update
	ModifyStyle(LVS_TYPEMASK, Style);
	if (Type == VTP_SMALLICON || Type == VTP_LIST)
		SetColumnWidth(0, CalcMinColumnWidth(0));
	// 25feb09: must reset item count, else scroll bar doesn't always update
	SetItemCountEx(m_DirList.GetCount());
}

void CFileBrowserListCtrl::SetSort(int Col, int Dir)
{
	if (Col == m_SortCol && Dir == m_SortDir)	// if same column and direction
		return;	// nothing to do
	if (Col != m_SortCol && m_SortCol >= 0)	// if different column has arrow
		DrawSortArrow(m_SortCol, -1);	// remove arrow from previous column
	if (Col >= 0) {	// if valid sort column
		int	items = m_DirList.GetCount();
		int	selcnt = GetSelectedCount();
		int	selmark = GetSelectionMark();
		if (selcnt || selmark >= 0) {	// if there's a selection
			// save item states and selection mark in list user data
			for (int i = 0; i < items; i++) {
				CDirItem&	item = m_DirList.GetItem(i);
				item.SetData(GetItemState(i, LVIS_SELECTED | LVIS_FOCUSED));
			}
			if (selmark >= 0) {
				CDirItem&	item = m_DirList.GetItem(selmark);
				// repurpose an item state bit to indicate selection mark
				item.SetData(item.GetData() | LVIS_ACTIVATING);
			}
		}
		DrawSortArrow(Col, Dir);	// draw appropriate arrow on new column
		m_DirList.Sort(Col, Dir);	// sort directory list
		UpdateThumbs();
		if (selcnt || selmark >= 0) {	// if there's a selection
			// restore item states and selection mark from list user data
			for (int i = 0; i < items; i++) {
				const CDirItem&	item = m_DirList.GetItem(i);
				DWORD	state = item.GetData();
				SetItemState(i, state, LVIS_SELECTED | LVIS_FOCUSED);
				if (state & LVIS_ACTIVATING)	// if item has selection mark
					SetSelectionMark(i);
			}
		}
		Invalidate();
	}
	m_SortCol = Col;
	m_SortDir = Dir;
}

void CFileBrowserListCtrl::SetFolder(LPCTSTR Path)
{
	if (!Path[0] || !m_DirList.List(Path, &m_FiltExt))
		m_DirList.ListDrives();
	if (m_SortCol >= 0)
		m_DirList.Sort(m_SortCol, m_SortDir);
	if (m_ViewType == VTP_SMALLICON || m_ViewType == VTP_LIST)
		SetColumnWidth(0, CalcMinColumnWidth(0));	// order matters
	int	items = m_DirList.GetCount();
	SetItemCountEx(0);	// delete item states
	SetItemCountEx(items);
	UpdateThumbs();
	m_CachedItemIdx = -1;	// invalidate cached file info
}

void CFileBrowserListCtrl::Refresh()
{
	SetFolder(GetFolder());
}

void CFileBrowserListCtrl::SelectItem(int ItemIdx, bool Enable)
{
	int	state = LVIS_SELECTED | LVIS_FOCUSED;
	SetItemState(ItemIdx, Enable ? state : 0, state);
}

void CFileBrowserListCtrl::SelectAll()
{
	int	items = m_DirList.GetCount();
	for (int i = 0; i < items; i++)
		SelectItem(i, TRUE);
}

void CFileBrowserListCtrl::ClearSelection()
{
	POSITION	pos = GetFirstSelectedItemPosition();
	while (pos) {
		int	idx = GetNextSelectedItem(pos);
		SetItemState(idx, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

int CFileBrowserListCtrl::GetFirstSelectedItem() const
{
	POSITION	pos = GetFirstSelectedItemPosition();
	if (pos != NULL)
		return(GetNextSelectedItem(pos));
	return(-1);	// no selection
}

void CFileBrowserListCtrl::GetSelectedItems(CStringArray& Path) const
{
	int	selcnt = GetSelectedCount();
	Path.SetSize(selcnt);
	POSITION	pos = GetFirstSelectedItemPosition();
	for (int i = 0; i < selcnt; i++)
		Path[i] = GetItemPath(GetNextSelectedItem(pos));
}

void CFileBrowserListCtrl::GetSelectedItems(CDirItemArray& DirItem) const
{
	int	selcnt = GetSelectedCount();
	DirItem.SetSize(selcnt);
	POSITION	pos = GetFirstSelectedItemPosition();
	for (int i = 0; i < selcnt; i++)
		DirItem[i] = GetDirItem(GetNextSelectedItem(pos));
}

void CFileBrowserListCtrl::GetSelection(CDWordArray& SelIdx) const
{
	int	selcnt = GetSelectedCount();
	SelIdx.SetSize(selcnt);
	POSITION	pos = GetFirstSelectedItemPosition();
	for (int i = 0; i < selcnt; i++)
		SelIdx[i] = GetNextSelectedItem(pos);
}

void CFileBrowserListCtrl::OpenSelectedItems()
{
	POSITION	pos = GetFirstSelectedItemPosition();
	while (pos != NULL && GetSelectedCount())	// opening a folder clears selection
		OpenItem(GetNextSelectedItem(pos));
}

void CFileBrowserListCtrl::OpenItem(int ItemIdx)
{
	// copy item instead of referencing it, because SetFolder changes m_DirList
	CDirItem	Item = m_DirList.GetItem(ItemIdx);
	CString	ItemPath = GetItemPath(ItemIdx);	// grab item path now too
	if (Item.IsDir()) {
		// if user selected parent folder and current folder is a root folder
		if (Item.IsDots() && PathIsRoot(GetFolder()))
			SetFolder(_T(""));	// list drives
		else
			SetFolder(ItemPath);
	}
	NMFBOPENITEM	nmoi;
	nmoi.pszPath = ItemPath;
	nmoi.bIsDir = Item.IsDir();
	Notify(FBLCN_OPENITEM, &nmoi);
}

void CFileBrowserListCtrl::CancelDrag()
{
	if (m_IsDragging) {
		m_IsDragging = FALSE;
		ReleaseCapture();
	}
}

void CFileBrowserListCtrl::SetThumbCache(CThumbCache *Cache)
{
	m_ThumbCache = Cache;
	if (m_ThumbCache != NULL)
		m_ThumbCache->SetResolution(m_ThumbSize, m_ColorDepth);
}

void CFileBrowserListCtrl::UpdateThumbResolution()
{
	if (m_ThumbCache != NULL) 
		m_ThumbCache->SetResolution(m_ThumbSize, m_ColorDepth);
	if (m_ThumbImgList.m_hImageList != NULL) {	// if image list is created
		m_ThumbImgList.DeleteImageList();	// recreate it
		m_ThumbImgList.Create(m_ThumbSize.cx, m_ThumbSize.cy, ILC_COLORDDB, 1, 1);
	}
}

void CFileBrowserListCtrl::SetThumbSize(CSize ThumbSize)
{
	if (ThumbSize == m_ThumbSize)
		return;	// nothing to do
	m_ThumbSize = ThumbSize;
	UpdateThumbResolution();
	m_ThumbsValid = FALSE;	// force rebuild all thumbnails, regardless of view type
	if (m_ViewType == VTP_THUMBNAIL) {
		// list control ignores new icon size unless we change view type
		SetViewType(VTP_SMALLICON);
		SetViewType(VTP_THUMBNAIL);
	}
}

void CFileBrowserListCtrl::SetColorDepth(DWORD ColorDepth)
{
	if (m_ColorDepth == ColorDepth)
		return;	// nothing to do
	m_ColorDepth = ColorDepth;
	UpdateThumbResolution();
	UpdateThumbs();
}

void CFileBrowserListCtrl::UpdateThumbs(bool Rebuild)
{
	m_ThumbThread.Kill();	// tell thumbnail thread to exit
	if (m_ViewType != VTP_THUMBNAIL) {
		if (Rebuild)
			m_ThumbsValid = FALSE;
		return;
	}
	int	items = m_DirList.GetCount();
	if (Rebuild || !m_ThumbsValid) {
		m_ThumbImgList.SetImageCount(0);	// destroy previous images
		m_ThumbImgList.SetImageCount(items);
		m_GotThumb.RemoveAll();
		m_GotThumb.SetSize(items);
		m_ThumbPos.RemoveAll();
		m_ThumbPos.SetSize(items);
	}
	CWaitCursor	wc;	// thumbnail I/O can take a while
	if (m_ThumbCache != NULL)
		m_ThumbCache->SetFolder(GetFolder());
	m_ThumbThread.SetItemCount(items);	// allocate thread parameters
	int	Extractions = 0;
	for (int i = 0; i < items; i++) {
		const CDirItem&	item = m_DirList.GetItem(i);
		if (!m_GotThumb[i] && !item.IsDir()) {
			CThumbPos	pos = m_ThumbCache != NULL ?
				m_ThumbCache->GetThumb(item.GetName()) : NULL;
			// if caching and item is mapped and last write time matches
			if (pos != NULL
			&& pos->value.GetLastWrite() == item.GetLastWrite()) {
				m_ThumbPos[i] = pos;	// thumbnail is cached; save link
			} else {	// thumbnail needs extracting
				m_ThumbThread.SetItemName(i, item.GetName());
				Extractions++;
			}
		}
	}
	if (Extractions)	// if thumbnails need to be extracted, launch thread
		m_ThumbThread.Launch(m_hWnd, GetFolder(), m_ThumbSize, m_ColorDepth);
	else
		m_ThumbThread.SetItemCount(0);	// delete thread parameters
	m_ThumbsValid = TRUE;
}

bool CFileBrowserListCtrl::MakeThumbFromIcon(int ItemIdx)
{
	static const COLORREF ThumbFrameColor = RGB(192, 192, 192);
	CFileInfo	FileInfo;
	const CDirItem&	item = m_DirList.GetItem(ItemIdx);
	if (!m_FileInfoCache.GetFileInfo(GetFolder(), item, FileInfo))
		return(FALSE);
	bool	retc = FALSE;
	CDC	*pDC = GetDC();
	CDC dcMem;
	if (dcMem.CreateCompatibleDC(pDC)) {
		CBitmap	bmp;
		if (bmp.CreateCompatibleBitmap(pDC, m_ThumbSize.cx, m_ThumbSize.cy)) {
			CBitmap	*pOldBitmap = dcMem.SelectObject(&bmp);
			CSize	IconSize = CSize(32, 32);
			CPoint	DstOrg((m_ThumbSize.cx - IconSize.cx) / 2,
				(m_ThumbSize.cy - IconSize.cy) / 2);
			CRect	r(0, 0, m_ThumbSize.cx, m_ThumbSize.cy);
			dcMem.FillSolidRect(r, GetSysColor(COLOR_WINDOW));
			CBrush	br;
			br.CreateSolidBrush(ThumbFrameColor);
			dcMem.FrameRect(r, &br);
			CImageList&	il = m_FileInfoCache.GetImageList(ICON_BIG);
			int	IconIdx = FileInfo.GetIconIdx();
			if (il.DrawIndirect(&dcMem, IconIdx, DstOrg, IconSize, CPoint(0, 0))) {
				dcMem.SelectObject(pOldBitmap);	// deselect bmp before image list use
				ReleaseDC(pDC);
				pDC = NULL;	// mark device context deleted
				m_ThumbImgList.Replace(ItemIdx, &bmp, NULL);
				m_GotThumb[ItemIdx] = TRUE;
				Update(ItemIdx);
				retc = TRUE;
			}
		}
	}
	if (pDC != NULL)
		ReleaseDC(pDC);
	return(retc);
}

int CFileBrowserListCtrl::CalcMinColumnWidth(int Col)
{
	enum {
		BORDER = 6,	// minimal spacing; less causes abbreviated text
		SLACK = 3	// prevents widest item from touching right edge
	};
	CWaitCursor	wc;	// iterating all items can be slow, especially for file type
	CClientDC	dc(this);
	HGDIOBJ	PrevFont = dc.SelectObject(GetFont());	// must use list control's font
	int	width = 0;
	CSize	sz;
	CFileInfo	FileInfo;
	CString	str;
	int	items = m_DirList.GetCount();
	for (int i = 0; i < items; i++) {
		const CDirItem&	item = m_DirList.GetItem(i);
		switch (Col) {
		case COL_NAME:
			str = item.GetName();
			break;
		case COL_SIZE:
			if (item.IsDir())
				continue;
			FormatSize(item.GetLength(), str);
			break;
		case COL_TYPE:	// slow if we have many unique file types that aren't cached
			m_FileInfoCache.GetFileInfo(GetFolder(), item, FileInfo);
			str = FileInfo.GetTypeName();
			break;
		case COL_MODIFIED:
			if (item.GetLastWrite() == 0)
				continue;
			FormatTime(item.GetLastWrite(), str);
			break;
		default:
			NODEFAULTCASE;
		}
		GetTextExtentPoint32(dc.m_hDC, str, str.GetLength(), &sz);
		if (sz.cx > width)
			width = sz.cx;
	}
	dc.SelectObject(PrevFont);	// restore DC's previous font
	// 25feb09: GetItemRect can fail e.g. if list is empty, in which case we
	// must avoid adding garbage to column width
	CRect	IconRect;
	if (GetItemRect(0, IconRect, LVIR_ICON))
		width += IconRect.Width();
	else	// can't get item rect, fall back to system metrics
		width += GetSystemMetrics(m_ViewType == VTP_ICON ? SM_CXICON : SM_CXSMICON);
	width += BORDER + SLACK;
	return(width);
}

bool CFileBrowserListCtrl::FormatTime(CTime Time, CString& Str)
{
	static const int MAX_STRING = 64;	// maximum date or time string
	SYSTEMTIME	st;
	Time.GetAsSystemTime(st);
	TCHAR	DateStr[MAX_STRING], TimeStr[MAX_STRING];
	if (!GetDateFormat(0, 0, &st, NULL, DateStr, MAX_STRING))
		return(FALSE);
	if (!GetTimeFormat(0, TIME_NOSECONDS, &st, NULL, TimeStr, MAX_STRING))
		return(FALSE);
	Str = CString(DateStr) + " " + TimeStr;
	return(TRUE);
}

bool CFileBrowserListCtrl::FormatSize(LONGLONG Size, CString& Str)
{
	static const NUMBERFMT	NumFmt = {0, 0, 3, _T("."), _T(","), 0};	// no decimal places
	static const int MAX_STRING = 64;	// maximum number string
	CString	ValStr;
	ValStr.Format(_T("%I64d"), (Size >> 10) + ((Size & 0x3ff) != 0));	// to KB, rounding up
	TCHAR	CommaStr[MAX_STRING];
	if (!GetNumberFormat(0, 0, ValStr, &NumFmt, CommaStr, MAX_STRING))
		return(FALSE);
	Str = CString(CommaStr) + " KB";
	return(TRUE);
}

void CFileBrowserListCtrl::OpenParentFolder()
{
	CPathStr	path(GetFolder());
	if (PathIsRoot(path))
		path.Empty();	// show drive list
	else
		path.Append(_T(".."));
	SetFolder(path);
}

bool CFileBrowserListCtrl::CanRename()
{
	int	sel = GetSelectionMark();
	if (sel >= 0) {
		const CDirItem& item = GetDirItem(sel);
		if (!(item.IsDrive() || item.IsDots()))
			return(TRUE);
	}
	return(FALSE);
}

bool CFileBrowserListCtrl::Rename()
{
	return(EditLabel(GetSelectionMark()) != NULL);
}

bool CFileBrowserListCtrl::RenameFile(HWND hWnd, LPCTSTR OldName, LPCTSTR NewName, FILEOP_FLAGS Flags)
{
	// pFrom/pTo can contain multiple names and must be double null-terminated
	CString	sFrom = OldName;
	CString	sTo = NewName;
	sFrom.Insert(sFrom.GetLength(), TCHAR(0));	// add extra terminator
	sTo.Insert(sTo.GetLength(), TCHAR(0));	// add extra terminator
	SHFILEOPSTRUCT	op;
	ZeroMemory(&op, sizeof(op));
	op.pFrom = sFrom;
	op.pTo = sTo;
	op.hwnd = hWnd;
	op.wFunc = FO_RENAME;
	op.fFlags = Flags;
	if (SHFileOperation(&op))	// non-zero result means failure
		return(FALSE);
	// if the rename causes a confirmation dialog and the user cancels, 
	// SHFileOperation returns success though the file was NOT renamed
	return(PathFileExists(NewName) != 0);	// verify that rename succeeded
}

bool CFileBrowserListCtrl::DeleteFileList(HWND hWnd, const CStringArray& List, FILEOP_FLAGS Flags)
{
	CString	sFrom;
	int	items = INT64TO32(List.GetSize());
	for (int i = 0; i < items; i++) {
		sFrom += List[i];
		sFrom += TCHAR(0);	// add terminator
	}
	SHFILEOPSTRUCT	op;
	ZeroMemory(&op, sizeof(op));
	op.pFrom = sFrom;
	op.hwnd = hWnd;
	op.wFunc = FO_DELETE;
	op.fFlags = Flags;
	return(!SHFileOperation(&op));	// non-zero result means failure
}

BEGIN_MESSAGE_MAP(CFileBrowserListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CFileBrowserListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_ODCACHEHINT, OnOdcachehintList)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclickList)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclkList)
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_ODFINDITEM, OnOdfinditem)
	ON_WM_CHAR()
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	//}}AFX_MSG_MAP
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnDividerdblclick)	// handle both ANSI
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnDividerdblclick)	// and UNICODE formats
	ON_MESSAGE(UWM_EXTRACTTHUMB, OnExtractThumb)
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileBrowserListCtrl message handlers

int CFileBrowserListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	Init();
	return 0;
}

void CFileBrowserListCtrl::PreSubclassWindow() 
{
	// if we were created from a dialog resource, OnCreate never gets called
	// because creation occurred before our CWnd was attached, so we must init
	// from here instead; we identify this case by existence of header control
	if (ListView_GetHeader(m_hWnd) != NULL)	// if header control exists
		Init();	// assume OnCreate won't be called
	CListCtrl::PreSubclassWindow();
}

void CFileBrowserListCtrl::OnDestroy() 
{
	m_ThumbThread.Cleanup(m_hWnd);
	CListCtrl::OnDestroy();
}

void CFileBrowserListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM&	ListItem = pDispInfo->item;
	int	ItemIdx = ListItem.iItem;
	if (ItemIdx < m_DirList.GetCount()) {
		const CDirItem&	DirItem = m_DirList.GetItem(ItemIdx);
		if (ItemIdx != m_CachedItemIdx) {	// if item isn't cached
			m_FileInfoCache.GetFileInfo(GetFolder(), DirItem, m_MRUFileInfo);
			m_CachedItemIdx = ItemIdx;
		}
		if (ListItem.mask & LVIF_TEXT) {
			switch (ListItem.iSubItem) {
			case COL_NAME:
				_tcscpy(ListItem.pszText, DirItem.GetName());
				break;
			case COL_SIZE:
				if (!DirItem.IsDir()) {
					CString	s;
					if (FormatSize(DirItem.GetLength(), s))
						_tcscpy(ListItem.pszText, s);
				}
				break;
			case COL_TYPE:
				_tcscpy(ListItem.pszText, m_MRUFileInfo.GetTypeName());
				break;
			case COL_MODIFIED:
				if (DirItem.GetLastWrite() > 0) {
					CString	s;
					if (FormatTime(DirItem.GetLastWrite(), s))
						_tcscpy(ListItem.pszText, s);
				}
				break;
			default:
				NODEFAULTCASE;
			}
		}
		if (ListItem.mask & LVIF_IMAGE) {
			if (m_ViewType == VTP_THUMBNAIL) {
				if (m_GotThumb[ItemIdx])
					ListItem.iImage = ItemIdx;
				else {
					CThumbPos	pos = m_ThumbPos[ItemIdx];	// from previous GetAssoc
					if (pos != NULL) {	// if thumbnail is cached
						CBitmap	bmp;
						pos->value.LoadBitmap( bmp);
						m_ThumbImgList.Replace(ItemIdx, &bmp, NULL);
						m_GotThumb[ItemIdx] = TRUE;
						ListItem.iImage = ItemIdx;
					} else {
						if (MakeThumbFromIcon(ItemIdx))
							ListItem.iImage = ItemIdx;
					}
				}
			} else
				ListItem.iImage = m_MRUFileInfo.GetIconIdx();
		}
	}	
	*pResult = 0;
}

void CFileBrowserListCtrl::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* pNMListView = (NMLISTVIEW*)pNMHDR;
	int	col = pNMListView->iSubItem;
	int	dir = m_SortDir;
	if (col == m_SortCol)	// if same sort column
		dir ^= 1;	// flip sort direction
	else
		dir = 0;	// default order
	SetSort(col, dir);
	*pResult = 0;
}

void CFileBrowserListCtrl::OnOdcachehintList(NMHDR* pNMHDR, LRESULT* pResult)
{
//	NMLVCACHEHINT *pCachehint = (NMLVCACHEHINT *)pNMHDR;
	if (m_ViewType == VTP_THUMBNAIL) {
		LVFINDINFO	fi;
		fi.flags = LVFI_NEARESTXY;
		fi.pt = CPoint(0, 0);	// not accounting for icon spacing
		fi.vkDirection = VK_LEFT;
		int item = FindItem(&fi);
		if (item >= 0)
			m_ThumbThread.SetNextItem(item);	// extract this item next
	}
	*pResult = 0;
}

LRESULT CFileBrowserListCtrl::OnExtractThumb(WPARAM wParam, LPARAM lParam)
{
	HBITMAP	hBmp = (HBITMAP)wParam;
	CThumbThread::THUMB_RESULT *tr = (CThumbThread::THUMB_RESULT *)lParam;
	// only accept messages from the current thumbnail thread instance
	if (tr->JobId == m_ThumbThread.GetCurJobId()) {
		int	ItemIdx = tr->ItemIdx;
		if (hBmp != NULL) {	// if extraction succeeded
			CBitmap	bmp;
			bmp.Attach(hBmp);	// CBitmap dtor will delete bitmap handle
			if (m_ThumbImgList.Replace(ItemIdx, &bmp, NULL)) {
				m_GotThumb[ItemIdx] = TRUE;
				RedrawItems(ItemIdx, ItemIdx);	// update list control item
				const CDirItem& item = m_DirList.GetItem(ItemIdx);
				if (m_ThumbCache != NULL) {
					CThumb	thumb;
					if (thumb.StoreBitmap(bmp)) {
						thumb.SetLastWrite(item.GetLastWrite());
						m_ThumbCache->SetAt(item.GetName(), thumb);
					}
				}
			}
		} else	// extraction failed
			MakeThumbFromIcon(ItemIdx);	// use file icon instead
	} else {	// spurious message, presumably from a previous thread instance
		if (hBmp != NULL)
			DeleteObject(hBmp);	// delete bitmap handle
	}
	delete tr;	// delete thumbnail result
	return(0);
}

void CFileBrowserListCtrl::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW	*nmlv = (NMLISTVIEW *)pNMHDR;
	int	ItemIdx = nmlv->iItem;
	if (ItemIdx >= 0)
		OpenItem(ItemIdx);
	*pResult = 0;
}

void CFileBrowserListCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (point.x == -1 && point.y == -1) {	// if triggered via keyboard
		int	ItemIdx = GetSelectionMark();
		if (ItemIdx >= 0) {	// if valid selection mark
			CPoint	ItemPt;
			GetItemPosition(ItemIdx, &ItemPt);
			CRect	r;
			GetClientRect(r);
			if (!r.PtInRect(ItemPt))	// if item outside client rect
				ItemPt = r.TopLeft();		// default to client top left
			ItemPt += CSize(10, 10);	// don't entirely cover item
			Notify(FBLCN_CONTEXTMENU, ItemIdx, ItemPt);
		} else	// no selection mark
			CListCtrl::OnContextMenu(pWnd, point);
	} else {	// triggered via mouse
		LVHITTESTINFO	hti;
		hti.pt = point;
		ScreenToClient(&hti.pt);
		int	ItemIdx = HitTest(&hti);
		if (hti.flags & (LVHT_NOWHERE | LVHT_ONITEM))
			Notify(FBLCN_CONTEXTMENU, ItemIdx, hti.pt);
		else
			CListCtrl::OnContextMenu(pWnd, point);
	}
}

void CFileBrowserListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* pNMListView = (NMLISTVIEW*)pNMHDR;
	m_IsDragging = TRUE;
	m_DragItem = pNMListView->iItem;
	SetCapture();	// let list capture input to avoid focus trouble
	Notify(FBLCN_DRAGBEGIN, pNMListView);
	*pResult = 0;
}

void CFileBrowserListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_IsDragging) {
		ClientToScreen(&point);	// list coords because list has capture
		Notify(FBLCN_DRAGMOVE, m_DragItem, point);
	}
	CListCtrl::OnMouseMove(nFlags, point);
}

void CFileBrowserListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_IsDragging) {
		m_IsDragging = FALSE;
		ReleaseCapture();
		ClientToScreen(&point);	// list coords because list has capture
		Notify(FBLCN_DRAGEND, m_DragItem, point);
	}	
	CListCtrl::OnLButtonUp(nFlags, point);
}

void CFileBrowserListCtrl::OnOdfinditem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVFINDITEM* pFindItem = (NMLVFINDITEM*)pNMHDR;
	int	retc = -1;
	int	flags = pFindItem->lvfi.flags;
	if (flags & LVFI_STRING) {
		int	len = INT64TO32(_tcslen(pFindItem->lvfi.psz));
		// according to MSDN, we should test the LVFI_PARTIAL flag and only do
		// a partial match if it's set, but in practice, we're ALWAYS passed a
		// partial string, and LVFI_PARTIAL is NEVER set; strange but it works
		int	count = m_DirList.GetCount();
		int	pos = pFindItem->iStart;
		LPCTSTR	target = pFindItem->lvfi.psz;
		for (int i = 0; i < count; i++) {
			if (pos >= count) {
				if (flags & LVFI_WRAP)
					pos = 0;	// continue search at the beginning
				else
					break;
			}
			const CDirItem& item = m_DirList.GetItem(pos);
			if (!item.GetName().Left(len).CompareNoCase(target)) {
				retc = pos;
				break;
			}
			pos++;
		}
	}
	*pResult = retc;
}

void CFileBrowserListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (_istprint(TCHAR(nChar))) {	// if printable character, including space
		// finding items by key can't get started unless an item is focused
		if (GetSelectionMark() < 0 && m_DirList.GetCount())	// if no focused item
			SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);	// focus first item
	}
	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}

LRESULT CFileBrowserListCtrl::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	m_DisplayBitsPerPel = wParam;	// set static bit count
	SetColorDepth(wParam);	// update thumbnail cache and image list
	return TRUE;
}

void CFileBrowserListCtrl::OnDividerdblclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *)pNMHDR;
	UINT	col = phdn->iItem;
	SetColumnWidth(col, CalcMinColumnWidth(col));
	*pResult = 0;
}

BOOL CFileBrowserListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	// The list control is supposed to send an NM_RETURN notification to the
	// parent window if the Enter key is pressed, but it doesn't. This is a
	// known bug, and some folks work around it by handling WM_GETDLGCODE and
	// returning DLGC_WANTALLKEYS. This solution sucks, because it makes the
	// list control EAT system keys, which causes unexpected behavior in the 
	// parent window, e.g. if the parent is a dialog, the dialog navigation 
	// keys won't work while the list control has input focus. It's better to
	// detect Enter here, send NM_RETURN, and then dispatch the key as usual.
	if (pMsg->message == WM_KEYDOWN) {
		if (m_LabelEdit) {	// while editing label
			if (pMsg->wParam == VK_ESCAPE)
				m_LabelEdit = FALSE;
			// disable main accelerators so edit keys behave as expected
			TranslateMessage(pMsg);
			DispatchMessage(pMsg);
			return TRUE;	// don't dispatch further
		}
		switch (pMsg->wParam) {
		case VK_RETURN:
			{
				CWnd	*wp = GetParent();
				if (wp != NULL) {
					NMHDR	nmh;
					nmh.hwndFrom = m_hWnd;
					nmh.idFrom = GetDlgCtrlID();
					nmh.code = NM_RETURN;
					wp->SendMessage(WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
				}
			}
			break;
		case VK_ESCAPE:
			CancelDrag();
			break;
		}
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}

void CFileBrowserListCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	m_LabelEdit = TRUE;
	*pResult = 0;
}

void CFileBrowserListCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CEdit	*pEdit = GetEditControl();
	if (m_LabelEdit && pEdit != NULL) {	// if label edit wasn't canceled
		CString	NewName;
		pEdit->GetWindowText(NewName);	// label is new item name
		int	ItemIdx = pDispInfo->item.iItem;
		CDirItem&	item = m_DirList.GetItem(ItemIdx);
		if (NewName != item.GetName()) {	// if name is different
			CPathStr	NewPath(GetFolder());
			NewPath.Append(NewName);	// make new item path
			CString	OldPath(GetItemPath(ItemIdx));
			if (RenameFile(m_hWnd, OldPath, NewPath, FOF_SILENT)) {
				item.SetName(NewName);	// update item name
				NMFBRENAMEITEM	nmri;
				nmri.pszOldPath = OldPath;
				nmri.pszNewPath = NewPath;
				Notify(FBLCN_RENAMEITEM, &nmri);
			}
		}
	}
	m_LabelEdit = FALSE;
	*pResult = 0;
}
