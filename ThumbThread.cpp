// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      07jan07	initial version
		
		worker thread to extract thumbnails

*/

#include "stdafx.h"
#include "ThumbThread.h"
#include "ExtractThumb.h"

CThumbThread::CParms::CParms()
{
	ZeroMemory(this, sizeof(CParms));
}

CThumbThread::CParms::~CParms()
{
	if (m_Folder != NULL)
		free(m_Folder);
	if (m_ItemName != NULL) {
		for (int i = 0; i < m_ItemCount; i++) {
			if (m_ItemName[i] != NULL)
				free(m_ItemName[i]);
		}
		delete [] m_ItemName;
	}
}

CThumbThread::CThumbThread()
{
	m_Parms = NULL;
	m_CurJobId = 0;
	m_NextItem = 0;
	m_ThrCount = 0;
}

CThumbThread::~CThumbThread()
{
	delete m_Parms;
}

void CThumbThread::SetItemCount(int Count)
{
	delete m_Parms;
	if (Count) {
		m_Parms = new CParms;
		m_Parms->m_ItemName = new LPTSTR[Count];
		ZeroMemory(m_Parms->m_ItemName, Count * sizeof(LPTSTR));
		m_Parms->m_ItemCount = Count;
	} else
		m_Parms = NULL;
}

void CThumbThread::Launch(HWND hWnd, LPCTSTR Folder, SIZE Size, DWORD ColorDepth)
{
	m_Parms->m_hWnd = hWnd;	// copy arguments to member data
	m_Parms->m_Folder = _tcsdup(Folder);
	m_Parms->m_Size = Size;
	m_Parms->m_ColorDepth = ColorDepth;
	m_CurJobId++;	// tell current thread instance to exit
	m_Parms->m_JobId = m_CurJobId;	// give new instance a unique identifier
	m_Parms->m_Parent = this;
	m_NextItem = 0;	// extraction begins with first item
	InterlockedIncrement(&m_ThrCount);	// increment thread instance count
	AfxBeginThread(ThreadFunc, m_Parms);	// launch thread
	m_Parms = NULL;	// thread deletes parameters
}

void CThumbThread::Cleanup(HWND hWnd)
{
	m_CurJobId++;	// tell current thread instance to exit
	while (m_ThrCount > 0)	// wait for thread instance count to reach zero
		Sleep(0);	// yield our time slice
	// remove pending UWM_EXTRACTTHUMB messages from queue and free their resources
	MSG	msg;
	while (PeekMessage(&msg, hWnd, UWM_EXTRACTTHUMB, UWM_EXTRACTTHUMB, PM_REMOVE)) {
		DeleteObject((HBITMAP)msg.wParam);	// delete bitmap
		delete (THUMB_RESULT *)msg.lParam;	// delete thumbnail result
	}
}

UINT CThumbThread::ThreadFunc(LPVOID pParam)
{
	CParms	*p = (CParms *)pParam;	// get thumbnail parameters
	CThumbThread	*tt = p->m_Parent;
	CoInitialize(NULL);
	{
		CExtractThumb	extr;
		if (extr.Create(p->m_Folder, p->m_Size, p->m_ColorDepth)) {
			int	Items = p->m_ItemCount;
			int	i = 0;	// index of current item; wraps around as needed
			int	Done = 0;	// number of items processed since last request
			int	NextItem = 0;	// index of most recently requested item
			while (Done < Items) {
				// if a specific item was requested
				if (tt->m_NextItem != NextItem && tt->m_NextItem < Items) {
					NextItem = tt->m_NextItem;	// extract requested item next
					i = NextItem;	// continue sequentially from requested item
					Done = 0;	// sequence was interrupted, so process all items
				}
				LPCTSTR	Name = p->m_ItemName[i];
				if (Name != NULL) {	// if item needs extracting
					HBITMAP	hBmp = extr.Extract(Name);	// extract thumbnail bitmap
					if (tt->m_CurJobId != p->m_JobId) {	// if new job identifier
						if (hBmp != NULL)
							DeleteObject(hBmp);	// delete bitmap
						break;	// abort processing
					}
					THUMB_RESULT	*result = new THUMB_RESULT;
					result->ItemIdx = i;
					result->JobId = p->m_JobId;
					// post thumbnail to destination window; recipient is responsible
					// for deleting both bitmap handle and thumbnail result struct
					::PostMessage(p->m_hWnd, 
						UWM_EXTRACTTHUMB, (WPARAM)hBmp, (LPARAM)result);
					free(p->m_ItemName[i]);	// delete item name
					p->m_ItemName[i] = NULL;	// mark item extracted
				}
				i++;
				if (i >= Items)	// wrap item index
					i = 0;
				Done++;
			}
		}
	}
	CoUninitialize();
	delete p;	// delete thumbnail parameters
	InterlockedDecrement(&tt->m_ThrCount);	// decrement thread instance count
	return(0);
}

