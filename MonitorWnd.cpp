// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      11jan07	initial version
		01		21jan07	replace AfxGetMainWnd with GetThis
		02		01dec07	let monitor source vary
		03		02may10	refactor for engine
		04		10may11	add SetDefaultSurface
		05		27dec11	in UpdateView, force redraw if paused
		06		26mar12	in UpdateView, add monitor source
		07		30mar12	add Clear
		08		24apr12	in UpdateView, redraw regardless of source
		09		23may12	in ctor, zero surface description
		
		monitor window
 
*/

// MonitorWnd.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MonitorWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMonitorWnd

IMPLEMENT_DYNAMIC(CMonitorWnd, CWnd);

CMonitorWnd::CMonitorWnd()
{
	m_Renderer = NULL;
	m_ClientSize = CSize(0, 0);
	m_FrameSize = CSize(0, 0);
	m_PrevBmp = NULL;
	m_FrameSurf = NULL;
	ZeroMemory(&m_SurfDesc, sizeof(m_SurfDesc));
	m_Quality = FALSE;
}

CMonitorWnd::~CMonitorWnd()
{
}

bool CMonitorWnd::CreateFrame(CSize FrameSize)
{
	m_FrameSize = FrameSize;
	return(m_Renderer->CreateSurface(m_SurfDesc, m_FrameSurf));
}

void CMonitorWnd::SetDefaultSurface()
{
	if (m_FrameSurf != NULL)
		m_FrameSurf->SetSurfaceDesc(CRenderer::GetDefaultSurface(), 0);
}

void CMonitorWnd::DestroyFrame()
{
	if (m_FrameSurf != NULL) {
		// reset surface to default before destroying it; prevents a major leak
		m_FrameSurf->SetSurfaceDesc(CRenderer::GetDefaultSurface(), 0);
		m_FrameSurf->Release();
		m_FrameSurf = NULL;
	}
}

void CMonitorWnd::Clear()
{
	m_OutDC.FillSolidRect(CRect(CPoint(0, 0), m_ClientSize), RGB(0, 0, 0));
	Invalidate();
}

void CMonitorWnd::UpdateView()
{
	if (m_Renderer->IsPaused()) {	// if paused, blit pause frame to output
		if (theApp.GetMain()->MonitoringOutput()) {
			CDC	FrameDC;
			FrameDC.CreateCompatibleDC(&m_OutDC);
			HGDIOBJ	PrevBmp = FrameDC.SelectObject(m_Renderer->GetPauseFrame());
			m_OutDC.StretchBlt(0, 0, m_ClientSize.cx, m_ClientSize.cy, &FrameDC, 
				0, 0, m_FrameSize.cx, m_FrameSize.cy, SRCCOPY);
			FrameDC.SelectObject(PrevBmp);
		}
		RedrawWindow();	// force redraw
	} else {	// not paused
		if (!theApp.GetEngine().GetPluginCount())	// if no plugins
			Clear();
		Invalidate();
	}
}

bool CMonitorWnd::CreateBackBuffer(int Width, int Height)
{
	if (m_PrevBmp != NULL) {
		m_OutDC.SelectObject(m_PrevBmp);
		m_OutBmp.DeleteObject();
	}
	CClientDC	dc(this);
	if (!m_OutBmp.CreateCompatibleBitmap(&dc, Width, Height))
		return(FALSE);
	m_PrevBmp = m_OutDC.SelectObject(m_OutBmp);
	m_ClientSize = CSize(Width, Height);
	UpdateView();
	return(TRUE);
}

void CMonitorWnd::TimerHook()
{
	PFRAME	Frame;
	if (m_Renderer->GetMonitorQueue().Read(Frame) == CFrameQueue::SUCCESS) {
		m_SurfDesc.lpSurface = Frame->Buf;
		if (SUCCEEDED(m_FrameSurf->SetSurfaceDesc(&m_SurfDesc, 0))) {
			HDC	sdc;
			if (SUCCEEDED(m_FrameSurf->GetDC(&sdc))) {
				StretchBlt(m_OutDC, 0, 0, m_ClientSize.cx, m_ClientSize.cy,
					sdc, 0, 0, m_FrameSize.cx, m_FrameSize.cy, SRCCOPY);
				Invalidate();
			}
			m_FrameSurf->ReleaseDC(sdc);
		}
		if (!InterlockedDecrement(&Frame->RefCount)) {
			theApp.GetEngine().GetFreeQueue()->Write(Frame);
		}
	}
}

inline void CMonitorWnd::ApplyQuality(bool Quality)
{
	static const int QualityMode[2] = {COLORONCOLOR, HALFTONE};
	m_OutDC.SetStretchBltMode(QualityMode[Quality]);
}

void CMonitorWnd::SetQuality(bool Quality)
{
	if (Quality == m_Quality)
		return;
	m_Quality = Quality;
	ApplyQuality(Quality);
	if (m_Renderer->IsPaused())
		UpdateView();
}

BEGIN_MESSAGE_MAP(CMonitorWnd, CWnd)
	//{{AFX_MSG_MAP(CMonitorWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMonitorWnd message handlers

int CMonitorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_Renderer = &theApp.GetMain()->GetRenderer();
	CClientDC	dc(this);
	if (!m_OutDC.CreateCompatibleDC(&dc))
		return -1;
	ApplyQuality(m_Quality);

	return 0;
}

void CMonitorWnd::OnDestroy() 
{
	DestroyFrame();
	CWnd::OnDestroy();
}

void CMonitorWnd::OnSize(UINT nType, int cx, int cy) 
{
	CreateBackBuffer(cx, cy);
	CWnd::OnSize(nType, cx, cy);
}

void CMonitorWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	dc.BitBlt(0, 0, m_ClientSize.cx, m_ClientSize.cy, &m_OutDC, 0, 0, SRCCOPY);
}
