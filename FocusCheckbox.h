// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29oct06	initial version

        uncaptioned checkbox with focus rectangle
 
*/

#if !defined(AFX_FOCUSCHECKBOX_H__357C23E1_D0F9_4B54_B87A_D7CC674705CF__INCLUDED_)
#define AFX_FOCUSCHECKBOX_H__357C23E1_D0F9_4B54_B87A_D7CC674705CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FocusCheckbox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFocusCheckbox window

class CFocusCheckbox : public CButton
{
	DECLARE_DYNAMIC(CFocusCheckbox);
// Construction
public:
	CFocusCheckbox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFocusCheckbox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFocusCheckbox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFocusCheckbox)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOCUSCHECKBOX_H__357C23E1_D0F9_4B54_B87A_D7CC674705CF__INCLUDED_)
