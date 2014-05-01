// Copyleft 2007 Chris Korda This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either version 2 of
// the License, or any later version.
/*
        chris korda

		revision history:
		rev		date	comments
        00      07jan07	initial version
		
		worker thread to extract thumbnails

*/

#ifndef CTHUMB_THREAD
#define CTHUMB_THREAD

// The thread posts UWM_EXTRACTTHUMB messages to the destination window
// specified by the Launch function's hWnd argument:
//
// wParam: HBITMAP of thumbnail bitmap lParam: pointer to THUMB_RESULT struct
//
// To avoid serious memory leaks, the message recipient MUST free both the
// bitmap handle and the thumbnail result struct, for example:
//
// DeleteObject((HBITMAP)wParam); delete (THUMB_RESULT *)lParam;
//
// Since multiple thread instances can exist at once, it may be necessary to
// determine which instance a given UWM_EXTRACTTHUMB message came from.  The
// JobId member of THUMB_RESULT is provided for this purpose; it's guaranteed
// to be unique for each thread instance.  If JobId matches the value returned
// by GetCurJobId, the message came from the current instance; else it came
// from a previous instance and can typically be ignored.  NOTE HOWEVER that
// the bitmap and thumbnail result must still be freed in this case.
//
// The destination window MUST call the Cleanup function from its WM_DESTROY
// handler, BEFORE destroying the window.  Failure to do so may cause memory
// leaks, or an access violation.  Cleanup ensures that all thumbnail thread
// instances have exited, and safely disposes of any pending UWM_EXTRACTTHUMB
// messages in the window's message queue.
//
class CThumbThread : public WObject {
public:
// Types
	typedef struct tagTHUMB_RESULT {
		int		ItemIdx;	// index of item for which thumbnail was extracted
		int		JobId;		// unique identifier of thumbnail thread instance
	} THUMB_RESULT;

// Construction
	CThumbThread();
	~CThumbThread();

// Attributes
	void	SetItemCount(int Count);
	void	SetItemName(int ItemIdx, LPCTSTR Name);
	int		GetCurJobId();
	void	SetNextItem(int ItemIdx);
	LONG	GetThrCount();

// Operations
	void	Launch(HWND hWnd, LPCTSTR Folder, SIZE Size, DWORD ColorDepth);
	void	Kill();
	void	Cleanup(HWND hWnd);	// call from OnDestroy

protected:
// Types
	class CParms : WObject {	// parameters passed to thread
	public:
		CParms();
		~CParms();
		HWND	m_hWnd;			// destination window handle
		LPTSTR	m_Folder;		// source folder
		LPTSTR	*m_ItemName;	// array of source file names; names may be null
		int		m_ItemCount;	// number of source file names
		SIZE	m_Size;			// thumbnail size
		DWORD	m_ColorDepth;	// thumbnail color depth in bits
		int		m_JobId;		// unique identifier of thread instance
		CThumbThread	*m_Parent;	// pointer to thread's parent object
	};

// Member data
	CParms	*m_Parms;			// pointer to current parameters
	volatile int	m_CurJobId;	// unique identifier of thread instance
	volatile int	m_NextItem;	// next item thread should extract
	LONG	m_ThrCount;			// number of existing thread instances

// Helpers
	static	UINT	ThreadFunc(LPVOID pParam);
};

inline void CThumbThread::SetItemName(int ItemIdx, LPCTSTR Name)
{
	ASSERT(m_Parms != NULL);	// make sure parameters are allocated
	ASSERT(ItemIdx >= 0 && ItemIdx < m_Parms->m_ItemCount);	// check index range
	ASSERT(m_Parms->m_ItemName[ItemIdx] == NULL);	// only one name per item
	m_Parms->m_ItemName[ItemIdx] = _tcsdup(Name);
}

inline int CThumbThread::GetCurJobId()
{
	return(m_CurJobId);
}

inline void CThumbThread::SetNextItem(int ItemIdx)
{
	m_NextItem = ItemIdx;
}

inline LONG CThumbThread::GetThrCount()
{
	return(m_ThrCount);
}

inline void CThumbThread::Kill()
{
	m_CurJobId++;	// tell current thread instance to exit
}

#endif
