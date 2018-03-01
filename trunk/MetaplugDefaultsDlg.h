// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02feb07	initial version

        metaplugin defaults dialog
 
*/

#if !defined(AFX_METAPLUGDEFAULTSDLG_H__3359879D_F8DE_40C5_98D5_54E0A7A4F7D6__INCLUDED_)
#define AFX_METAPLUGDEFAULTSDLG_H__3359879D_F8DE_40C5_98D5_54E0A7A4F7D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetaplugDefaultsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetaplugDefaultsDlg dialog

class CMetaplugDefaultsDlg : public CDialog
{
	DECLARE_DYNCREATE(CMetaplugDefaultsDlg)
// Construction
public:
	CMetaplugDefaultsDlg(CWnd* pParent = NULL);   // standard constructor
	~CMetaplugDefaultsDlg();

// Dialog Data
	//{{AFX_DATA(CMetaplugDefaultsDlg)
	enum { IDD = IDD_METAPLUG_DEFAULTS };
	CString	m_About;
	CString	m_Descrip;
	CString	m_UniqueID;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaplugDefaultsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(CMetaplugDefaultsDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METAPLUGDEFAULTSDLG_H__3359879D_F8DE_40C5_98D5_54E0A7A4F7D6__INCLUDED_)
