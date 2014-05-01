// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		26mar10	initial version
		01		16apr11	get renderer name
		02		09may11	in GraphThread, close process info handles

        graph view
 
*/

// GraphView.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "GraphView.h"
#include "Engine.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphView

IMPLEMENT_DYNCREATE(CGraphView, CScrollView)

CGraphView::CGraphView() :
	m_Engine(theApp.GetEngine())
{
	m_GraphSize = CSize(0, 0);
	m_PrevClientSize = CSize(0, 0);
}

CGraphView::~CGraphView()
{
}

BOOL CGraphView::PreCreateWindow(CREATESTRUCT& cs) 
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

UINT CGraphView::GraphThread(LPVOID pParam)
{
	LPTSTR	pDataPath = (LPTSTR)pParam;
	CPathStr	PicPath(pDataPath);
	PicPath.RenameExtension(_T(".gif"));	// same as data path except extension
	CString	cmdline;
	cmdline.Format(_T("dot.exe -Tgif -o\"%s\" \"%s\""), PicPath, pDataPath);
	TCHAR	*pCmdLine = cmdline.GetBuffer(0);
	STARTUPINFO	si;
	GetStartupInfo(&si);
	PROCESS_INFORMATION	pi;
	LPTSTR	pPicPath = NULL;
	UINT	dwFlags = CREATE_NO_WINDOW;	// avoid flashing console window
	if (CreateProcess(NULL, pCmdLine, NULL, NULL, FALSE, dwFlags, NULL, NULL, &si, &pi)) {
		static const int DOT_TIMEOUT = 10000;	// generous timeout
		if (WaitForSingleObject(pi.hThread, DOT_TIMEOUT) == WAIT_OBJECT_0) {
			pPicPath = _tcsdup(PicPath);	// clone picture path string on heap
		} else
			AfxMessageBox(IDS_GRAPH_DOT_TIMEOUT);
		CloseHandle(pi.hProcess);	// close both handles to avoid handle leak
		CloseHandle(pi.hThread);
	} else {	// can't run dot
		static const LPCTSTR GRAPHVIZ_URL = _T("http://www.graphviz.org/Download_windows.php");
		CString	msg;
		msg.Format(IDS_GRAPH_CANT_RUN_DOT, GetLastError());
		if (AfxMessageBox(msg, MB_YESNO) == IDYES)
			ShellExecute(NULL, NULL, GRAPHVIZ_URL, NULL, NULL, SW_SHOWNORMAL);
	}
	// OnGraphDone is responsible for deleting pPicPath
	theApp.GetMain()->PostMessage(UWM_GRAPHDONE, (WPARAM)pPicPath);
	DeleteFile(pDataPath);	// delete data temp file
	delete pDataPath;	// delete data path string
	return(0);
}

void CGraphView::OnGraphDone(LPTSTR pPicPath)
{
	if (pPicPath != NULL) {
		if (m_Graph.Open(pPicPath)) {
			BITMAP	bmp;
			if (GetObject(m_Graph.GetHandle(), sizeof(BITMAP), &bmp)) {
				m_GraphSize = CSize(bmp.bmWidth, bmp.bmHeight);
				SetScrollSizes(MM_TEXT, m_GraphSize);
				Invalidate();
			} else
				AfxMessageBox(IDS_GRAPH_CANT_GET_OBJECT);
		} else
			AfxMessageBox(IDS_GRAPH_CANT_OPEN_PICTURE);
		DeleteFile(pPicPath);	// delete picture temp file
		delete pPicPath;	// delete picture path string
	} else
		GetParent()->ShowWindow(SW_HIDE);
}

bool CGraphView::UpdateView()
{
	m_Graph.Close();
	Invalidate();
	CString	DataPath;
	if (!theApp.GetTempFileName(DataPath, _T("etp")))
		return(FALSE);
	CClientDC	dc(this);
	CSize	LogPix(dc.GetDeviceCaps(LOGPIXELSX), dc.GetDeviceCaps(LOGPIXELSY));
	double	DPI = max(LogPix.cx, LogPix.cy);
	SetScrollSizes(MM_TEXT, CSize(0, 0));	// remove scroll bars before measuring client area
	CRect	rc;
	GetClientRect(rc);
	if (rc.Width() >= 30000)	// if invalid rect, avoid crashing dot
		return(FALSE);
	double	Width = (rc.Width() - 1) / DPI;	// undershoot to avoid scroll bars due to rounding
	double	Height = (rc.Height() - 1) / DPI;
	if (!MakeGraphData(DataPath, Width, Height, DPI)) {
		DeleteFile(DataPath);
		return(FALSE);
	}
	LPTSTR	pDataPath = _tcsdup(DataPath);	// clone data path string on heap
	// GraphThread is responsible for deleting pDataPath
	AfxBeginThread(GraphThread, pDataPath, 0, 0, 0, NULL);
	m_PrevClientSize = rc.Size();
	return(TRUE);
}

bool CGraphView::MakeGraphData(LPCTSTR Path, double Width, double Height, double DPI)
{
	static const LPCTSTR NS_SOURCE	= _T("ellipse");
	static const LPCTSTR NS_MIXER	= _T("invtriangle");
	static const LPCTSTR NS_EFFECT	= _T("box");
	static const LPCTSTR NS_SINK	= _T("trapezium");
	static const LPCTSTR FontName	= _T("Verdana");
	int	plugs = m_Engine.GetPluginCount();
	if (!plugs)
		return(FALSE);	// nothing to graph
	FILE	*fp = _tfopen(Path, _T("w"));
	if (fp == NULL)
		return(FALSE);
	_ftprintf(fp, 
		_T("digraph G {\n")
		_T("graph[rankdir=TB,size=\"%f,%f\",ratio=fill,dpi=%f];\n")
		_T("node[fontname=\"%s\"];\n"),
		Width, Height, DPI, FontName);
	for (int i = 0; i < plugs; i++) {
		CPlugin&	plug = m_Engine.GetPlugin(i);
		int	SlotIdx = plug.GetSlotIdx();
		LPCTSTR	NodeShape;
		if (plug.IsSource())
			NodeShape = NS_SOURCE;
		else if (plug.IsMixer())
			NodeShape = NS_MIXER;
		else
			NodeShape = NS_EFFECT;
		_ftprintf(fp, _T("p%d[label=\"%s\",shape=%s];\n"), 
			SlotIdx, plug.GetName(), NodeShape);
		if (!plug.IsSource()) {	// if plugin takes input
			int	Inputs = plug.GetNumInputs();
			for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {
				CPlugin	*src = plug.GetInputSource(InpIdx);
				if (src != NULL) {
					CString	InpLabel;
					if (plug.IsMixer())
						InpLabel.Format(_T(",label=\"%c\""), 'A' + InpIdx);	// label input
					_ftprintf(fp, _T("p%d->p%d[fontname=\"%s\"%s];\n"), 
						src->GetSlotIdx(), SlotIdx, FontName, InpLabel);
				}
			}
		}
	}
	_ftprintf(fp, _T("out[label=\"%s\",shape=%s];\n"), 
		m_Engine.GetRenderer().GetName(), NS_SINK);
	if (plugs > 0)
		_ftprintf(fp, _T("p%d->out;\n"), m_Engine.GetPlugin(plugs - 1).GetSlotIdx());
	_ftprintf(fp, _T("}\n"));
	fclose(fp);
	return(TRUE);
}

void CGraphView::DelayedUpdate()
{
	m_PrevClientSize = CSize(0, 0);
	PostMessage(WM_TIMER, RESIZE_TIMER_ID, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CGraphView drawing

void CGraphView::OnInitialUpdate()
{
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	m_PrevClientSize = CSize(0, 0);
	CScrollView::OnInitialUpdate();
}

void CGraphView::OnDraw(CDC* pDC)
{
	CRect	cb;
	pDC->GetClipBox(cb);
	if (m_Graph.IsOpen()) {
		CDC	bdc;
		bdc.CreateCompatibleDC(NULL);
		HGDIOBJ	PrevObj = bdc.SelectObject(m_Graph.GetHandle());
		pDC->BitBlt(cb.left, cb.top, cb.Width(), cb.Height(), 
			&bdc, cb.left, cb.top, SRCCOPY);
		bdc.SelectObject(PrevObj);
		pDC->ExcludeClipRect(CRect(CPoint(0, 0), m_GraphSize));
	}
	pDC->FillSolidRect(cb, RGB(255, 255, 255));
}

/////////////////////////////////////////////////////////////////////////////
// CGraphView diagnostics

#ifdef _DEBUG
void CGraphView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CGraphView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGraphView message map

BEGIN_MESSAGE_MAP(CGraphView, CScrollView)
	//{{AFX_MSG_MAP(CGraphView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphView message handlers

int CGraphView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	m_ResizeTimer.Create(m_hWnd, RESIZE_TIMER_ID, RESIZE_DELAY, FALSE);

	return 0;
}

int CGraphView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return MA_ACTIVATE;	// don't call base class, prevents activation problems
}

void CGraphView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	m_ResizeTimer.Run(FALSE);
	m_ResizeTimer.Run(TRUE);	// restart timer
}

void CGraphView::OnTimer(W64UINT nIDEvent) 
{
	if (nIDEvent == RESIZE_TIMER_ID) {
		CRect	rc;
		GetClientRect(rc);
		if (rc.Size() != m_PrevClientSize)
			UpdateView();
		m_ResizeTimer.Run(FALSE);
	}
	CScrollView::OnTimer(nIDEvent);
}
