// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		23nov07	support Unicode
		02		15jan08	replace OnNotify with individual handlers
		03		02may11	add ReadJobFile
		04		30nov11	derive from tool dialog

		job control dialog

*/

#if !defined(AFX_JOBCONTROLDLG_H__3139E2D2_D34F_4023_B0B8_E7A19650928F__INCLUDED_)
#define AFX_JOBCONTROLDLG_H__3139E2D2_D34F_4023_B0B8_E7A19650928F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JobControlDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CJobControlDlg dialog

#include "ToolDlg.h"
#include "CtrlResize.h"
#include "JobList.h"
#include "DragVirtualListCtrl.h"

class CMainFrame;

class CJobControlDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CJobControlDlg);
// Construction
public:
	CJobControlDlg(CWnd* pParent = NULL);   // standard constructor
	~CJobControlDlg();

// Constants
	enum {	// define job states; must match m_JobStateID array init
		JOB_WAITING,
		JOB_POSTPONED,
		JOB_RUNNING,
		JOB_ABORTED,
		JOB_FAILED,
		JOB_DONE,
		JOB_STATES
	};

// Attributes
	int		GetJobCount() const;
	int		GetCurSel();
	void	SetCurSel(int JobIdx);
	int		GetStatus(int JobIdx) const;
	void	SetStatus(int JobIdx, int Status);
	bool	IsRunning(int JobIdx) const;
	void	GetInfo(int JobIdx, CJobInfo& Info) const;
	void	SetProgressPos(int Pos);
	bool	GetShutdown() const;

// Operations
	void	Add(CJobInfo& Info);
	void	Delete(int JobIdx);
	void	Move(int Src, int Dest);
	void	ToggleStatus(int JobIdx, int Status1, int Status2);
	void	UpdateStatuses(int OldStatus, int NewStatus);
	int		FindByID(int JobID) const;
	int		FindByStatus(int Status) const;
	int		FindWaiting() const;
	int		FindRunning() const;
	bool	StartJob(int JobIdx);
	bool	FinishJob();
	bool	SkipJob();
	bool	FailJob(LPCTSTR ErrorMsg);
	bool	ShowErrorMsg(int JobIdx);
	void	Abort();
	void	UpdateUI(int JobIdx = -1);
	bool	ReadJobFile();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJobControlDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CJobControlDlg)
	enum { IDD = IDD_JOB_CONTROL };
	CStatic	m_Percent;
	CStatic	m_ProgCap;
	CButton	m_OKBtn;
	CButton	m_StartBtn;
	CButton	m_SkipBtn;
	CProgressCtrl	m_Progress;
	CButton	m_PostponeBtn;
	CButton	m_MoveUpBtn;
	CButton	m_MoveDownBtn;
	CDragVirtualListCtrl	m_List;
	CButton	m_AbortBtn;
	CButton	m_DeleteBtn;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CJobControlDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAbort();
	afx_msg void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	afx_msg void OnDestroy();
	afx_msg void OnEditClearList();
	afx_msg void OnEditDeleteDone();
	afx_msg void OnEditDoneWaiting();
	afx_msg void OnEditFailedWaiting();
	afx_msg void OnEditPostponedWaiting();
	afx_msg void OnEditWaitingPostponed();
	afx_msg void OnFileLoadList();
	afx_msg void OnFileSaveList();
	afx_msg void OnGetdispinfoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	afx_msg void OnOptsShutdown();
	afx_msg void OnPostpone();
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReorderList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSkip();
	afx_msg void OnStart();
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOptsShutdown(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePostpone(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSkip(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewError(CCmdUI* pCmdUI);
	afx_msg void OnViewError();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct {
		int		TitleID;
		int		Align;
		int		Width;
	} LIST_COL;

// Constants
	enum {	// define list columns; must match m_ListCol array init
		COL_NAME,
		COL_SOURCE,
		COL_DEST,
		COL_START,
		COL_END,
		COL_STATUS,
		COLS
	};
	static const LIST_COL	m_ListCol[COLS];
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];	// zero terminated
	static const int	m_JobStateID[JOB_STATES];
	static const LPCTSTR	JOB_FILE_PATH;
	static const LPCTSTR	JOB_FILE_EXT;

// Member data
	CMainFrame	*m_Main;		// pointer to main frame
	CCtrlResize	m_Resize;		// control resizing object
	CRect	m_InitRect;			// initial window rectangle
	CJobList	m_Job;			// list of jobs
	CString	m_JobPath;			// path of job file
	bool	m_JobFileRead;		// if true, job list was read from file
	bool	m_Shutdown;			// if true, shutdown PC when jobs are done

// Helpers
	void	UpdateList();
	CString FormatTime(CTime Time);
};

inline int CJobControlDlg::GetJobCount() const
{
	return(m_Job.m_Info.GetSize());
}

inline int CJobControlDlg::GetCurSel()
{
	return(m_List.GetSelectionMark());	// why isn't GetSelectionMark const?
}

inline int CJobControlDlg::GetStatus(int JobIdx) const
{
	return(m_Job.m_Info[JobIdx].m_Status);
}

inline bool CJobControlDlg::IsRunning(int JobIdx) const
{
	return(GetStatus(JobIdx) == JOB_RUNNING);
}

inline void CJobControlDlg::GetInfo(int JobIdx, CJobInfo& Info) const
{
	Info = m_Job.m_Info[JobIdx];
}

inline int CJobControlDlg::FindWaiting() const
{
	return(FindByStatus(JOB_WAITING));
}

inline int CJobControlDlg::FindRunning() const
{
	return(FindByStatus(JOB_RUNNING));
}

inline bool CJobControlDlg::GetShutdown() const
{
	return(m_Shutdown);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JOBCONTROLDLG_H__3139E2D2_D34F_4023_B0B8_E7A19650928F__INCLUDED_)
