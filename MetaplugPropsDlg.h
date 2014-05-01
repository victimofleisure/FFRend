// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version
		01		02feb07	add defaults dialog

        metaplugin properties dialog
 
*/

#if !defined(AFX_METAPLUGPROPSDLG_H__EDEBE80E_0935_42EA_AB21_55B9697CE044__INCLUDED_)
#define AFX_METAPLUGPROPSDLG_H__EDEBE80E_0935_42EA_AB21_55B9697CE044__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetaplugPropsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetaplugPropsDlg dialog

#include "MetaplugDefaultsDlg.h"

class CMetaplugin;

class CMetaplugPropsDlg : public CDialog
{
	DECLARE_DYNCREATE(CMetaplugPropsDlg)
// Construction
public:
	CMetaplugPropsDlg(CMetaplugin& Metaplug, LPCTSTR FileName, CWnd* pParent = NULL);

// Operations
	static	bool	EmbedCheck(CString& NonFreePlugs);
	bool	GetModified() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaplugPropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMetaplugPropsDlg)
	enum { IDD = IDD_METAPLUG_PROPS };
	CButton	m_Embed;
	CEdit	m_Name;
	CEdit	m_UniqueID;
	CString	m_About;
	CString	m_Descrip;
	int		m_Type;
	UINT	m_VersionMinor;
	UINT	m_VersionMajor;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMetaplugPropsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditInputs();
	afx_msg void OnDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Data members
	CMetaplugin&	m_Metaplug;	// reference to metaplugin data
	CMetaplugDefaultsDlg	m_DefaultsDlg;	// metaplugin defaults dialog
	CString	m_FileName;	// metaplugin filename, for setting default plugin name
	bool	m_ModFlag;	// true if properties were changed
};

inline bool CMetaplugPropsDlg::GetModified() const
{
	return(m_ModFlag);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METAPLUGPROPSDLG_H__EDEBE80E_0935_42EA_AB21_55B9697CE044__INCLUDED_)
