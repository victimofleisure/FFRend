// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      15aug06	initial version
		01		23apr10	convert from dialog bar

        master dialog

*/

#if !defined(AFX_MASTERDLG_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
#define AFX_MASTERDLG_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MasterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMasterDlg dialog

#include "EditSliderCtrl.h"
#include "CtrlResize.h"

class CMasterDlg : public CDialog
{
	DECLARE_DYNAMIC(CMasterDlg);
// Construction
public:
	CMasterDlg(CWnd* pParent = NULL);   // standard constructor

// Attributes
public:
	double	GetSpeed() const;
	void	SetSpeed(double Speed);
	double	GetSpeedNorm() const;
	void	SetSpeedNorm(double Speed);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMasterDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMasterDlg)
	enum { IDD = IDD_MASTER };
	CStatic	m_SpeedCap;
	CEditSliderCtrl	m_SpeedSlider;
	CNumEdit	m_SpeedEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMasterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangedSpeedEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const CEditSliderCtrl::INFO m_SliderInfo;
	static const CCtrlResize::CTRL_LIST	m_CtrlList[];
	
// Member data
	CCtrlResize	m_Resize;
};

inline double CMasterDlg::GetSpeed() const
{
	return(m_SpeedSlider.GetVal());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASTERDLG_H__3952AAB6_552B_45CB_B970_07578B3A2C6F__INCLUDED_)
