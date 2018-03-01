// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06mar10	initial version
        01      08mar10	add worker thread
        02      20jun10	in Render, blits were using flip wait flag
		03		10may11	add OnDeleteFrames
		04		27dec11	force pause frame's color depth to 24-bit
		05		05jan12	in Work, bump frame counter before freeing frame
		06		10jan12	in Pause, pause before creating frame to avoid race
		07		10jan12	in CreatePauseFrame, flush monitor queue
		08		16jan12	in OnPaint, verify pause back buffer exists
		09		16jan12	in Run, if no plugins, clear output and monitor
		10		30mar12	in FlushMonitorQueue, stop render
		11		13apr12	standardize thread function
		12		24apr12	in CreatePauseFrame, don't flush if monitoring plugin
		13		24apr12	create monitor queue in LaunchRenderThread
		14		27apr12	in Run, remove erroneous initial no-op test
		15		27apr12	invalidate current frame on start instead of stop
		16		10may12	in SetExclusive, only recreate backbuffer once
		17		31may12	in CreatePauseFrame, use MonitoringOutput accessor
		18		01jun12	move queue flush into engine

        rendering window
 
*/

// Renderer.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "Persist.h"
#include "Renderer.h"
#include "Benchmark.h"
#include "FFEngine.h"

#pragma warning(push)
#pragma warning(disable : 4201)	// nonstandard extension: nameless struct/union
#include "mmsystem.h"
#pragma warning(pop)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenderer

#define RK_RENDERER	_T("Renderer")

DWORD CRenderer::m_DefSurfMem[2]; // pointed to by m_DefSurf.lpSurface

DDSURFACEDESC2 CRenderer::m_DefSurf = {
	sizeof(DDSURFACEDESC2),	// dwSize
	DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT,	// dwFlags
	1,	// dwHeight
	1,	// dwWidth
	4,	// lPitch (Width * BitCount / 8)
	0, 0, 0, 0, // dwBackBufferCount, dwMipMapCount, dwAlphaBitDepth, dwReserved
	&m_DefSurfMem, // lpSurface
	{0, 0}, {0, 0}, {0, 0}, {0, 0}, // color keys
	{
		sizeof(DDPIXELFORMAT),	// dwSize
		DDPF_RGB,	// dwFlags
		0,			// dwFourCC
		32, 		// dwRGBBitCount
		0xff0000,	// dwRBitMask
		0x00ff00,	// dwGBitMask
		0x0000ff	// dwBBitMask
	}
};

CRenderer::CRun::CRun(CRenderer& Renderer, bool Enable) : 
	m_Renderer(Renderer)
{
	m_PrevRun = Renderer.IsRunning();	// save state
	// enter desired state, if not already in it
	m_Succeeded = Enable != m_PrevRun ? Renderer.Run(Enable) : TRUE;
}

CRenderer::CRun::~CRun()
{
	if (m_Succeeded && m_PrevRun != m_Renderer.IsRunning())	// if ctor changed state
		m_Renderer.Run(m_PrevRun);	// restore previous state
}

CRenderer::CRenderer(CFFEngine *Engine)
{
	m_Engine = Engine;
	m_Name = RK_RENDERER;
	m_Main = NULL;
	m_WasShown = FALSE;
	m_IsExclusive = FALSE;
	m_IsDualMonitor = FALSE;
	m_IsPaused = FALSE;
	m_InSetExclusive = FALSE;
	m_BackBuf = NULL;
	m_FrameSize = CSize(0, 0);
	m_ColorDepth = 0;
	m_hFrameTimer = NULL;
	m_FramePeriod = 0;
	m_FrameCounter = 0;
	m_CurFrame = NULL;
	m_IsSingleStep = FALSE;
	m_IsMonitoring = FALSE;
	m_IsRateLocked = FALSE;
	m_UseMMTimer = FALSE;
	m_MMFrameTimer = 0;
}

CRenderer::~CRenderer()
{
}

void CRenderer::DumpState(FILE *fp)
{
	m_Engine->DumpStallInfo(fp, m_Name, m_StallInfo);
	_fputts(_T("  Monitor: "), fp);
	m_MonitorQueue.DumpState(fp);
}

bool CRenderer::LaunchRenderThread()
{
	if (!m_MonitorQueue.Create(1, m_Engine->GetStopEvent(), 0)) {	// zero timeout
		AfxMessageBox(IDS_ENGERR_CANT_CREATE_QUEUE);
		return(FALSE);
	}
	if (!m_FrameTimer.Create(NULL, FALSE, NULL)) {
		AfxMessageBox(IDS_REND_CANT_CREATE_TIMER);
		return(FALSE);
	}
	if (!m_MMTimerEvent.Create(NULL, FALSE, FALSE, NULL)) {
		AfxMessageBox(IDS_REND_CANT_CREATE_TIMER);
		return(FALSE);
	}
	if (!CEngineThread::Create(ThreadFunc, this, 0, 0, m_Engine->GetFrameTimeout())) {
		AfxMessageBox(IDS_REND_CANT_CREATE_THREAD);
		return(FALSE);
	}
	return(TRUE);
}

bool CRenderer::KillRenderThread()
{
	if (!CEngineThread::Destroy())
		return(FALSE);
	Cleanup();
	return(TRUE);
}

void CRenderer::CancelTimer()
{
	if (m_MMFrameTimer) {	// if multimedia timer exists
		timeKillEvent(m_MMFrameTimer);	// destroy it
		m_MMFrameTimer = 0;
	} else	// assume Win32 timer
		m_FrameTimer.Cancel();	// cancel it
}

bool CRenderer::SetFrameTimer(UINT Period)
{
#ifdef RENDERER_NATTER
	_tprintf(_T("CRenderer::SetFrameTimer %d\n"), Period);
#endif
	CancelTimer();
	if (m_UseMMTimer) {	// if using multimedia timer
		HANDLE	hEvent = m_MMTimerEvent;
		m_MMFrameTimer = timeSetEvent(Period, 1, (LPTIMECALLBACK)hEvent, 
			(long)m_hWnd, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
		if (!m_MMFrameTimer)
			return(FALSE);
		m_hFrameTimer = m_MMTimerEvent;	// worker blocks on multimedia timer event
	} else {	// using Win32 timer
		LARGE_INTEGER	Due;
		Due.QuadPart = Period * -10000;	// relative UTC
		if (!m_FrameTimer.Set(&Due, Period, NULL, NULL, FALSE))
			return(FALSE);
		m_hFrameTimer = m_FrameTimer;	// worker blocks on Win32 timer
	}
	return(TRUE);
}

bool CRenderer::SetFramePeriod(UINT Period)
{
	if (Period == m_FramePeriod)
		return(TRUE);
	if (!SetFrameTimer(Period))
		return(FALSE);
	m_FramePeriod = Period;
	return(TRUE);
}

bool CRenderer::Run(bool Enable)
{
#ifdef RENDERER_NATTER
	_tprintf(_T("CRenderer::Run Enable=%d\n"), Enable);
#endif
	if (Enable) {	// if starting thread
		if (IsPaused()) {
			m_IsPaused = FALSE;
			DestroyPauseFrame();
		}
		if (!m_Engine->GetPluginCount()) {	// if no plugins
			Invalidate();	// clear output window
			m_Main->GetMonitorBar().UpdateView();	// monitor too
		}
		if (m_IsRateLocked && !m_IsSingleStep) {
			if (!SetFrameTimer(m_FramePeriod))
				return(FALSE);
		}
		m_CurFrame = NULL;	// invalidate current frame
		if (!CEngineThread::Run(TRUE))
			return(FALSE);
#ifdef RENDERER_NATTER
		_tprintf(_T("CRenderer::Run thread started\n"));
#endif
	} else {	// stopping thread
		if (!CEngineThread::Run(FALSE))
			return(FALSE);
#ifdef RENDERER_NATTER
		_tprintf(_T("CRenderer::Run thread stopped\n"));
#endif
		if (m_IsRateLocked)
			CancelTimer();
	}
	return(TRUE);
}

bool CRenderer::SetFrameProps(CSize FrameSize, UINT ColorDepth)
{
	STOP_RENDER(*this);
	m_FrameSize = FrameSize;
	m_ColorDepth = ColorDepth;
	return(Setup());
}

bool CRenderer::SetFrameSize(CSize FrameSize)
{
	return(SetFrameProps(FrameSize, m_ColorDepth));
}

bool CRenderer::SetColorDepth(UINT ColorDepth)
{
	return(SetFrameProps(m_FrameSize, ColorDepth));
}

bool CRenderer::SetWindowSize(CSize Size)
{
	STOP_RENDER(*this);
	return(m_dd.CreateSurface(Size.cx, Size.cy));
}

bool CRenderer::SetLockFrameRate(bool Enable)
{
	if (Enable == m_IsRateLocked)
		return(TRUE);
	STOP_RENDER(*this);
	m_IsRateLocked = Enable;
	return(TRUE);
}

bool CRenderer::SetUseMMTimer(bool Enable)
{
	if (Enable == m_UseMMTimer)
		return(TRUE);
	STOP_RENDER(*this);
	m_UseMMTimer = Enable;
	return(TRUE);
}

bool CRenderer::CreateSurfaces()
{
	STOP_RENDER(*this);
	Cleanup();
	if (!m_dd.Create(NULL, m_hWnd, FALSE)) {
		AfxMessageBox(IDS_REND_CANT_CREATE_DDRAW);
		return(FALSE);
	}
	CRect	rc;
	GetClientRect(rc);
	if (!m_dd.CreateSurface(rc.Width(), rc.Height())) {
		AfxMessageBox(IDS_REND_CANT_CREATE_SURFACE);
		return(FALSE);
	}
	if (m_ColorDepth && !Setup())
		return(FALSE);
	return(TRUE);
}

bool CRenderer::CreateSurface(DDSURFACEDESC2& sd, IDirectDrawSurface7*& Surf)
{
	ZeroMemory(&sd, sizeof(sd));
	sd.dwSize = sizeof(sd);
	sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	sd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	sd.dwWidth = m_FrameSize.cx; 
	sd.dwHeight = m_FrameSize.cy;
	if (FAILED(m_dd.CreateSurface(&sd, &Surf))) {
		AfxMessageBox(IDS_REND_CANT_CREATE_BACKBUF);
		return(FALSE);
	}
	sd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
	sd.lPitch = m_FrameSize.cx * (m_ColorDepth >> 3);
	sd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	sd.ddpfPixelFormat.dwRGBBitCount = m_ColorDepth;
	switch (m_ColorDepth) {
	case 16:
		sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		sd.ddpfPixelFormat.dwRBitMask = 0xf800;	// 5-6-5
		sd.ddpfPixelFormat.dwGBitMask = 0x07e0;
		sd.ddpfPixelFormat.dwBBitMask = 0x001f;
		break;
	case 24:
	case 32:
		sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		sd.ddpfPixelFormat.dwRBitMask = 0xff0000;
		sd.ddpfPixelFormat.dwGBitMask = 0x00ff00;
		sd.ddpfPixelFormat.dwBBitMask = 0x0000ff;
		break;
	default:
		return(FALSE);
	}
	return(TRUE);
}

bool CRenderer::Setup()
{
	Cleanup();
	if (!CreateSurface(m_SurfDesc, m_BackBuf))
		return(FALSE);
	return(m_Main->GetMonitorBar().CreateFrame(m_FrameSize));
}

void CRenderer::Cleanup()
{
	if (m_BackBuf != NULL) {
		m_BackBuf->SetSurfaceDesc(&m_DefSurf, 0);	// prevents a major leak
		m_BackBuf->Release();
		m_BackBuf = NULL;
	}
	m_Main->GetMonitorBar().DestroyFrame();
}

void CRenderer::OnDeleteFrames()
{
	// unlink surfaces from frames to avoid major leaks
	if (m_BackBuf != NULL)
		m_BackBuf->SetSurfaceDesc(&m_DefSurf, 0);
	m_Main->GetMonitorBar().SetDefaultSurface();
}

bool CRenderer::Render()
{
	m_SurfDesc.lpSurface = m_CurFrame->Buf;
	if (FAILED(m_BackBuf->SetSurfaceDesc(&m_SurfDesc, 0))) {
		AfxMessageBox(IDS_REND_CANT_SET_SURFACE);
		return(FALSE);
	}
	while (1) {
		HRESULT hRet;
		if (m_dd.IsExclusive()) {
			// source and dest must be same size, else bad stuff happens in
			// dual-monitor mode, e.g. dragging main window causes artifacts
			hRet = m_dd.GetBack()->Blt(NULL, m_BackBuf, NULL, DDBLT_WAIT, NULL);
			if (SUCCEEDED(hRet))
				hRet = m_dd.GetFront()->Flip(NULL, DDFLIP_WAIT);
		} else {
			CPoint	org(0, 0);
			CRect	Src(org, m_FrameSize);
			::ClientToScreen(m_hWnd, &org);
			CRect	Dest(org, m_dd.GetSize());
			hRet = m_dd.GetFront()->Blt(Dest, m_BackBuf, Src, DDBLT_WAIT, NULL);
		}
		if (SUCCEEDED(hRet))
			break;
		if (hRet == DDERR_SURFACELOST) {
			if (m_dd.GetFront() == NULL || FAILED(m_dd.GetFront()->Restore()) 
			|| m_dd.GetBack() == NULL || FAILED(m_dd.GetBack()->Restore()))
				return(FALSE);
		} else {
			if (hRet != DDERR_WASSTILLDRAWING)
				return(FALSE);
		}
	}
	return(TRUE);
}

UINT CRenderer::ThreadFunc(LPVOID Arg)
{
	ENGINE_THREAD_INIT(CRenderer);
	while (pThread->Work());
	ENGINE_THREAD_EXIT();
}

bool CRenderer::Work()
{
	while (1) {
		if (GetStopFlag()) {
#ifdef RENDERER_NATTER
			_tprintf(_T("render thread: stop flag set\n"));
#endif
			m_StallInfo.Queue = NULL;
			return(WaitForStart());
		} else {
			if (m_IsRateLocked) {
				if (WaitForSingleObject(m_hFrameTimer, CEngine::FRAME_TIMEOUT) != WAIT_OBJECT_0) {
					if (m_IsSingleStep) {	// if single-stepping
						// timer was closed intentionally to force a wait error;
						// set flag so we stop at top of loop after rendering
						m_StopFlag = TRUE;
					} else {	// wait error is real
						AfxMessageBox(IDS_REND_TIMER_WAIT_ERROR);
						return(WaitForStart());
					}
				}
			} else {
				// Exclusive mode blit spins (burns CPU) waiting for vsync.
				// This isn't a problem in the standard game model, because
				// the render thread spends most of the inter-frame interval
				// working. However in a pipelined model, where the render
				// thread blocks on an input queue while the work is done by
				// worker threads, the render thread can potentially spend 
				// the entire retrace interval spinning in blit, and beating
				// against the worker threads.
			}
			if (m_Engine->GetPluginCount()) {
				QREAD(m_Engine->GetRenderQueue(), m_CurFrame);
				AddProcessHistorySample(TRUE);
				Render();
				AddProcessHistorySample(FALSE);
				m_FrameCounter++;	// before free queue write which could exit
				if (!m_IsMonitoring || m_MonitorQueue.GetCount()
				|| m_MonitorQueue.Write(m_CurFrame) != CFrameQueue::SUCCESS) {
					if (!InterlockedDecrement(&m_CurFrame->RefCount)) {
						QWRITE(m_Engine->GetFreeQueue(), m_CurFrame);
					}
				}
			}
		}
	}
}

bool CRenderer::Pause(bool Enable)
{
	// pause worker thread before creating pause frame to avoid race
	if (!CEngineThread::Pause(Enable))	// pause or resume worker thread
		return(FALSE);
	m_IsPaused = Enable;	// update pause state
	if (Enable)	// if pausing
		CreatePauseFrame();	// create pause frame
	else	// resuming
		DestroyPauseFrame();	// destroy pause frame
	return(TRUE);
}

bool CRenderer::SingleStep()
{
	if (!IsPaused())
		return(FALSE);
	bool	PrevRateLock = m_IsRateLocked;
	bool	PrevIsMonitoring = m_IsMonitoring;
	m_IsRateLocked = TRUE;	// lock frame rate to frame timer
	m_IsSingleStep = TRUE;	// enter single step mode
	m_hFrameTimer = NULL;	// force worker's frame timer wait to fail
	m_IsMonitoring = FALSE;	// don't queue rendered frame to monitor window;
							// monitor window updates itself from pause frame
	bool	retc = FALSE;
	if (m_Engine->Pause(FALSE)) {	// unpause
		// our worker should render one frame and stop at top of loop;
		// wait for our worker to stop before pausing engine, else we 
		// could pause too soon and stall if worker is doing queue I/O
		if (WaitForSingleObject(m_Stopped, CEngine::FRAME_TIMEOUT) == WAIT_OBJECT_0) {
			if (m_Engine->Pause(TRUE))	// pause again
				retc = TRUE;	// success
		}
	}
	m_IsSingleStep = FALSE;	// exit single step mode
	m_IsRateLocked = PrevRateLock;
	m_IsMonitoring = PrevIsMonitoring;
	if (!SetFrameTimer(m_FramePeriod))
		return(FALSE);
	return(retc);
}

bool CRenderer::CreatePauseFrame()
{
	if (m_SurfDesc.lpSurface == NULL)	// if surface is invalid
		return(FALSE);
	// copy most recently shown surface to device-independent bitmap
	WORD	BitCount = 24;	// 16-bit DIBs are messy so force 24-bit
	if (!m_PauseFrame.Create(m_FrameSize.cx, m_FrameSize.cy, BitCount))
		return(FALSE);
	CDC	dc;
	if (!dc.CreateCompatibleDC(NULL))
		return(FALSE);
	HDC	sdc;
	if (m_BackBuf == NULL || m_BackBuf->GetDC(&sdc) != DD_OK)
		return(FALSE);
	HGDIOBJ	PrevObj = dc.SelectObject(m_PauseFrame);
	BitBlt(dc, 0, 0, m_FrameSize.cx, m_FrameSize.cy, sdc, 0, 0, SRCCOPY);
	dc.SelectObject(PrevObj);
	m_BackBuf->ReleaseDC(sdc);
	if (!m_dd.IsExclusive()) {
		if (!CreatePauseBackBuf(m_dd.GetSize()))
			return(FALSE);
	}
	if (m_Main->MonitoringOutput()) {	// if monitoring output, not a plugin
		FlushMonitorQueue();	// avoid competition with explicit monitor update
		m_Main->GetMonitorBar().UpdateView();
	}
	return(TRUE);
}

void CRenderer::DestroyPauseFrame()
{
	m_PauseFrame.Destroy();
	m_PauseBackBuf.DeleteObject();
}

BOOL CRenderer::CreatePauseBackBuf(CSize Size)
{
	ASSERT(!m_PauseFrame.IsEmpty());
	CClientDC	dc(this);
	CDC	StopFrmDC, BackBufDC;
	if (!BackBufDC.CreateCompatibleDC(&dc))
		return(FALSE);
	if (!StopFrmDC.CreateCompatibleDC(&dc))
		return(FALSE);
	m_PauseBackBuf.DeleteObject();
	if (!m_PauseBackBuf.CreateCompatibleBitmap(&dc, Size.cx, Size.cy))
		return(FALSE);
	HGDIOBJ	StopFrmPrevBmp, BackBufPrevBmp;
	StopFrmPrevBmp = StopFrmDC.SelectObject(m_PauseFrame);
	BackBufPrevBmp = BackBufDC.SelectObject(&m_PauseBackBuf);
	BackBufDC.SetStretchBltMode(COLORONCOLOR);
	BackBufDC.StretchBlt(0, 0, Size.cx, Size.cy, 
		&StopFrmDC, 0, 0, m_FrameSize.cx, m_FrameSize.cy, SRCCOPY);
	StopFrmDC.SelectObject(StopFrmPrevBmp);
	BackBufDC.SelectObject(BackBufPrevBmp);
	Invalidate();
	return(TRUE);
}

bool CRenderer::ExportBitmap(LPCTSTR Path, int Resolution, CFileException *pError)
{
	CFFEngine::CPause	pause(*GetEngine());
	if (!pause)
		return(FALSE);
	return(m_PauseFrame.Write(Path, Resolution, pError));
}

bool CRenderer::DualMonitorCheck(HWND hWnd)
{
	HMONITOR	hMonMain = MonitorFromWindow(AfxGetMainWnd()->m_hWnd, MONITOR_DEFAULTTONEAREST);
	HMONITOR	hMonThis = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	return(hMonMain != hMonThis);
}

bool CRenderer::SetExclusive(bool Enable)
{
#ifdef RENDERER_NATTER
	_tprintf(_T("CRenderer::SetExclusive Enable=%d\n"), Enable);
#endif
	if (Enable == m_IsExclusive)
		return(TRUE);
	STOP_RENDER(*this);
	Cleanup();	// destroy back buffer; must recreate it before returning
	if (Enable) {
		if (!IsWindowVisible())	// if output window is hidden
			ShowWindow(SW_SHOWNOACTIVATE);	// show it
		else {
			if (IsIconic())	// if output window is minimized
				ShowWindow(SW_RESTORE);	// restore it
		}
	}
	const int MAX_RETRIES = 10;
	const int RETRY_PAUSE = 500;
	int	Retries;
	m_InSetExclusive = TRUE;
	for (Retries = 0; Retries < MAX_RETRIES; Retries++) {
		if (m_dd.SetExclusive(m_hWnd, m_hWnd, Enable))
			break;	// success
#ifdef RENDERER_NATTER
		_tprintf(_T("CRenderer::SetExclusive retry %d\n"), Retries);
#endif
		Sleep(RETRY_PAUSE);
	}
	m_InSetExclusive = FALSE;
	if (!Setup())	// recreate back buffer
		return(FALSE);	// fatal error, back buffer is null
	if (Retries >= MAX_RETRIES) {	// if mode switch failed
		ModifyStyle(0, WS_CAPTION | WS_THICKFRAME);	// restore non-client area
		return(FALSE);
	}
	// dispatch any pending messages before resuming render thread
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) {	// for our window only
	    TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	m_IsExclusive = Enable;
	m_IsDualMonitor = DualMonitorCheck(m_hWnd);
#ifdef RENDERER_NATTER
	_tprintf(_T("CRenderer::SetExclusive exit m_IsExclusive = %d\n"), m_IsExclusive);
#endif
	return(TRUE);
}

bool CRenderer::FlushMonitorQueue()
{
	STOP_RENDER(*this);	// stop rendering
	m_Engine->FlushQueue(m_MonitorQueue);	// flush monitor queue
	return(TRUE);
}

bool CRenderer::Monitor(bool Enable)
{
	if (Enable == m_IsMonitoring)
		return(TRUE);
	if (!m_hWnd) {	// in case we haven't been created yet
		m_IsMonitoring = Enable;
		return(TRUE);
	}
	if (!Enable && IsRunning())	// if disabling monitoring while running
		FlushMonitorQueue();
	m_IsMonitoring = Enable;
	return(TRUE);
}

BEGIN_MESSAGE_MAP(CRenderer, CWnd)
	//{{AFX_MSG_MAP(CRenderer)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenderer message handlers

int CRenderer::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_Main = theApp.GetMain();
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	if (!CreateSurfaces())
		return -1;

	return 0;
}

void CRenderer::OnDestroy() 
{
	KillRenderThread();
	SetExclusive(FALSE);
	CPersist::SaveWnd(REG_SETTINGS, this, RK_RENDERER);
	CWnd::OnDestroy();
}

void CRenderer::OnClose() 
{
	if (m_IsExclusive)
		SetExclusive(FALSE);
	else
		ShowWindow(SW_HIDE);
}

void CRenderer::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
	if (!m_WasShown && !IsWindowVisible()) {
		m_WasShown = TRUE;
		CPersist::LoadWnd(REG_SETTINGS, this, RK_RENDERER, CPersist::NO_MINIMIZE);
	}
}

void CRenderer::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	if (m_dd.IsCreated()) {
		if (!m_dd.IsExclusive() && !m_InSetExclusive)
			SetWindowSize(CSize(cx, cy));
		if (IsPaused())
			CreatePauseBackBuf(CSize(cx, cy));	// resize pause back buffer
	}
}

void CRenderer::OnPaint() 
{
	CPaintDC	dc(this);
	// if worker thread is stopped or can't render due to lack of plugins
	if (!(IsRunning() && m_Engine->GetPluginCount())) {
		CRect	cb;
		dc.GetClipBox(cb);
		if (IsPaused() && m_PauseBackBuf.m_hObject != NULL) {
			CDC	BackBufDC;
			BackBufDC.CreateCompatibleDC(&dc);
			HGDIOBJ	PrevBmp = BackBufDC.SelectObject(&m_PauseBackBuf);
			dc.BitBlt(cb.left, cb.top, cb.Width(), cb.Height(), 
				&BackBufDC, cb.left, cb.top, SRCCOPY);
			BackBufDC.SelectObject(PrevBmp);
		} else	// not paused, or invalid pause back buffer
			dc.FillSolidRect(cb, RGB(0, 0, 0));
	}
}

BOOL CRenderer::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_IsExclusive) {
		SetCursor(NULL);
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
