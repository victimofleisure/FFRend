// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29jul06	initial version

        dialog for editing slider selection
 
*/

#if !defined(AFX_SLIDERSELECTIONDLG_H__2AEF3D1E_C9BF_4F99_8057_98A7AFD5C140__INCLUDED_)
#define AFX_SLIDERSELECTIONDLG_H__2AEF3D1E_C9BF_4F99_8057_98A7AFD5C140__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SliderSelectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSliderSelectionDlg dialog

class CSliderSelectionDlg : public CDialog
{
// Construction
public:
	CSliderSelectionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSliderSelectionDlg)
	enum { IDD = IDD_SLIDER_SELECTION };
	float	m_End;
	float	m_Start;
	//}}AFX_DATA
	CString	m_Caption;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSliderSelectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSliderSelectionDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERSELECTIONDLG_H__2AEF3D1E_C9BF_4F99_8057_98A7AFD5C140__INCLUDED_)
