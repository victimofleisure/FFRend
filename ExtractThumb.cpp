// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17sep07	initial version

		extract thumbnail from file
 
*/

#include "stdafx.h"
#include "ExtractThumb.h"

#ifndef IEIFLAG_ASYNC	// IExtractImage wasn't defined in older SDKs

#define IEIFLAG_ASYNC       0x0001
#define IEIFLAG_CACHE		0x0002
#define IEIFLAG_ASPECT      0x0004
#define IEIFLAG_OFFLINE     0x0008
#define IEIFLAG_GLEAM       0x0010
#define IEIFLAG_SCREEN      0x0020
#define IEIFLAG_ORIGSIZE    0x0040
#define IEIFLAG_NOSTAMP     0x0080
#define IEIFLAG_NOBORDER    0x0100
#define IEIFLAG_QUALITY     0x0200
#define IEIFLAG_REFRESH     0x0400

MIDL_INTERFACE("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1")
IExtractImage : public IUnknown {
public:
virtual HRESULT STDMETHODCALLTYPE GetLocation(
	LPWSTR pszPathBuffer,
	DWORD cch,
	DWORD *pdwPriority,
	const SIZE *Size,
	DWORD ColorBits,
	DWORD *pdwFlags) = 0;

virtual HRESULT STDMETHODCALLTYPE Extract(
	HBITMAP *phBmpThumbnail) = 0;
};

typedef IExtractImage *LPEXTRACTIMAGE;
static const GUID IID_IExtractImage = {
	0xBB2E617C, 0x0920, 0x11d1, 0x9A, 0x0B, 0x00, 0xC0, 0x4F, 0xC2, 0xD6, 0xC1
};

#endif

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }

CExtractThumb::CExtractThumb()
{
	m_pMalloc = NULL;
	m_pDesktopFolder = NULL;
	m_pFolderIdl = NULL;
	m_pItemFolder = NULL;
}

CExtractThumb::~CExtractThumb()
{
	Destroy();
}

bool CExtractThumb::Create(LPCTSTR Folder, SIZE Size, DWORD ColorDepth)
{
	Destroy();	// in case we're already created
	HRESULT	hr = SHGetMalloc(&m_pMalloc);
	if (SUCCEEDED(hr)) {
		hr = SHGetDesktopFolder(&m_pDesktopFolder);
		if (SUCCEEDED(hr)) {
#ifdef UNICODE
			OLECHAR	*DisplayName = const_cast<OLECHAR *>(Folder);
#else
			OLECHAR	DisplayName[MAX_PATH];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Folder, -1, 
				DisplayName, MAX_PATH);
#endif
			ULONG	chEaten;
			ULONG	dwAttributes;
			hr = m_pDesktopFolder->ParseDisplayName(NULL, NULL, DisplayName, 
				&chEaten, &m_pFolderIdl, &dwAttributes);
			if (SUCCEEDED(hr)) {
				hr = m_pDesktopFolder->BindToObject(m_pFolderIdl, NULL, 
					IID_IShellFolder, (void **)&m_pItemFolder);
				if (SUCCEEDED(hr)) {
					m_Size = Size;
					m_ColorDepth = ColorDepth;
					return(TRUE);	// success, we're created
				}
			}
		}
	}
	Destroy();	// failure, clean up resources
	return(FALSE);
}

void CExtractThumb::Destroy()
{
	SAFE_RELEASE(m_pItemFolder);
	if (m_pFolderIdl != NULL) {
		m_pMalloc->Free(m_pFolderIdl);
		m_pFolderIdl = NULL;
	}
	SAFE_RELEASE(m_pDesktopFolder);
	SAFE_RELEASE(m_pMalloc);
}

HBITMAP CExtractThumb::Extract(LPCTSTR FileName)
{
	if (m_pItemFolder == NULL)
		return(NULL);	// we're not created
	HBITMAP	hBmp = NULL;
#ifdef UNICODE
	OLECHAR	*DisplayName = const_cast<OLECHAR *>(FileName);
#else
	OLECHAR	DisplayName[MAX_PATH];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, FileName, -1, 
		DisplayName, MAX_PATH);
#endif
	LPITEMIDLIST	pItemIdl;
	ULONG	chEaten;
	ULONG	dwAttributes;
	HRESULT	hr = m_pItemFolder->ParseDisplayName(NULL, NULL, DisplayName, 
		&chEaten, &pItemIdl, &dwAttributes);
	if (SUCCEEDED(hr)) {
		LPEXTRACTIMAGE	pExtractImage;
		LPCITEMIDLIST	apidl = pItemIdl;
		hr = m_pItemFolder->GetUIObjectOf(NULL, 1, &apidl, 
			IID_IExtractImage, NULL, (void **)&pExtractImage);
		if (SUCCEEDED(hr)) {
			OLECHAR	szPathBuf[MAX_PATH];
			DWORD	dwPriority = 0;
			DWORD	dwFlags = IEIFLAG_SCREEN;
			hr = pExtractImage->GetLocation(szPathBuf, MAX_PATH, 
				&dwPriority, &m_Size, m_ColorDepth, &dwFlags);
			if (SUCCEEDED(hr))
				hr = pExtractImage->Extract(&hBmp);
			pExtractImage->Release();
		}
		m_pMalloc->Free(pItemIdl);
	}
	return(hBmp);
}
