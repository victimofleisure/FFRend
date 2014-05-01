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
		02		16apr11	refactor UpdateView
		03		16apr11	get renderer name
		04		13jan12	show frame counts while paused

        queue view
 
*/

// QueueView.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "QueueView.h"
#include "Benchmark.h"
#include "FFEngine.h"
#include "HistoryView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueueView

IMPLEMENT_DYNCREATE(CQueueView, CScrollView)

CQueueView::CQueueView() :
	m_Engine(theApp.GetEngine())
{
	m_PrevBmp = NULL;
	m_ClientSize = CSize(0, 0);
	m_ViewSize = CSize(0, 0);
	m_PrevScrollPos = CPoint(0, 0);
	m_MaxRowFrames = 0;
	m_FrameStyle = FS_INDEX | FS_REFS;
}

CQueueView::~CQueueView()
{
}

BOOL CQueueView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// override default window class styles CS_HREDRAW and CS_VREDRAW
	// otherwise resizing frame redraws entire view, causing flicker
	cs.lpszClass = AfxRegisterWndClass(	// create our own window class
		CS_DBLCLKS,						// request double-clicks
		theApp.LoadStandardCursor(IDC_ARROW),	// standard cursor
		NULL,									// no background brush
		theApp.LoadIcon(IDR_MAINFRAME));		// app's icon
	return CScrollView::PreCreateWindow(cs);
}

static const int MaxInputs = 2;
static const CSize	FrameSize(60, 20);
static const CSize	FrameGutter(4, 8);
static const CSize	FrameOffset(FrameSize + FrameGutter);
static const CSize	PluginMargin(8, 8);
static const int	PluginBaseWidth = 70;
static const int	PluginWidth = FrameSize.cx * 2 + PluginMargin.cx * 2 + PluginBaseWidth;
static const CSize	PluginGutter(PluginMargin);
static const CSize	InProcessOrg(PluginWidth - PluginMargin.cx - FrameSize.cx, PluginMargin.cy);
static const CSize	QueueOrg(PluginWidth + PluginGutter.cx, FrameGutter.cy);
static const CSize	OutputOffset(40, PluginMargin.cy + FrameOffset.cy);
static const CSize	CaptionMargin(5, 3);
static const CSize	RenderSize(PluginWidth, FrameOffset.cy + FrameGutter.cy);
static const CSize	PageMargin(PluginMargin);
static const CSize	PluginNameOffset(PluginMargin + CSize(-4, -4));
static const CSize	FrameCounterOffset(PluginMargin + CSize(-4, 11));
static const COLORREF	ActiveColor = RGB(0, 255, 0);

inline CSize CQueueView::GetFrameArea(int Frames, int Rows)
{
	return(CSize(FrameSize.cx * Frames + FrameGutter.cx * (Frames - 1),
		FrameSize.cy * Rows + FrameGutter.cy * (Rows - 1)));
}

void CQueueView::DrawFrame(CPoint pt, const PFRAME Frame)
{
	CString	s, t;
	m_DC.SelectObject(GetStockObject(WHITE_BRUSH));
	m_DC.Rectangle(CRect(pt, FrameSize));
	if (m_FrameStyle & FS_INDEX) {
		t.Format(_T("%d"), Frame->Idx);
		if (s.GetLength())
			s += _T(" : ");
		s += t;
	}
	if (m_FrameStyle & FS_CONTENT) {
		t.Format(_T("%d"), ((int *)Frame->Buf)[FRAME_CONTENT]);
		if (s.GetLength())
			s += _T(" : ");
		s += t;
	}
	if (m_FrameStyle & FS_REFS) {
		t.Format(_T("%d"), Frame->RefCount);
		if (s.GetLength())
			s += _T(" : ");
		s += t;
	}
	if (m_FrameStyle & FS_TIME) {
		t.Format(_T("%d"), ((int *)Frame->Buf)[FRAME_TIME]);
		if (s.GetLength())
			s += _T(" : ");
		s += t;
	}
	m_DC.SetTextAlign(TA_CENTER);
	m_DC.TextOut(pt.x + FrameSize.cx / 2, pt.y + CaptionMargin.cy, s);
}

inline void CQueueView::DrawPlugin(const CRect& rect, LPCTSTR Name, bool IsProcessing)
{
	if (IsProcessing)
		m_DC.SelectObject(m_ActiveBrush);
	else
		m_DC.SelectObject(GetStockObject(NULL_BRUSH));
	m_DC.Rectangle(rect);
	m_DC.SetTextAlign(TA_LEFT);
	CPoint	pt(rect.TopLeft() + PluginNameOffset);
	m_DC.TextOut(pt.x, pt.y, Name);
}

inline void CQueueView::DrawInputFrame(int InpIdx, CPoint pt, const PFRAME Frame)
{
	CString	s;
	s.Format(_T("%c:"), InpIdx + 'A');
	m_DC.SetTextAlign(TA_RIGHT);
	m_DC.TextOut(pt.x - CaptionMargin.cx, pt.y + CaptionMargin.cy, s);
	if (Frame != NULL)
		DrawFrame(pt, Frame);
}

inline void CQueueView::DrawOutputFrame(CPoint pt, const PFRAME Frame)
{
	m_DC.SetTextAlign(TA_RIGHT);
	m_DC.TextOut(pt.x - CaptionMargin.cx, pt.y + CaptionMargin.cy, "Out:");
	if (Frame != NULL)
		DrawFrame(pt, Frame);
}

inline void CQueueView::DrawFrameArray(CPoint pt, const CFrameArray& InQFrame)
{
	int	frames = InQFrame.GetSize();
	if (frames > m_MaxRowFrames)
		m_MaxRowFrames = frames;
	for (int i = 0; i < frames; i++) {
		DrawFrame(pt, InQFrame[i]);
		pt.x += FrameOffset.cx;
	}
}

inline void CQueueView::DrawFrameCounter(CPoint pt, UINT FrameCounter, int RenderTime)
{
	CString	s;
	s.Format(_T("T=%d (%+d)"), FrameCounter, FrameCounter - RenderTime);
	pt += FrameCounterOffset;
	m_DC.TextOut(pt.x, pt.y, s);
}

void CQueueView::UpdateView(bool Repaint)
{
	m_DC.FillSolidRect(CRect(CPoint(0, 0), m_ClientSize), GetSysColor(COLOR_WINDOW));
	m_DC.SetBkMode(TRANSPARENT);
	m_DC.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
	CPoint	ScrollPos = GetScrollPosition();
	CPoint	PlugPt(PageMargin - ScrollPos);
	CRenderer&	Render = theApp.GetMain()->GetRenderer();
	int	RenderTime = Render.GetFrameCounter();
	int	plugs = m_Engine.GetPluginCount();
	CFrameArray	QFrame;
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CPlugin&	plug = m_Engine.GetPlugin(PlugIdx);
		int	Inputs = plug.GetNumInputs();
		bool	UsingProcessCopy = plug.UsingProcessCopy();
		int	FrameRows = UsingProcessCopy ? max(Inputs, 2) : 1;
		int	PluginHeight = PluginMargin.cy * 2 + GetFrameArea(1, FrameRows).cy;
		CSize	PluginSize(PluginWidth, PluginHeight);
		CRect	PlugRect(PlugPt, PluginSize);
		DrawPlugin(PlugRect, plug.GetName(), plug.IsProcessing());
		if (m_Engine.IsRunning())
			DrawFrameCounter(PlugPt, plug.GetFrameCounter(), RenderTime);
		if (UsingProcessCopy)
			DrawOutputFrame(PlugPt + OutputOffset, plug.GetOutputFrame());
		CPoint	FramePt(PlugPt + InProcessOrg);
		for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {
			DrawInputFrame(InpIdx, FramePt, plug.GetInputFrame(InpIdx));
			plug.GetInputQueue(InpIdx)->DumpState(QFrame);
			DrawFrameArray(CPoint(PlugPt.x + QueueOrg.cx, FramePt.y), QFrame);
			FramePt.y += FrameOffset.cy;
		}
		PlugPt.y += PluginSize.cy + PluginMargin.cy;
		int	helpers = plug.GetHelperCount();
		for (int HelpIdx = 0; HelpIdx < helpers; HelpIdx++) {
			CPluginHelper&	helper = plug.GetHelper(HelpIdx);
			CRect	HelpRect(PlugPt, PluginSize);
			DrawPlugin(HelpRect, helper.GetName(), helper.IsProcessing());
			if (m_Engine.IsRunning())
				DrawFrameCounter(PlugPt, helper.GetFrameCounter(), RenderTime);
			if (UsingProcessCopy)
				DrawOutputFrame(PlugPt + OutputOffset, helper.GetOutputFrame());
			CPoint	FramePt(PlugPt + InProcessOrg);
			for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {
				DrawInputFrame(InpIdx, FramePt, helper.GetInputFrame(InpIdx));
				FramePt.y += FrameOffset.cy;
			}
			PlugPt.y += PluginSize.cy + PluginMargin.cy;
		}
	}
	int	TotalPluginHeight = PlugPt.y + ScrollPos.y;
	DrawPlugin(CRect(PlugPt, RenderSize), Render.GetName(), FALSE);
	DrawFrameCounter(PlugPt, RenderTime, RenderTime);
	PFRAME	OutFrame = Render.GetCurFrame();
	CPoint	FramePt(PlugPt + InProcessOrg);
	if (OutFrame != NULL)
		DrawFrame(FramePt, OutFrame);
	m_Engine.GetRenderQueue()->DumpState(QFrame);
	FramePt.x = PlugPt.x + QueueOrg.cx;
	DrawFrameArray(FramePt, QFrame);
	FramePt.x = PlugPt.x;
	FramePt.y += RenderSize.cy;
	m_Engine.GetFreeQueue()->DumpState(QFrame);
	DrawFrameArray(FramePt, QFrame);
	CSize	ViewSize(PluginWidth + m_MaxRowFrames * FrameOffset.cx, 
		TotalPluginHeight + RenderSize.cy + FrameOffset.cy + PageMargin.cy);
	if (ViewSize != m_ViewSize) {	// if view size changed
		m_ViewSize = ViewSize;
		SetScrollSizes(MM_TEXT, ViewSize);	// set scroll sizes
	}
	if (Repaint)
		Invalidate();
}

bool CQueueView::CreateBackBuffer(int Width, int Height)
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

/////////////////////////////////////////////////////////////////////////////
// CQueueView drawing

void CQueueView::OnInitialUpdate()
{
	m_ViewSize = CSize(0, 0);	// reset state so UpdateView sets scroll sizes
	m_MaxRowFrames = 0;
	CScrollView::OnInitialUpdate();
}

void CQueueView::OnDraw(CDC* pDC)
{
	CRect	cb;
	pDC->GetClipBox(cb);
	CPoint	sp = GetScrollPosition();
	if (m_PrevScrollPos != sp) {
		UpdateView(FALSE);	// don't invalidate
		m_PrevScrollPos = sp;
	}
	pDC->BitBlt(cb.left, cb.top, cb.Width(), cb.Height(), 
		&m_DC, cb.left - sp.x, cb.top - sp.y, SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////
// CQueueView diagnostics

#ifdef _DEBUG
void CQueueView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CQueueView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CQueueView message map

BEGIN_MESSAGE_MAP(CQueueView, CScrollView)
	//{{AFX_MSG_MAP(CQueueView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueueView message handlers

int CQueueView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	CClientDC	dc(this);
	m_DC.CreateCompatibleDC(&dc);
	m_ActiveBrush.CreateSolidBrush(ActiveColor);

	return 0;
}

int CQueueView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_ACTIVATE;	// don't call base class, prevents activation problems
}

void CQueueView::OnSize(UINT nType, int cx, int cy) 
{
	CreateBackBuffer(cx, cy);	// order matters, else bars are erratic
	UpdateView();
	CScrollView::OnSize(nType, cx, cy);
}

BOOL CQueueView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
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
