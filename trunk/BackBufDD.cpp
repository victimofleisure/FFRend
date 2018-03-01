// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10may05	initial version
        01      11may05	add error strings
        02      12may05	adapt for DX7
        03      20may05	make DX7 optional
        04      05jun05	add mirror blit
        05      22jun05	add SetOptions
        06      29jun05	add mirror precision option
        07      10jul05	add auto memory option
        08      14oct05	add exclusive mode
        09      14oct05	kill DirectDraw's leaky timer
		10		19oct05	in SetExclusive, save/restore window placement
		11		26oct05	don't set position before restoring placement
		12		05may06	add GetErrorString
		13		02aug06	overload Blt for external surfaces
		14		23nov07	support Unicode
		15		30jan08	fix unmirrored exclusive in system memory case
		16		15feb09	use forward declarations
		17		06mar10	move monitor info to its own class
		18		06mar10	make mirror support optional
		19		20jun10	in SetExclusive, add no copy bits flag
		20		10may12	in SetExclusive, add frame changed flag
		21		10may12	in SetExclusive, hide/show non-client area
		22		10may12	in SetExclusive, save/restore window size

        DirectDraw back buffer for off-screen drawing
 
*/

#include "stdafx.h"
#include "BackBufDD.h"
#include <ddraw.h>
#include "MonitorInfo.h"

#define DIRDRAWERR(x) {x, _T(#x)},
const CBackBufDD::ERRTRAN CBackBufDD::m_ErrTran[] = {
#include "DirDrawErrs.h"
{0, NULL}};

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

CBackBufDD::CBackBufDD()
{
	m_dd = NULL;
	m_Front = NULL;
	m_Back = NULL;
	m_SysBack = NULL;
	m_Unmirror = NULL;
	m_Mirror = NULL;
	m_DrawBuf = NULL;
	m_Clipper = NULL;
	m_Main = NULL;
	m_View = NULL;
	m_hr = 0;
	m_Width = 0;
	m_Height = 0;
	m_IsMirrored = FALSE;
	m_IsExclusive = FALSE;
	m_Options = OPT_AUTO_MEMORY | OPT_MIRROR_PRECISE;
	m_PreExclStyle = 0;
	m_PreExclSize = CSize(0, 0);
	ZeroMemory(&m_PreExclPlace, sizeof(m_PreExclPlace));
}

CBackBufDD::~CBackBufDD()
{
	Destroy();
}

BOOL CBackBufDD::Create(HWND Main, HWND View, GUID *Driver, bool Exclusive)
{
	Destroy();
	if (FAILED(m_hr = DirectDrawCreateEx(Driver, (VOID **)&m_dd, IID_IDirectDraw7, NULL)))
		return(FALSE);
	m_IsExclusive = Exclusive;	// order matters
	int	mode = Exclusive ? (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL;
	if (FAILED(m_hr = m_dd->SetCooperativeLevel(Exclusive ? Main : NULL, mode)))
		return(FALSE);
	// In exclusive mode, DirectDraw creates a 1.5 second periodic timer for
	// unknown reasons.  If the app is switched to exclusive mode and back in
	// less than 1.5 seconds, DirectDraw fails to clean up the timer, and the
	// app's main window receives spurious timer messages for timer ID 16962.
	KillTimer(Main, 16962);	// nuke it, seems to work fine
	m_Main = Main;
	m_View = View;
	return(TRUE);
}

void CBackBufDD::Destroy()
{
	if (!IsCreated())
		return;
	DeleteSurface();
	m_dd->Release();
	m_dd = NULL;
}

bool CBackBufDD::CreateSurface(int Width, int Height)
{
	if (!IsCreated())
		return(FALSE);
	Width = max(Width, 1);	// CreateSurface won't accept zero
	Height = max(Height, 1);
	DeleteSurface();
	DDSURFACEDESC2	sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.dwSize = sizeof(sd);
	if (m_IsExclusive) {
		sd.dwFlags 	= DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		sd.dwBackBufferCount = 2;	// prevents tearing
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Front, NULL)))
			return(FALSE);
		DDSCAPS2	caps;
		ZeroMemory(&caps, sizeof(caps));
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(m_hr = m_Front->GetAttachedSurface(&caps, &m_Back)))
			return(FALSE);
#ifndef BACKBUFDD_NOMIRROR	// if mirror support
		// if desired back buffer location is auto or video memory
		if ((m_Options & (OPT_AUTO_MEMORY | OPT_USE_VIDEO_MEM)) != 0) {
			m_Unmirror = m_Back;	// unmirrored back buffer in video memory
		} else {	// desired back buffer location is system memory
			sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
			sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
				| DDSCAPS_SYSTEMMEMORY;
			sd.dwWidth	= Width;
			sd.dwHeight	= Height;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_SysBack, NULL)))
				return(FALSE);
			m_Unmirror = m_SysBack;	// unmirrored back buffer in system memory
		}
		sd.dwFlags 	= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
			| DDSCAPS_SYSTEMMEMORY;	// draw to system memory in mirror mode
		sd.dwWidth	= Width >> 1;	// only need upper-left quadrant
		sd.dwHeight	= Height >> 1;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Mirror, NULL)))
			return(FALSE);
		m_DrawBuf = m_IsMirrored ? m_Mirror : m_Unmirror;
#endif
	} else {	// not exclusive
		sd.dwFlags = DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Front, NULL)))
			return(FALSE);
		sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		// NOTE: must specify DDSCAPS_3DDEVICE to use Direct3D hardware
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		// NOTE: mirroring may not work if DDSCAPS_VIDEOMEMORY is specified
		bool	UseVideoMem;
		if (m_Options & OPT_AUTO_MEMORY)
			UseVideoMem = !m_IsMirrored;
		else
			UseVideoMem = (m_Options & OPT_USE_VIDEO_MEM) != 0;
		if (UseVideoMem)
			sd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			sd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
#ifndef BACKBUFDD_NOMIRROR	// if mirror support
		if (m_IsMirrored) {
			sd.dwWidth		= (Width + 1) >> 1;	// only need upper-left quadrant
			sd.dwHeight		= (Height + 1) >> 1;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
				return(FALSE);
			if (m_Options & OPT_MIRROR_PRECISE) {	// mirror to intermediate surface
				sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
					| DDSCAPS_VIDEOMEMORY;
				sd.dwWidth	= Width;
				sd.dwHeight	= Height;
				if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Mirror, NULL)))
					return(FALSE);
			}
		} else {	// unmirrored
			sd.dwWidth = Width;
			sd.dwHeight	= Height;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
				return(FALSE);
		}
#else	// no mirror support
		sd.dwWidth = Width;
		sd.dwHeight	= Height;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
			return(FALSE);
#endif
		if (FAILED(m_hr = m_dd->CreateClipper(0, &m_Clipper, NULL)))
			return(FALSE);
		if (FAILED(m_hr = m_Clipper->SetHWnd(0, m_View)))
			return(FALSE);
		if (FAILED(m_hr = m_Front->SetClipper(m_Clipper)))
			return(FALSE);
		m_DrawBuf = m_Back;
	}
	m_Width = Width;
	m_Height = Height;
	return(TRUE);
}

void CBackBufDD::DeleteSurface()
{
	if (!IsCreated())
		return;
	SAFE_RELEASE(m_Back);
	SAFE_RELEASE(m_SysBack);
	SAFE_RELEASE(m_Mirror);
	SAFE_RELEASE(m_Clipper);
	SAFE_RELEASE(m_Front);
	m_Unmirror = NULL;
	m_DrawBuf = NULL;
}

HRESULT CBackBufDD::CreateSurface(DDSURFACEDESC2 *SurfaceDesc, IDirectDrawSurface7 *FAR *Surface)
{
	return(m_dd->CreateSurface(SurfaceDesc, Surface, NULL));
}

bool CBackBufDD::Blt()
{
#ifdef BACKBUFDD_NOMIRROR	// if no mirror support
	return(FALSE);	// do your own blitting
#endif
	enum {
		WAIT	= DDBLT_WAIT,
		WAITFX	= DDBLT_WAIT | DDBLT_DDFX
	};
	while (1) {
		HRESULT hRet;
		if (m_IsExclusive) {
			if (m_IsMirrored) {	// mirrored blit
				int	w = m_Width >> 1;
				int	h = m_Height >> 1;
				CRect	r(0, 0, w, h);
				DDBLTFX	fx;
				ZeroMemory(&fx, sizeof(fx));
				fx.dwSize = sizeof(fx);
				hRet = m_Back->Blt(r, m_Mirror, NULL, WAIT, &fx);	// upper left
				if (hRet == DD_OK) {
					fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
					r += CSize(w, 0);
					hRet = m_Back->Blt(r, m_Mirror, NULL, WAITFX, &fx);	// upper right
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
						r += CSize(0, h);
						hRet = m_Back->Blt(r, m_Mirror, NULL, WAITFX, &fx);	// lower right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
							r += CSize(-w, 0);
							hRet = m_Back->Blt(r, m_Mirror, NULL, WAITFX, &fx);	// lower left
							if (hRet == DD_OK)
								hRet = m_Front->Flip(NULL, DDFLIP_WAIT);	// update display
						}
					}
				}
			} else {	// unmirrored
				if (m_SysBack != NULL)	// if drawing to system memory
					// blit system memory back buffer to flipping chain back buffer
					hRet = m_Back->Blt(NULL, m_SysBack, NULL, WAIT, NULL);
				hRet = m_Front->Flip(NULL, DDFLIP_WAIT);	// update display
			}
		} else {	// not exclusive
			CPoint	org(0, 0);
			ClientToScreen(m_View, &org);
			if (m_IsMirrored) {	// mirrored blit
				CRect	r(0, 0, (m_Width + 1) >> 1, (m_Height + 1) >> 1);
				int	w = (m_Width >> 1);
				int	h = (m_Height >> 1);
				DDBLTFX	fx;
				ZeroMemory(&fx, sizeof(fx));
				fx.dwSize = sizeof(fx);
				if (m_Mirror != NULL) {	// mirror to intermediate surface
					hRet = m_Mirror->Blt(r, m_Back, NULL, WAIT, NULL);	// upper left
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
						r += CSize(w, 0);
						hRet = m_Mirror->Blt(r, m_Back, NULL, WAITFX, &fx);	// upper right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
							r += CSize(0, h);
							hRet = m_Mirror->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower right
							if (hRet == DD_OK) {
								fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
								r += CSize(-w, 0);
								hRet = m_Mirror->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower left
								if (hRet == DD_OK) {	// blit mirrored image to display
									CRect	wr(0, 0, m_Width, m_Height);
									wr += org;
									hRet = m_Front->Blt(wr, m_Mirror, NULL, WAIT, NULL);
								}
							}
						}
					}
				} else {	// mirror directly to display; quick and dirty
					r += org;
					hRet = m_Front->Blt(r, m_Back, NULL, WAIT, NULL);	// upper left
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
						r += CSize(w, 0);
						hRet = m_Front->Blt(r, m_Back, NULL, WAITFX, &fx);	// upper right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
							r += CSize(0, h);
							hRet = m_Front->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower right
							if (hRet == DD_OK) {
								fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
								r += CSize(-w, 0);
								hRet = m_Front->Blt(r, m_Back, NULL, WAITFX, &fx);	// lower left
							}
						}
					}
				}
			} else {	// ordinary blit
				CRect	r(0, 0, m_Width, m_Height);
				r += org;
				hRet = m_Front->Blt(r, m_Back, NULL, WAIT, NULL);
			}
		}
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST) {
			if (FAILED(m_Front->Restore()) || FAILED(m_Back->Restore()))
				return(FALSE);
		} else if (hRet != DDERR_WASSTILLDRAWING)
			return(FALSE);
	}
	return(TRUE);
}

bool CBackBufDD::Blt(LPRECT DestRect, LPDIRECTDRAWSURFACE7 SrcSurface,
					 LPRECT SrcRect, DWORD Flags, LPDDBLTFX BltFx)
{
	while (1) {
		HRESULT hRet;
		if (m_IsExclusive) {
			hRet = m_Back->Blt(DestRect, SrcSurface, SrcRect, Flags, BltFx);
			if (hRet == DD_OK)
				hRet = m_Front->Flip(NULL, DDFLIP_WAIT);
		} else
			hRet = m_Front->Blt(DestRect, SrcSurface, SrcRect, Flags, BltFx);
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST) {
			if (FAILED(m_Front->Restore()) || FAILED(m_Back->Restore()))
				return(FALSE);
		} else if (hRet != DDERR_WASSTILLDRAWING)
			return(FALSE);
	}
	return(TRUE);
}

LPCTSTR CBackBufDD::GetErrorString(HRESULT hr)
{
	for (int i = 0; m_ErrTran[i].Text != NULL; i++) {
		if (m_ErrTran[i].Code == hr)
			return(m_ErrTran[i].Text);
	}
	return(_T("unknown error"));
}

void CBackBufDD::SetMirror(bool Enable)
{
#ifndef BACKBUFDD_NOMIRROR	// if mirror support
	if (m_IsExclusive)
		m_DrawBuf = Enable ? m_Mirror : m_Unmirror;
	m_IsMirrored = Enable;
#endif
}

void CBackBufDD::SetOptions(int Options)
{
	m_Options = Options;
}

bool CBackBufDD::SetExclusive(HWND Main, HWND View, bool Enable)
{
	GUID	MonGuid, *Device = NULL;
	CRect	rc;
	CSize	NewSize;
	if (Enable) {	// if exclusive mode
		GetClientRect(Main, rc);
		m_PreExclSize = rc.Size();	// save window client size
		GetWindowPlacement(Main, &m_PreExclPlace);	// save window placement
		m_PreExclStyle = GetWindowLong(Main, GWL_STYLE);	// save window style
		// get rect and handle of monitor that this window is mostly on
		HMONITOR	hMon = CMonitorInfo::GetFullScreenRect(View, rc);
		if (CMonitorInfo::GetMonitorGUID(hMon, MonGuid))	// if GUID is available
			Device = &MonGuid;	// pass GUID to Create for hardware acceleration
		NewSize = rc.Size();
	} else {	// windowed mode
		NewSize = m_PreExclSize;	// restore previous size
	}
	if (!Create(Main, View, Device, Enable))	// recreate DirectDraw
		return(FALSE);
	if (!CreateSurface(NewSize.cx, NewSize.cy))	// recreate surfaces
		return(FALSE);
	if (Enable) {	// if exclusive mode
		SetWindowLong(Main, GWL_STYLE,	// update style to hide non-client area
			m_PreExclStyle & ~(WS_CAPTION | WS_THICKFRAME));
		// set topmost attribute and go full-screen; don't copy stale bits
		SetWindowPos(Main, HWND_TOPMOST,
			rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOCOPYBITS);
	} else {	// windowed mode
		SetWindowLong(Main, GWL_STYLE, m_PreExclStyle);	// restore previous style
		// clear topmost attribute, but don't set position or copy stale bits;
		// frame changed forces repaint of non-client area, needed if maximized
		SetWindowPos(Main, HWND_NOTOPMOST, 0, 0, 0, 0, 
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_FRAMECHANGED);
		SetWindowPlacement(Main, &m_PreExclPlace);	// restore previous placement
	}
	return(TRUE);
}
