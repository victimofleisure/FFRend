// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		21feb09	initial version

		single-threaded modeless message box

*/

#if !defined(AFX_MSGBOXDLG_H__198B7582_3662_4DA1_95B1_FAC99E3E05EC__INCLUDED_)
#define AFX_MSGBOXDLG_H__198B7582_3662_4DA1_95B1_FAC99E3E05EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgBoxDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxDlg dialog

class CMsgBoxDlg : public CDialog
{
	DECLARE_DYNAMIC(CMsgBoxDlg);
// Construction
public:
	CMsgBoxDlg(CWnd* pParent = NULL);   // standard constructor

// Operations
	static	bool	Do(LPCTSTR lpText, LPCTSTR lpCaption = NULL, UINT uType = 0);
	bool	CreateMsg(LPCTSTR lpText, LPCTSTR lpCaption = NULL, UINT uType = 0);

// Helpers
	static	CSize	GetMsgExtent(CWnd& Wnd, LPCTSTR Text, CStringArray *pTextLine);
	static	HMONITOR	GetWorkspaceRect(HWND hWnd, CRect& rc);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgBoxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMsgBoxDlg)
	enum { IDD = IDD_MSG_BOX };
	CButton	m_OkBtn;
	CStatic	m_Icon;
	CStatic	m_TextCtrl;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMsgBoxDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	CString	m_Text;			// message box text
	CString	m_Caption;		// message box caption
	UINT	m_Type;			// message box style
	CStringArray	m_TextLine;	// message box text lines
	int		m_LineHeight;	// height of a text line, in logical units
	static	UINT	m_InstCount;	// current number of our instances
	bool	m_AutoDelete;	// true if instance should delete itself
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGBOXDLG_H__198B7582_3662_4DA1_95B1_FAC99E3E05EC__INCLUDED_)
