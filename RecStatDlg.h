// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08aug06	initial version

        record status dialog
 
*/

#if !defined(AFX_RECSTATDLG_H__00913E36_FA6E_4984_91FB_F2B069EC893C__INCLUDED_)
#define AFX_RECSTATDLG_H__00913E36_FA6E_4984_91FB_F2B069EC893C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecStatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecStatDlg dialog

#include "Benchmark.h"
#include "RunAvg.h"
#include "ToolDlg.h"

class CMainFrame;

class CRecStatDlg : public CToolDlg
{
	DECLARE_DYNAMIC(CRecStatDlg);
// Construction
public:
	CRecStatDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
	bool	Shutdown() const;

// Operations
	void	Start();
	void	Stop();
	void	Reset();
	void	TimerHook();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecStatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRecStatDlg)
	enum { IDD = IDD_REC_STAT };
	CButton	m_ShutdownChk;
	CButton	m_AbortBtn;
	CProgressCtrl	m_Progress;
	CStatic	m_Remaining;
	CStatic	m_Recorded;
	CStatic	m_Elapsed;
	CStatic	m_Duration;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRecStatDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		RUNAVG_SIZE = 5		// size of running average, in samples
	};

// Member data
	CMainFrame	*m_Main;	// pointer to main frame window
	CBenchmark	m_Clock;	// measures elapsed time via performance counter 
	CRunAvg<float>	m_RunAvgFR;	// running average of actual frame rate

// Helpers
	static	void	SetTime(CStatic& Ctrl, int Secs);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECSTATDLG_H__00913E36_FA6E_4984_91FB_F2B069EC893C__INCLUDED_)
