// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04may11	initial version

        MIDI options dialog
 
*/

#if !defined(AFX_OPTSMIDIDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTSMIDIDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptsMidiDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptsMidiDlg dialog

#include "OptionsInfo.h"

class COptsMidiDlg : public CPropertyPage
{
// Construction
public:
	COptsMidiDlg(COptionsInfo& Info);
	~COptsMidiDlg();

// Attributes
	int		GetMidiDevice() const;
	CString	GetMidiDeviceName() const;
	bool	SetMidiDeviceName(const CString& DevName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptsMidiDlg)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptsMidiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMidiRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dialog data
	//{{AFX_DATA(COptsMidiDlg)
	enum { IDD = IDD_OPTS_MIDI };
	CComboBox	m_MidiDevCombo;
	//}}AFX_DATA

// Constants

// Member data
	COptionsInfo&	m_oi;		// reference to parent's options info 
	int		m_MidiDev;			// index of MIDI device in m_DevList
	CStringArray	m_DevList;	// MIDI device list

// Helpers
	void	UpdateDevCombo();
	void	UpdateUI();
};

inline int COptsMidiDlg::GetMidiDevice() const
{
	return(m_MidiDev);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTSMIDIDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
