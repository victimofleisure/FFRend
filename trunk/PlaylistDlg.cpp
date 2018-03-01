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

// PlaylistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "PlaylistDlg.h"
#include "MultiFileDlg.h"
#include "PathStr.h"
#include "MissingFilesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlaylistDlg dialog

IMPLEMENT_DYNAMIC(CPlaylistDlg, CToolDlg);

// all dialog controls must have MFC objects and must be listed here
const CCtrlResize::CTRL_LIST CPlaylistDlg::m_CtrlList[] = {
	{IDC_PL_LIST,	BIND_ALL},
	{0, 0}	// list terminator
};

const int CPlaylistDlg::m_StateIcon[ITEM_STATES] = {	// must match item state enum
	IDI_PL_NORMAL,
	IDI_PL_STOPPED,
	IDI_PL_PLAYING,
};

// this array init must match list column enum
const CPlaylistDlg::LIST_COL CPlaylistDlg::m_ListCol[] = {
	{IDS_PL_COL_PROJECT,	LVCFMT_LEFT, 250},
};

#define PLAYLIST_HEADER _T("FFRendPlaylist %d")

#define RK_PL_AUTO_LOOP		_T("PlaylistAutoLoop")
#define RK_PL_AUTO_SHUFFLE	_T("PlaylistAutoShuffle")
#define RK_PL_AUTO_PERIOD	_T("PlaylistAutoPeriod")

CPlaylistDlg::CPlaylistDlg(CWnd* pParent /*=NULL*/)
	: CToolDlg(IDD_PLAYLIST, IDR_PLAYLIST, _T("PlaylistDlg"), pParent),
	m_RecentPlaylist(0, _T("Recent Playlists"), _T("Playlist%d"), MAX_RECENT_PLAYLISTS)
{
	//{{AFX_DATA_INIT(CPlaylistDlg)
	//}}AFX_DATA_INIT
	m_CurItem = -1;
	m_InTimer = FALSE;
	m_Playing = FALSE;
	m_Looping = CPersist::GetInt(REG_SETTINGS, RK_PL_AUTO_LOOP, TRUE) != 0;
	m_Shuffling = CPersist::GetInt(REG_SETTINGS, RK_PL_AUTO_SHUFFLE, FALSE) != 0;
	m_Modified = FALSE;
	m_Period = CPersist::GetInt(REG_SETTINGS, RK_PL_AUTO_PERIOD, DEFAULT_PERIOD);
	m_RecentPlaylist.ReadList();
}

CPlaylistDlg::~CPlaylistDlg()
{
	CPersist::WriteInt(REG_SETTINGS, RK_PL_AUTO_LOOP, m_Looping);
	CPersist::WriteInt(REG_SETTINGS, RK_PL_AUTO_SHUFFLE, m_Shuffling);
	CPersist::WriteInt(REG_SETTINGS, RK_PL_AUTO_PERIOD, m_Period);
	m_RecentPlaylist.WriteList();
}

bool CPlaylistDlg::ReadPlaylist(LPCTSTR Path)
{
	TRY {
		CStdioFile	fp(Path, CFile::modeRead);
		CString	s;
		fp.ReadString(s);
		int	Version;
		int	conv = _stscanf(s, PLAYLIST_HEADER, &Version);
		if (conv != 1 || Version > PLAYLIST_VERSION)
			AfxThrowArchiveException(CArchiveException::badIndex, Path);
		CStringArray	proj;
		while (fp.ReadString(s)) {
			s.TrimRight();
			if (!s.IsEmpty())
				proj.Add(s);
		}
		m_Project.Copy(proj);
		m_List.SetItemCountEx(proj.GetSize(), 0);
	}
	CATCH (CFileException, e) {
		e->ReportError();
		return(FALSE);
	}
	CATCH (CArchiveException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
};

bool CPlaylistDlg::WritePlaylist(LPCTSTR Path) const
{
	TRY {
		CStdioFile	fp(Path, CFile::modeCreate | CFile::modeWrite);
		CString	s;
		s.Format(PLAYLIST_HEADER, PLAYLIST_VERSION);
		fp.WriteString(s + '\n');
		int	items = GetItemCount();
		for (int i = 0; i < items; i++) {
			fp.WriteString(m_Project[i] + '\n');
		}
	}
	CATCH (CFileException, e) {
		e->ReportError();
		return(FALSE);
	}
	END_CATCH
	return(TRUE);
};

void CPlaylistDlg::UpdateList()
{
	int	items = GetItemCount();
	m_List.DeleteAllItems();
	for (int i = 0; i < items; i++)
		m_List.InsertItem(i, LPSTR_TEXTCALLBACK);
	UpdateStateIcon();
	m_RandList.Init(items);
}

void CPlaylistDlg::GetSelection(CDWordArray& Sel)
{
	Sel.RemoveAll();
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	while (pos != NULL) {
		int nItem = m_List.GetNextSelectedItem(pos);
		Sel.Add(nItem);
	}
}

void CPlaylistDlg::DeleteSelectedItems(int& CurPos)
{
	CDWordArray sel;
	GetSelection(sel);
	int	items = sel.GetSize();
	if (items) {
		// reverse iterate to maintain stability during deletion
		for (int i = items - 1; i >= 0; i--) {
			int	ItemIdx = sel[i];	// get position of selected item
			m_Project.RemoveAt(ItemIdx);	// delete item
			if (ItemIdx < CurPos)	// if deletion below current position
				CurPos--;	// compensate current position
			if (ItemIdx == m_CurItem)	// if deleting playing item
				m_CurItem = -1;	// no playing item
			else if (ItemIdx < m_CurItem)	// if deletion below playing item
				m_CurItem--;	// compensate playing item
		}
		UpdateList();
		m_Modified = TRUE;
	}
}

void CPlaylistDlg::SelectItem(int ItemIdx, bool Enable)
{
	int	state = LVIS_SELECTED | LVIS_FOCUSED;
	m_List.SetItemState(ItemIdx, Enable ? state : 0, state);
}

void CPlaylistDlg::ClearSelection()
{
	POSITION	pos = m_List.GetFirstSelectedItemPosition();
	while (pos) {
		int	ItemIdx = m_List.GetNextSelectedItem(pos);
		m_List.SetItemState(ItemIdx, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
}

void CPlaylistDlg::InsertItems(int ItemIdx, const CStringArray& Item)
{
	int	items = Item.GetSize();
	for (int i = 0; i < items; i++)
		m_Project.InsertAt(ItemIdx + i, Item[i]);
	if (ItemIdx <= m_CurItem)	// if inserting at or before playing item
		m_CurItem += items;	// compensate playing item
	UpdateList();
	m_Modified = TRUE;
	SelectItem(ItemIdx, TRUE);
	m_List.SetSelectionMark(ItemIdx);
}

bool CPlaylistDlg::PromptForProjects(CStringArray& Project) const
{
	UINT	flags = OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
	CString	filter((LPCTSTR)IDS_PROJECT_FILTER);
	CString	folder(theApp.GetMain()->GetFolder(CMainFrame::DIR_PROJECTS));
	CString	title((LPCTSTR)IDS_FB_PROJECTS);
	CMultiFileDlg	dlg(TRUE, PROJECT_EXT, NULL, flags, filter, NULL, title, &folder);
	if (dlg.DoModal() != IDOK)
		return(FALSE);
	dlg.GetPathArray(Project);
	return(TRUE);
}

bool CPlaylistDlg::NewPlaylist()
{
	if (!SaveCheck())
		return(FALSE);
	m_Project.RemoveAll();
	m_DocPath.Empty();
	m_CurItem = -1;
	m_Modified = FALSE;
	UpdateList();
	return(TRUE);
}

bool CPlaylistDlg::Open(LPCTSTR Path)
{
	if (!NewPlaylist())
		return(FALSE);
	if (!ReadPlaylist(Path))
		return(FALSE);
	CString	Filter((LPCTSTR)IDS_PROJECT_FILTER);
	CMissingFilesDlg	dlg(m_Project, PROJECT_EXT, Filter);
	if (dlg.Check() == IDCANCEL)	// if missing files and user canceled
		return(FALSE);
	// remove any empty items resulting from missing files
	int i = 0;
	while (i < GetItemCount()) {	// don't cache item count
		if (m_Project[i].IsEmpty())
			m_Project.RemoveAt(i);	// decrements item count
		else	// not empty
			i++;	// next item
	}
	m_DocPath = Path;
	m_RecentPlaylist.Add(Path);
	UpdateList();
	if (m_Playing) {
		OnTimer(AUTO_TIMER_ID);	// open a project
		SetPeriod(m_Period);	// reset automation timer
	}
	return(TRUE);
}

bool CPlaylistDlg::OpenRecent(int FileIdx)
{
	if (!NewPlaylist())
		return(FALSE);
	if (FileIdx < 0 || FileIdx >= m_RecentPlaylist.GetSize())
		return(FALSE);
	if (!Open(m_RecentPlaylist[FileIdx])) {
		m_RecentPlaylist.Remove(FileIdx);
		return(FALSE);
	}
	return(TRUE);
}

bool CPlaylistDlg::Save()
{
	if (m_DocPath.IsEmpty())
		return(SaveAs());
	if (!WritePlaylist(m_DocPath))
		return(FALSE);
	m_Modified = FALSE;
	return(TRUE);
}

bool CPlaylistDlg::SaveAs()
{
	CString	Path;
	bool	retc = theApp.GetMain()->PromptFile(
		CMainFrame::DIR_PLAYLIST, FALSE, NULL, NULL, Path);
	if (retc != IDOK)
		return(FALSE);
	if (!WritePlaylist(Path))
		return(FALSE);
	m_DocPath = Path;
	m_RecentPlaylist.Add(Path);
	m_Modified = FALSE;
	return(TRUE);
}

bool CPlaylistDlg::SaveCheck()
{
	if (!m_Modified || !theApp.GetMain()->GetOptions().GetSaveChgsWarn())
		return(TRUE);
	CString	msg, DocTitle;
	if (m_DocPath.IsEmpty())
		DocTitle = _T("Untitled");
	else
		DocTitle = PathFindFileName(m_DocPath);
	AfxFormatString1(msg, IDS_PL_SAVE_CHANGES, DocTitle);
	if (AfxMessageBox(msg, MB_YESNO) == IDNO)
		return(TRUE);
	return(Save());
}

void CPlaylistDlg::SetStateIcon(int ItemIdx, int State)
{
	m_List.SetItemState(ItemIdx, INDEXTOSTATEIMAGEMASK(State + 1), LVIS_STATEIMAGEMASK);
}

void CPlaylistDlg::UpdateStateIcon()
{
	if (m_CurItem >= 0) {
		int	NewState = m_Playing ? IS_PLAYING : IS_STOPPED;
		SetStateIcon(m_CurItem, NewState);
	}
}

void CPlaylistDlg::SetPeriod(int Period)
{
	m_Period = Period;
	m_PeriodEdit.SetVal(GetPeriodSecs());
	if (m_Playing)
		SetTimer(AUTO_TIMER_ID, Period, NULL);	// reset timer
}

int CPlaylistDlg::GetPeriodSecs() const
{
	return(m_Period / 1000);
}

void CPlaylistDlg::SetPeriodSecs(int PeriodSecs)
{
	PeriodSecs = CLAMP(PeriodSecs, 0, INT_MAX / 1000);	// avoid wrapping
	SetPeriod(PeriodSecs * 1000);
}

void CPlaylistDlg::SetPlay(bool Enable)
{
	if (Enable == m_Playing)
		return;	// nothing to do
	if (Enable) {
		SetTimer(AUTO_TIMER_ID, m_Period, NULL);	// start timer
		int	items = GetItemCount();
		if (items) {
			int	ItemIdx;
			if (m_Shuffling) {
				srand(GetTickCount());	// init random seed from tick count
				ItemIdx = m_RandList.GetNext();	// get first randomized item
			} else {	// sequential order
				if (m_CurItem >= 0) {
					ItemIdx = m_CurItem;
					m_CurItem = -1;	// spoof no-op test
				} else	// no current item
					ItemIdx = 0;	// play first item
			}
			OpenProject(ItemIdx);
		}
	} else
		KillTimer(AUTO_TIMER_ID);
	m_Playing = Enable;
	UpdateStateIcon();
	m_PlayBtn.SetCheck(Enable);
}

void CPlaylistDlg::SetLoop(bool Enable)
{
	if (Enable == m_Looping)
		return;	// nothing to do
	m_Looping = Enable;
	m_LoopBtn.SetCheck(Enable);
}

void CPlaylistDlg::SetShuffle(bool Enable)
{
	if (Enable == m_Shuffling)
		return;	// nothing to do
	m_Shuffling = Enable;
	m_ShuffleBtn.SetCheck(Enable);
}

bool CPlaylistDlg::OpenProject(int ItemIdx)
{
	if (ItemIdx == m_CurItem)
		return(TRUE);	// nothing to do
	if (ItemIdx >= 0) {
		if (!theApp.OpenDocumentFile(m_Project[ItemIdx]))
			return(FALSE);
	}
	if (m_CurItem >= 0)	// if playing item is valid
		SetStateIcon(m_CurItem, IS_NORMAL);	// erase its icon
	m_CurItem = ItemIdx;
	UpdateStateIcon();
	if (m_Playing && !m_InTimer)	// if autoplay and not in timer handler
		SetTimer(AUTO_TIMER_ID, m_Period, NULL);	// reset timer
	return(TRUE);
}

bool CPlaylistDlg::DropProject(LPCTSTR Path)
{
	if (!IsWindowVisible())	// if dialog hidden
		return(FALSE);
	CRect	r;
	GetWindowRect(r);
	CPoint	cursor;
	GetCursorPos(&cursor);
	if (!r.PtInRect(cursor))	// if cursor outside dialog
		return(FALSE);
	m_List.ScreenToClient(&cursor);
	int	ItemIdx = m_List.HitTest(cursor);
	if (ItemIdx < 0)	// if not over an item
		ItemIdx = GetItemCount();	// append
	CStringArray	proj;
	proj.Add(Path);
	InsertItems(ItemIdx, proj);
	return(TRUE);
}

void CPlaylistDlg::DoDataExchange(CDataExchange* pDX)
{
	CToolDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaylistDlg)
	DDX_Control(pDX, IDC_PL_PERIOD_SPIN, m_PeriodSpin);
	DDX_Control(pDX, IDC_PL_PERIOD_EDIT, m_PeriodEdit);
	DDX_Control(pDX, IDC_PL_SHUFFLE, m_ShuffleBtn);
	DDX_Control(pDX, IDC_PL_LOOP, m_LoopBtn);
	DDX_Control(pDX, IDC_PL_PLAY, m_PlayBtn);
	DDX_Control(pDX, IDC_PL_LIST, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPlaylistDlg, CToolDlg)
	//{{AFX_MSG_MAP(CPlaylistDlg)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_PL_LIST, OnGetdispinfoList)
	ON_NOTIFY(NM_DBLCLK, IDC_PL_LIST, OnDblclkList)
	ON_COMMAND(ID_PL_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_PL_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_PL_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_PL_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_PL_EDIT_INSERT, OnEditInsert)
	ON_COMMAND(ID_PL_AUTO_PLAY, OnAutoPlay)
	ON_COMMAND(ID_PL_AUTO_LOOP, OnAutoLoop)
	ON_COMMAND(ID_PL_AUTO_SHUFFLE, OnAutoShuffle)
	ON_WM_TIMER()
	ON_COMMAND(ID_PL_EDIT_ADD, OnEditAdd)
	ON_UPDATE_COMMAND_UI(ID_PL_AUTO_PLAY, OnUpdateAutoPlay)
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_PL_AUTO_LOOP, OnUpdateAutoLoop)
	ON_UPDATE_COMMAND_UI(ID_PL_AUTO_SHUFFLE, OnUpdateAutoShuffle)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_PL_EDIT_OPEN_PROJECT, OnEditOpenProject)
	ON_WM_SYSCOLORCHANGE()
	ON_COMMAND(ID_PL_FILE_NEW, OnFileNew)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_PL_LOOP, OnLoop)
	ON_BN_CLICKED(IDC_PL_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_PL_SHUFFLE, OnShuffle)
	ON_UPDATE_COMMAND_UI(ID_PL_EDIT_OPEN_PROJECT, OnUpdateEditDelete)
	ON_NOTIFY(ULVN_REORDER, IDC_PL_LIST, OnReorderList)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_PL_FILE_MRU_FILE1, ID_PL_FILE_MRU_FILE4, OnFileMru)
	ON_UPDATE_COMMAND_UI(ID_PL_FILE_MRU_FILE1, OnUpdateFileMru)
	ON_NOTIFY(NEN_CHANGED, IDC_PL_PERIOD_EDIT, OnChangedPeriodEdit)
	ON_WM_ENTERMENULOOP()
	ON_WM_EXITMENULOOP()
	ON_WM_MENUSELECT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaylistDlg message handlers

BOOL CPlaylistDlg::OnInitDialog() 
{
	CToolDlg::OnInitDialog();
	
	m_Resize.AddControlList(this, m_CtrlList);
	int	i;
	for (i = 0; i < COLS; i++) {	// create list columns
		const LIST_COL& lc = m_ListCol[i];
		m_List.InsertColumn(i, LDS(lc.TitleID), lc.Align, lc.Width);
	}
	GetWindowRect(m_InitRect);	// save dialog's initial size
	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	// build item state icon list
	m_StateImg.Create(12, 12, ILC_COLOR | ILC_MASK, ITEM_STATES, ITEM_STATES);
	m_StateImg.SetBkColor(GetSysColor(COLOR_WINDOW));
	for (i = 0; i < ITEM_STATES; i++)
		m_StateImg.Add(theApp.LoadIcon(m_StateIcon[i]));
	m_List.SetImageList(&m_StateImg, LVSIL_STATE);
	m_PlayBtn.SetIcons(IDI_PL_PLAYU, IDI_PL_PLAYD);
	m_PlayBtn.SetCheck(m_Playing);
	m_LoopBtn.SetIcons(IDI_PL_LOOPU, IDI_PL_LOOPD);
	m_LoopBtn.SetCheck(m_Looping);
	m_ShuffleBtn.SetIcons(IDI_PL_SHUFFLEU, IDI_PL_SHUFFLED);
	m_ShuffleBtn.SetCheck(m_Shuffling);
	m_PeriodEdit.SetFormat(CNumEdit::DF_INT);
	SetPeriod(m_Period);	// set automation timer

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlaylistDlg::OnOK()
{
	HWND	hWnd = ::GetFocus();
	if (hWnd == m_List.m_hWnd) {	// if list has focus
		int	ItemIdx = m_List.GetSelectionMark();
		if (ItemIdx >= 0)	// if current item
			OpenProject(ItemIdx);	// open it
	} else if (hWnd == m_PeriodEdit.m_hWnd)	// if period edit box has focus
		SetFocus();	// end edit
}

void CPlaylistDlg::OnSize(UINT nType, int cx, int cy) 
{
	CToolDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();
}

void CPlaylistDlg::OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM&	ListItem = pDispInfo->item;
	int	ItemIdx = ListItem.iItem;
	if (ListItem.mask & LVIF_TEXT) {
		switch (ListItem.iSubItem) {
		case COL_PROJECT:
			{
				CPathStr	name(PathFindFileName(m_Project[ItemIdx]));
				name.RemoveExtension();
				_tcscpy(ListItem.pszText, name);
			}
			break;
		}
	}
}

void CPlaylistDlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == AUTO_TIMER_ID) {
		// prevent reentrance in case OpenProject causes save changes dialog
		if (!m_InTimer) {
			m_InTimer = TRUE;
			int	items = GetItemCount();
			if (items) {
				int	ItemIdx = 0;
				if (m_Shuffling) {	// if random order
					// if not looping and random sequence finished
					if (!m_Looping && !m_RandList.GetAvail())
						SetPlay(FALSE);	// stop playing
					else
						ItemIdx = m_RandList.GetNext();
				} else {	// sequential order
					ItemIdx = m_CurItem + 1;
					if (ItemIdx >= items) {	// if at end of list
						if (m_Looping)	// if looping
							ItemIdx = 0;	// wrap to start of list
						else	// not looping
							SetPlay(FALSE);	// stop playing
					}
				}
				if (m_Playing)
					OpenProject(ItemIdx);
			}
			m_InTimer = FALSE;
		}
	}
	CToolDlg::OnTimer(nIDEvent);
}

void CPlaylistDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW	lpnmlv = (LPNMLISTVIEW)pNMHDR;
	int	ItemIdx = lpnmlv->iItem;
	if (ItemIdx >= 0)
		OpenProject(ItemIdx);

	*pResult = 0;
}

void CPlaylistDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	UpdateMenu(this, pPopupMenu);	// call menu's UI handlers
 	CToolDlg::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

void CPlaylistDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (point.x == -1 && point.y == -1) {	// if triggered via keyboard
		int	ItemIdx = m_List.GetSelectionMark();
		if (ItemIdx < 0)	// if no selection mark
			return;
		m_List.GetItemPosition(ItemIdx, &point);
		m_List.ClientToScreen(&point);
		point += CSize(10, 10);	// offset looks nicer
	}
	CMenu	menu, *mp;
	menu.LoadMenu(IDR_PLAYLIST_CTX);
	mp = menu.GetSubMenu(0);
	CPoint	Cursor = point;
	ScreenToClient(&Cursor);
	theApp.UpdateMenu(this, mp);
	mp->TrackPopupMenu(0, point.x, point.y, this);
}

void CPlaylistDlg::OnFileNew() 
{
	NewPlaylist();
}

void CPlaylistDlg::OnFileOpen() 
{
	CString	Path;
	bool	retc = theApp.GetMain()->PromptFile(
		CMainFrame::DIR_PLAYLIST, TRUE, NULL, NULL, Path);
	if (retc == IDOK) {
		Open(Path);
	}
}

void CPlaylistDlg::OnFileSave() 
{
	Save();
}

void CPlaylistDlg::OnFileSaveAs() 
{
	SaveAs();
}

void CPlaylistDlg::OnUpdateFileMru(CCmdUI* pCmdUI) 
{
	m_RecentPlaylist.UpdateMenu(pCmdUI);
}

void CPlaylistDlg::OnFileMru(UINT nID)
{
	OpenRecent(nID - ID_PL_FILE_MRU_FILE1);
}

void CPlaylistDlg::OnEditAdd() 
{
	CStringArray	proj;
	if (PromptForProjects(proj))
		InsertItems(GetItemCount(), proj);
}

void CPlaylistDlg::OnEditInsert() 
{
	CStringArray	proj;
	if (PromptForProjects(proj)) {
		int	ItemIdx = m_List.GetSelectionMark();
		if (ItemIdx < 0)
			ItemIdx = GetItemCount();
		InsertItems(ItemIdx, proj);
	}
}

void CPlaylistDlg::OnEditDelete() 
{
	int	ItemIdx = 0;
	DeleteSelectedItems(ItemIdx);
}

void CPlaylistDlg::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_List.GetSelectedCount());
}

void CPlaylistDlg::OnEditOpenProject()
{
	OpenProject(m_List.GetSelectionMark());
}

void CPlaylistDlg::OnReorderList(NMHDR* pNMHDR, LRESULT* pResult)
{
	int	InsPos = m_List.GetInsertPos();
	CDWordArray	sel;
	GetSelection(sel);
	int	SelItems = sel.GetSize();
	CStringArray	InsList;
	InsList.SetSize(SelItems);
	int	PlayingItemOfs = -1;
	for (int i = 0; i < SelItems; i++) {
		int	ItemIdx = sel[i];
		InsList[i] = m_Project[ItemIdx];
		if (ItemIdx == m_CurItem)	// if selection includes playing item
			PlayingItemOfs = i;	// save offset of playing item within insertion list
	}
	DeleteSelectedItems(InsPos);
	InsertItems(InsPos, InsList);
	if (PlayingItemOfs >= 0) {
		m_CurItem = InsPos + PlayingItemOfs;
		UpdateStateIcon();
	}

	*pResult = 0;
}

void CPlaylistDlg::OnAutoPlay() 
{
	SetPlay(!m_Playing);
}

void CPlaylistDlg::OnUpdateAutoPlay(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Playing);
}

void CPlaylistDlg::OnAutoLoop() 
{
	SetLoop(!m_Looping);
}

void CPlaylistDlg::OnUpdateAutoLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Looping);
}

void CPlaylistDlg::OnAutoShuffle() 
{
	SetShuffle(!m_Shuffling);
}

void CPlaylistDlg::OnUpdateAutoShuffle(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_Shuffling);
}

void CPlaylistDlg::OnSysColorChange()
{
	m_StateImg.SetBkColor(GetSysColor(COLOR_WINDOW));
	CToolDlg::OnSysColorChange();
}

void CPlaylistDlg::OnPlay() 
{
	SetPlay(!m_Playing);
}

void CPlaylistDlg::OnLoop() 
{
	SetLoop(!m_Looping);
}

void CPlaylistDlg::OnShuffle() 
{
	SetShuffle(!m_Shuffling);
}

void CPlaylistDlg::OnChangedPeriodEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetPeriodSecs(m_PeriodEdit.GetIntVal());
}

void CPlaylistDlg::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	// allow our context menu to display hints in the status bar
	theApp.GetMain()->SendMessage(WM_SETMESSAGESTRING, nItemID);	// show hint for this menu item
}

void CPlaylistDlg::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	// prevent main's frame counter from overwriting our menu hints
	theApp.GetMain()->SendMessage(WM_ENTERMENULOOP, bIsTrackPopupMenu);
}

void CPlaylistDlg::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	// restore main's frame counter behavior
	theApp.GetMain()->SendMessage(WM_EXITMENULOOP, bIsTrackPopupMenu);
}
