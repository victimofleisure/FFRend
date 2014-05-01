// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29mar10	initial version
		01		15sep10	add plugin helpers
		02		18sep10	make zoom persistent
		03		30apr11	move thread iterator into engine
		04		16may11	let scroll bars display their context menus
		05		19may11	in CalcTimeSpan, skip renderer

        history view
 
*/

// HistoryView.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "HistoryView.h"
#include "HistoryInfo.h"
#include "MainFrm.h"
#include "Benchmark.h"
#include "Plugin.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryView

IMPLEMENT_DYNCREATE(CHistoryView, CScrollView)

const CSize CHistoryView::m_DefaultZoom(200, 20);
const CSize CHistoryView::m_MinZoom(5, 10);
const CSize CHistoryView::m_MaxZoom(3000, 200);

static const COLORREF CellBrushColor = RGB(0, 255, 0);
static const COLORREF CellPenColor = RGB(0, 192, 0);
static const COLORREF GridColor = RGB(192, 192, 192);
static const CSize LabelMargin(2, 0);
static const int ReticleHeight = 4;
static const int NominalGridSpacing = 40;

#define RK_HISTORY_ZOOM	_T("HistoryZoom")

CHistoryView::CHistoryView() :
	m_PauseRenderer(NULL)
{
	m_Engine = &theApp.GetEngine();
	m_PrevBmp = NULL;
	m_ClientSize = CSize(0, 0);
	m_ViewSize = CSize(0, 0);
	m_Zoom = theApp.RdRegStruct(RK_HISTORY_ZOOM, m_Zoom, m_DefaultZoom);
	m_StartTime = 0;
	m_PrevScrollPos = CPoint(0, 0);
	m_Dragging = FALSE;
	m_DragOrg = CPoint(0, 0);
	m_DragZoom = CSize(0, 0);
	m_IsRunning = FALSE;
	m_TotalTimeSpan = 0;
	m_TextHeight = 0;
	m_CtxMenuOrg = CPoint(0, 0);
	m_GridStep = 1;
	m_ShowFrameTimes = FALSE;
	m_PauseEngine = NULL;
	m_ResumeStartTime = 0;
	UpdateGridStep();
}

CHistoryView::~CHistoryView()
{
	delete m_PauseEngine;
	theApp.WrRegStruct(RK_HISTORY_ZOOM, m_Zoom);
}

BOOL CHistoryView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	return CView::PreCreateWindow(cs);
}

void CHistoryView::UpdateView(UINT Flags)
{
	int	threads = m_Engine->GetThreadCount();
	CSize	ViewSize;
	int	PageHeight = (threads + 1) * m_Zoom.cy + ReticleHeight;
	ViewSize.cy = PageHeight + m_TextHeight;
	int	RightT;
	int	LeftT;
	CPoint	ScrollPos = GetScrollPosition();
	if (m_IsRunning) {	// if running
		RightT = 0;
		LeftT = XToTime(m_ClientSize.cx);
		ViewSize.cx = m_ClientSize.cx;
	} else {	// stopped
		RightT = XToTime(GetMaxScrollPos().cx - ScrollPos.x);
		LeftT = XToTime(m_ViewSize.cx - ScrollPos.x);
		ViewSize.cx = TimeToX(m_TotalTimeSpan);
	}
	if (ViewSize != m_ViewSize) {	// if view size changed
		m_ViewSize = ViewSize;
		SetScrollSizes(MM_TEXT, ViewSize);	// set scroll sizes
	}
	if (Flags & UVF_SET_SIZE_ONLY)
		return;
	m_DC.FillSolidRect(0, 0, m_ClientSize.cx, m_ClientSize.cy, GetSysColor(COLOR_WINDOW));
	int	y1 = -ScrollPos.y;
	int	y2 = y1 + PageHeight;
	int	GridInterval = round(1000 * m_GridStep);
	if (threads) {
		int	n = 0;
		int	t = 0;
		CString	s;
		m_DC.SetTextAlign(TA_TOP | TA_CENTER);
		int	t1 = LeftT + GridInterval;	// add slack for right half of centered number
		while (t < t1) {
			int	x = TimeToX(t) + ScrollPos.x;
			x = m_ViewSize.cx - 1 - x;
			m_DC.FillSolidRect(x, y1, 1, PageHeight, GridColor);
			s.Format(_T("%g"), n++ * m_GridStep);
			m_DC.TextOut(x, y2, s);
			t += GridInterval;
		}
		m_DC.SetTextAlign(TA_TOP);
		for (CEngine::CThreadIter ThreadIter(*m_Engine); !ThreadIter.End();) {
			y2 = y1 + m_Zoom.cy;
			if (y2 >= 0) {	// if row isn't above client
				CProcessHistory&	ProcHist = (*ThreadIter).GetProcessHistory();
				LPCTSTR	Name = (*ThreadIter).GetName();
				CProcessHistory::CRevIter	iter(ProcHist);
				PROCESS_HISTORY_SAMPLE	samp;
				m_DC.FillSolidRect(0, y2, m_ViewSize.cx, 1, GridColor);
				int	PrevT = 0;
				while (iter.GetNext(samp)) {
					int	CurT = m_StartTime - samp.Time;	// relative to start time
					if (samp.Enable && CurT >= RightT) {
						int	x1 = TimeToX(CurT) + ScrollPos.x;
						int	x2 = TimeToX(PrevT) + ScrollPos.x;
						x1 = m_ViewSize.cx - x1;
						x2 = m_ViewSize.cx - x2 + 1;
						m_DC.Rectangle(x1, y1, x2, y2);
						if (m_ShowFrameTimes) {
							s.Format(_T("%d"), samp.FrameCount);
							m_DC.TextOut(x1, y1, s);
						}
						if (PrevT > LeftT)
							break;
					}
					PrevT = CurT;
				}
				m_DC.TextOut(LabelMargin.cx - ScrollPos.x, 
					y1 + m_Zoom.cy / 2 - m_TextHeight / 2, Name);
			}
			y1 = y2;
			if (y1 > m_ClientSize.cy)	// if next row is below client
				break;	// early out
		}
	}
	if (!(Flags & UVF_NO_INVALIDATE))
		Invalidate();
}

bool CHistoryView::CreateBackBuffer(int Width, int Height)
{
	if (m_PrevBmp != NULL) {
		m_DC.SelectObject(m_PrevBmp);
		m_BackBuf.DeleteObject();
	}
	CClientDC	dc(this);
	if (!m_BackBuf.CreateCompatibleBitmap(&dc, Width, Height))
		return(FALSE);
	m_PrevBmp = m_DC.SelectObject(m_BackBuf);
	m_ClientSize = CSize(Width, Height);
	return(TRUE);
}

void CHistoryView::ResetView()
{
	m_ViewSize = CSize(0, 0);	// reset state so UpdateView sets scroll sizes
	m_TotalTimeSpan = 0;
	m_PrevScrollPos = CPoint(-1, 0);	// force OnDraw to update view
}

void CHistoryView::CalcTimeSpan()
{
	m_TotalTimeSpan = 0;
	int	plugs = m_Engine->GetPluginCount();
	for (CEngine::CThreadIter ThreadIter(*m_Engine); !ThreadIter.End();) {
		if (ThreadIter.GetPluginIndex() > plugs)	// if renderer
			break;
		CProcessHistory&	ProcHist = (*ThreadIter).GetProcessHistory();
		PROCESS_HISTORY_SAMPLE	samp;
		int	i = ProcHist.Read(&samp, 1);
		if (i) {
			int	dt = m_StartTime - samp.Time;
			if (dt > m_TotalTimeSpan)
				m_TotalTimeSpan = dt;
		}
	}
}

void CHistoryView::Run(bool Enable)
{
	if (Enable == m_IsRunning)
		return;	// nothing to do
	if (!IsPaused()) {	// don't touch our run state while paused
		m_IsRunning = Enable;
		if (!Enable) {	// if stopping
			m_StartTime = GetTickCount();
			CalcTimeSpan();
			UpdateView();
			ScrollToPosition(CPoint(GetMaxScrollPos().cx, GetScrollPosition().y));
		} else
			ResetView();
	}
}

void CHistoryView::Pause(bool Enable)
{
	if (Enable == IsPaused())
		return;	// nothing to do
	if (Enable) {	// if pausing
		bool	WasRunning = m_IsRunning;	// SetInfo modifies this
		CHistoryInfo	info;
		GetInfo(info);	// retrieve history from real engine
		SetInfo(info);	// load history into pause engine
		// if we were running, stay scrolled far right for visual continuity
		if (WasRunning)
			ScrollToPosition(CPoint(GetMaxScrollPos().cx, GetScrollPosition().y));
	} else {	// resuming
		m_Engine = &theApp.GetEngine();	// point to real engine again
		delete m_PauseEngine;	// clean up pause engine
		m_PauseEngine = NULL;
		if (theApp.GetEngine().IsRunning())	// if real engine is running
			Run(TRUE);	// follow real engine
		else {	// real engine is stopped, just update view
			ResetView();
			m_StartTime = m_ResumeStartTime;
			CalcTimeSpan();
			UpdateView();
		}
	}
}

void CHistoryView::UpdateZoom(CPoint ScrollPos)
{
	SetRedraw(FALSE);
	UpdateView(UVF_SET_SIZE_ONLY);
	CSize	maxsp = GetMaxScrollPos();
	ScrollPos.x = CLAMP(ScrollPos.x, 0, maxsp.cx);
	ScrollPos.y = CLAMP(ScrollPos.y, 0, maxsp.cy);
	ScrollToPosition(ScrollPos);
	SetRedraw(TRUE);
	UpdateGridStep();
	UpdateView();
}

void CHistoryView::SetZoom(CSize Zoom)
{
	m_Zoom = Zoom;
	UpdateZoom(GetScrollPosition());
}

void CHistoryView::Zoom(CPoint Origin, double ZoomX, double ZoomY)
{
	m_Zoom.cx = round(m_Zoom.cx * ZoomX);
	m_Zoom.cy = round(m_Zoom.cy * ZoomY);
	CPoint	sp = GetScrollPosition();
	sp += Origin;
	sp.x = round(sp.x * ZoomX);
	sp.y = round(sp.y * ZoomY);
	sp -= Origin;
	UpdateZoom(sp);
}

void CHistoryView::ZoomCursor(double ZoomX, double ZoomY)
{
	CPoint	CurPos;
	GetCursorPos(&CurPos);
	ScreenToClient(&CurPos);
	Zoom(CurPos, ZoomX, ZoomY);
}

void CHistoryView::UpdateGridStep()
{
	double	StepSize = double(NominalGridSpacing) / m_Zoom.cx;
	m_GridStep = QuantizeStep(StepSize);
}

double CHistoryView::QuantizeStep(double StepSize)
{
	double	r = log10(StepSize);
	if (r >= 0) {
		r = pow(10, ceil(r));
		if (StepSize < r / 5)
			return(r / 5);
		if (StepSize < r / 2)
			return(r / 2);
		return(r);
	} else {
		r = pow(10, floor(r));
		if (StepSize > r * 5)
			return(r * 10);
		if (StepSize > r * 2)
			return(r * 5);
		return(r * 2);
	}
}

void CHistoryView::GetInfo(CHistoryInfo& Info) const
{
	// stop real engine while reading its history, to avoid race conditions
	CEngine::CRun	stop(theApp.GetEngine());
	Info.m_StartTime = m_StartTime;
	int	rows = m_Engine->GetThreadCount() + 1;	// add one for output
	Info.m_Row.SetSize(rows);
	int	RowIdx = 0;
	for (CEngine::CThreadIter ThreadIter(*m_Engine); !ThreadIter.End();) {
		CProcessHistory&	ProcHist = (*ThreadIter).GetProcessHistory();
		LPCTSTR	Name = (*ThreadIter).GetName();
		CHistoryInfo::CRow&	row = Info.m_Row[RowIdx++];
		row.m_Name = Name;
		int	samps = ProcHist.GetCount();
		row.m_Sample.SetSize(samps);
		ProcHist.Read(row.m_Sample.GetData(), samps);
	}
}

void CHistoryView::SetInfo(const CHistoryInfo& Info)
{
	if (!IsPaused()) {	// if we're not already paused
		m_PauseEngine = new CPauseEngine(m_PauseRenderer);	// create pause engine
		m_Engine = m_PauseEngine;	// switch engine pointer to pause engine
		m_IsRunning = FALSE;	// pause implies we're also stopped
		m_ResumeStartTime = m_StartTime;	// save current start time for unpause
	}
	m_StartTime = Info.m_StartTime;
	int	plugs = INT64TO32(Info.m_Row.GetSize()) - 1;	// subtract one for output
	m_PauseEngine->SetPluginCount(plugs);
	for (int PlugIdx = 0; PlugIdx <= plugs; PlugIdx++) {	// include output
		CProcessHistory	*ProcHist;
		if (PlugIdx < plugs) {
			CPlugin	*plug = m_Engine->GetSlot(PlugIdx);
			plug->SetName(Info.m_Row[PlugIdx].m_Name);
			ProcHist = &plug->GetProcessHistory();
		} else	// output
			ProcHist = &m_Engine->GetRenderer().GetProcessHistory();
		const CHistoryInfo::CRow&	row = Info.m_Row[PlugIdx];
		int	samps = row.m_Sample.GetSize();
		ProcHist->Create(samps);
		for (int i = 0; i < samps; i++)
			ProcHist->Push(row.m_Sample[i]);
	}
	ResetView();
	CalcTimeSpan();
	UpdateView();
}

bool CHistoryView::Write(LPCTSTR Path)
{
	CHistoryInfo	info;
	GetInfo(info);
	return(info.Write(Path));
}

bool CHistoryView::Read(LPCTSTR Path)
{
	CHistoryInfo	info;
	if (!info.Read(Path))
		return(FALSE);
	SetInfo(info);
	ScrollToPosition(CPoint(0, 0));	// reset scroll position to home
	return(TRUE);
}

void CHistoryView::CPauseEngine::SetPluginCount(int Plugins)
{
	m_Slot.SetSize(Plugins);
	for (int SlotIdx = 0; SlotIdx < Plugins; SlotIdx++) {
		CPlugin	*plug = new CPlugin;
		CSlotPtr	slot(plug);
		m_Slot[SlotIdx] = slot;
	}
	OnEndSlotChange();
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryView drawing

void CHistoryView::OnInitialUpdate()
{
	if (IsPaused())
		return;	// ignore document updates while paused
	ResetView();
	CScrollView::OnInitialUpdate();
}

void CHistoryView::OnDraw(CDC* pDC)
{
	CRect	cb;
	pDC->GetClipBox(cb);
	CPoint	sp = GetScrollPosition();
	if (m_PrevScrollPos != sp) {
		UpdateView(UVF_NO_INVALIDATE);	// don't invalidate
		m_PrevScrollPos = sp;
	}
	pDC->BitBlt(cb.left, cb.top, cb.Width(), cb.Height(), 
		&m_DC, cb.left - sp.x, cb.top - sp.y, SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////
// CHistoryView diagnostics

#ifdef _DEBUG
void CHistoryView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CHistoryView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHistoryView message map

BEGIN_MESSAGE_MAP(CHistoryView, CScrollView)
	//{{AFX_MSG_MAP(CHistoryView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_VIEW_ZOOM_IN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, OnViewZoomOut)
	ON_COMMAND(ID_VIEW_RESET_ZOOM, OnViewResetZoom)
	ON_COMMAND(ID_HISTORY_PAUSE, OnHistoryPause)
	ON_UPDATE_COMMAND_UI(ID_HISTORY_PAUSE, OnUpdateHistoryPause)
	ON_COMMAND(ID_HISTORY_SHOW_TIMES, OnHistoryShowTimes)
	ON_UPDATE_COMMAND_UI(ID_HISTORY_SHOW_TIMES, OnUpdateHistoryShowTimes)
	ON_WM_MOUSEACTIVATE()
	ON_COMMAND(ID_HISTORY_OPEN, OnHistoryOpen)
	ON_COMMAND(ID_HISTORY_SAVE, OnHistorySave)
	ON_WM_MENUSELECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryView message handlers

int CHistoryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetScrollSizes(MM_TEXT, CSize(0, 0));
	CClientDC	dc(this);
	m_DC.CreateCompatibleDC(&dc);
	m_DC.SetBkMode(TRANSPARENT);
	m_DC.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	m_DC.SelectObject(GetStockObject(DC_PEN));
	m_DC.SelectObject(GetStockObject(DC_BRUSH));
	SetDCBrushColor(m_DC, CellBrushColor);
	SetDCPenColor(m_DC, CellPenColor);
	TEXTMETRIC	tm;
	if (m_DC.GetTextMetrics(&tm))
		m_TextHeight = tm.tmHeight;
	m_StartTime = GetTickCount();	// prevents initial flicker

	return 0;
}

int CHistoryView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_ACTIVATE;	// don't call base class, prevents activation problems
}

void CHistoryView::OnSize(UINT nType, int cx, int cy) 
{
	CreateBackBuffer(cx, cy);	// order matters, else bars are erratic
	UpdateView();
	CScrollView::OnSize(nType, cx, cy);
}

void CHistoryView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetCapture();
	m_Dragging = TRUE;
	m_DragOrg = point;
	m_DragZoom = m_Zoom;
	SetCursor(theApp.LoadStandardCursor(IDC_SIZEALL));
	CScrollView::OnLButtonDown(nFlags, point);
}

void CHistoryView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	m_Dragging = FALSE;
	CScrollView::OnLButtonUp(nFlags, point);
}

void CHistoryView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_Dragging) {
		CSize	dz(m_DragOrg.x - point.x, point.y - m_DragOrg.y);	// reverse y-axis
		CSize	NewZoom(m_DragZoom + dz);
		NewZoom.cx = CLAMP(NewZoom.cx, m_MinZoom.cx, m_MaxZoom.cx);
		NewZoom.cy = CLAMP(NewZoom.cy, m_MinZoom.cy, m_MaxZoom.cy);
		if (NewZoom != m_Zoom) {
			if (m_IsRunning) {
				SetZoom(NewZoom);
			} else {
				Zoom(m_DragOrg, double(NewZoom.cx) / m_Zoom.cx, 
					double(NewZoom.cy) / m_Zoom.cy);
			}
		}
	}
	CScrollView::OnMouseMove(nFlags, point);
}

BOOL CHistoryView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	// workaround to allow scroll ranges > 32K, see MS Q166473
	SCROLLINFO	info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;
	if (LOBYTE(nScrollCode) == SB_THUMBTRACK) {
		GetScrollInfo(SB_HORZ, &info);
		nPos = info.nTrackPos;
	}
	if (HIBYTE(nScrollCode) == SB_THUMBTRACK) {
		GetScrollInfo(SB_VERT, &info);
		nPos = info.nTrackPos;
	}
	return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}

void CHistoryView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CRect	rc;
	GetClientRect(rc);
	CPoint	ClientPt(point);
	ScreenToClient(&ClientPt);
	if (rc.PtInRect(ClientPt)) {	// if within client area
		CMenu	menu;
		menu.LoadMenu(IDR_HISTORY_CTX);
		CMenu	*mp = menu.GetSubMenu(0);
		theApp.UpdateMenu(this, &menu);
		m_CtxMenuOrg = ClientPt;
		mp->TrackPopupMenu(0, point.x, point.y, this);
	} else	// let base class handle scroll bar menus
		CScrollView::OnContextMenu(pWnd, point);
}

void CHistoryView::OnViewZoomIn()
{
	if (m_Zoom.cx < SHRT_MAX)
		Zoom(m_CtxMenuOrg, 2, 2);
}

void CHistoryView::OnViewZoomOut()
{
	if (m_Zoom.cx > 1)
		Zoom(m_CtxMenuOrg, .5, .5);
}

void CHistoryView::OnViewResetZoom()
{
	SetZoom(m_DefaultZoom);
}

void CHistoryView::OnHistoryShowTimes()
{
	m_ShowFrameTimes ^= 1;
	UpdateView();
}

void CHistoryView::OnUpdateHistoryShowTimes(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_ShowFrameTimes);
}

void CHistoryView::OnHistoryOpen()
{
	CString	Filter((LPCTSTR)IDS_HISTORY_FILTER);
	CFileDialog	fd(TRUE, HISTORY_EXT, NULL, OFN_HIDEREADONLY, Filter);
	if (fd.DoModal() == IDOK)
		Read(fd.GetPathName());
}

void CHistoryView::OnHistorySave()
{
	CString	Filter((LPCTSTR)IDS_HISTORY_FILTER);
	CFileDialog	fd(FALSE, HISTORY_EXT, NULL, OFN_OVERWRITEPROMPT, Filter);
	if (fd.DoModal() == IDOK)
		Write(fd.GetPathName());
}

void CHistoryView::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	// allow our context menu to display hints in the status bar
	theApp.GetMain()->SendMessage(WM_SETMESSAGESTRING, nItemID);	// show hint for this menu item
}

void CHistoryView::OnHistoryPause()
{
	Pause(!IsPaused());
}

void CHistoryView::OnUpdateHistoryPause(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPaused());
}

