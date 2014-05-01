// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      03nov06	initial version
		01		23nov07	support Unicode
		02		15jan08	replace OnNotify with individual handlers
		03		27apr10	rename row dialog base class

		MIDI setup dialog row
 
*/

#if !defined(AFX_MIDISETUPROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_)
#define AFX_MIDISETUPROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiSetupRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupRow dialog

#include "RowDlg.h"
#include "NoteEdit.h"
#include "NumSpin.h"
#include "MidiInfo.h"

class CMidiSetupRow : public CRowDlg
{
	DECLARE_DYNAMIC(CMidiSetupRow);
// Construction
public:
	CMidiSetupRow(CWnd* pParent = NULL);   // standard constructor

// Types

// Constants

// Attributes
	void	GetInfo(CMidiInfo& Info) const;
	void	SetInfo(const CMidiInfo& Info);
	void	Assign(int Event, int Chan, int Ctrl);
	void	SetEvent(int Event);
	int		GetEvent() const;
	float	GetRange() const;
	void	SetCaption(LPCTSTR Title);
	void	SetValue(int Val);

// Operations
	void	SetSelected(bool Enable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiSetupRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMidiSetupRow)
	enum { IDD = IDD_MIDI_SETUP_ROW };
	CNumSpin	m_RangeSpin;
	CNumSpin	m_CtrlSpin;
	CNumSpin	m_ChanSpin;
	CComboBox	m_Event;
	CNumEdit	m_Value;
	CNumEdit	m_Chan;
	CNoteEdit	m_Ctrl;
	CStatic	m_Title;
	CNumEdit	m_Range;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMidiSetupRow)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeEvent();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnChangedNumEdit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const COLORREF m_SelColor;	// selection color
	static const CBrush	m_SelBrush;		// selection brush

// Data members
	bool	m_Selected;	// true if we're selected

// Helpers
};

inline float CMidiSetupRow::GetRange() const
{
	return(float(m_Range.GetVal()));
}

inline int CMidiSetupRow::GetEvent() const
{
	return(m_Event.GetCurSel());
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDISETUPROW_H__5B1D8F56_9A40_44F1_AD9E_7CB1DE873C7A__INCLUDED_)
