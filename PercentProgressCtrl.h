// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01may11	initial version

		progress control with percentage overlay
 
*/

#if !defined(AFX_PERCENTPROGRESSCTRL_H__FC6B09C7_219B_41EF_AE36_37DA462AD113__INCLUDED_)
#define AFX_PERCENTPROGRESSCTRL_H__FC6B09C7_219B_41EF_AE36_37DA462AD113__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PercentProgressCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPercentProgressCtrl window

class CPercentProgressCtrl : public CProgressCtrl
{
// Construction
public:
	CPercentProgressCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPercentProgressCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPercentProgressCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPercentProgressCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERCENTPROGRESSCTRL_H__FC6B09C7_219B_41EF_AE36_37DA462AD113__INCLUDED_)
