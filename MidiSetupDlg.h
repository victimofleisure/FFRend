// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03nov06	initial version
		01		26nov06	distinguish dialog rows from MIDI info rows
        02      29dec06	remove OnShowWindow, set visible in dialog resource
		03		19jan07	convert from dialog to control bar
		04		31jan07	move FastIsVisible into base class
		05		22dec07	add ParmToRow so plugin page can handle empty slots
		06		22dec07	add GetPageType
		07		29jan08	replace CDWordArray with CIdxArray
		08		27apr10	refactor for engine
		09		24nov11	add get/set scroll position
		10		18may12	use extended tab control for wheel scrolling

		MIDI setup dialog
 
*/

#if !defined(AFX_MIDISETUPDLG_H__0AC35BFF_87FD_4FA5_B639_E8A8D39DD0DB__INCLUDED_)
#define AFX_MIDISETUPDLG_H__0AC35BFF_87FD_4FA5_B639_E8A8D39DD0DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MidiSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMidiSetupDlg dialog

#include "ArrayEx.h"
#include "RowView.h"
#include "MidiSetupRow.h"
#include "MidiInfo.h"
#include "TabCtrlEx.h"

class CMainFrame;
class CMidiSetupView;
class CMidiEngine;

class CMidiSetupDlg : public CDialog
{
	DECLARE_DYNCREATE(CMidiSetupDlg)
// Construction
public:
	CMidiSetupDlg(CWnd* pParent = NULL);   // standard constructor

// Constants
	enum {	// columns
		COL_TITLE,
		COL_RANGE,
		COL_EVENT,
		COL_CHAN,
		COL_CTRL,
		COL_VALUE,
		COLS
	};

// Types

// Attributes
	int		GetRowCount(int SlotIdx) const;
	int		GetPageType() const;
	int		GetPlugSel() const;
	int		GetPropSel() const;
	int		GetTabSel() const;
	int		GetRowSel() const;
	int		GetPluginCount() const;
	void	SetLearn(bool Enable);
	bool	IsLearning() const;
	bool	IsShowingRowSel() const;
	void	SetRowInfo(int RowIdx, CMidiSetupRow& Row);

// Operations
	void	UpdateView();
	void	SetDefaults();
	void	UpdateMidiInfo();
	void	ShowRowSel(bool Enable);
	void	SelectTab(int TabIdx);
	void	SelectRow(int RowIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMidiSetupDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMidiSetupDlg)
	enum { IDD = IDD_MIDI_SETUP };
	CButton	m_LearnChk;
	CComboBox	m_PropCombo;
	CTabCtrlEx	m_TabCtrl;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMidiSetupDlg)
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnLearn();
	afx_msg void OnSelchangeProp();
	//}}AFX_MSG
	afx_msg LRESULT OnMidiRowEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMidiRowSel(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMidiIn(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		TAB_CTRL_HEIGHT = 28,		// height of tab control
		TAB_CTRL_OVERLAP = 2,		// overlap of tab control onto row view header
	};
	enum { // child control coords
		PROP_COMBO_X = 4,
		PROP_COMBO_Y = 4,
		PROP_COMBO_WIDTH = 120,
		PROP_COMBO_HEIGHT = 100,
		LEARN_CHK_X = PROP_COMBO_X + PROP_COMBO_WIDTH + 20,
		LEARN_CHK_Y = 6,
		LEARN_CHK_WIDTH = 100,
		LEARN_CHK_HEIGHT = 16
	};
	static const CRowView::COLINFO	m_ColInfo[COLS];
	static const CRect	m_PropRect;
	static const CRect	m_LearnRect;

// Types
	typedef CArrayEx<int, int> CIdxArray;

// Member data
	CMidiSetupView	*m_View;	// pointer to row view
	CMidiEngine	*m_Engine;	// pointer to engine
	CIdxArray	m_TabPlug;	// for each tab, index of its plugin, or a special tab ID
	int		m_PageType;		// MIDI page type; see CMainFrame.h
	int		m_PlugSel;		// slot index of current plugin selection
	int		m_RowSel;		// current row selection, or -1 if none
	int		m_PropSel;		// current property selection
	int		m_PropSelList[MIDI_PAGE_TYPES];	// property selection for each page type
	int		m_PluginCount;	// our plugin count; may differ from main frame's
	bool	m_ShowRowSel;	// true if row selection is visible
	bool	m_Learn;		// true if we're in learn mode
	int		m_InitTabCtrlY;	// initial tab control y-pos in client coords
	CPoint	m_ScrollPos[MIDI_PAGE_TYPES];	// scroll position for each page type

// Helpers
	CMidiSetupRow	*GetRow(int Idx) const;
	int		RowToParm(int SlotIdx, int RowIdx) const;
	int		ParmToRow(int SlotIdx, int ParmIdx) const;
	int		GetPageType(int SlotIdx) const;
	int		GetTabIdx(int PageType) const;
	void	PopulatePropCombo(int PageType);
	void	Assign(int RowIdx, const CMidiInfo& Info);
	CPoint	GetScrollPos(int SlotIdx) const;
	void	SetScrollPos(int SlotIdx, CPoint ScrollPos);
};

class CMidiSetupView : public CRowView {
public:
	DECLARE_DYNCREATE(CMidiSetupView);
	CRowDlg	*CreateRow(int Idx);
	void	UpdateRow(int Idx);
	CMidiSetupDlg	*m_Parent;
};

inline CMidiSetupRow *CMidiSetupDlg::GetRow(int Idx) const
{
	return((CMidiSetupRow *)m_View->GetRow(Idx));
}

inline int CMidiSetupDlg::GetPageType() const
{
	return(m_PageType);
}

inline int CMidiSetupDlg::GetPlugSel() const
{
	return(m_PlugSel);
}

inline int CMidiSetupDlg::GetPropSel() const
{
	return(m_PropSel);
}

inline int CMidiSetupDlg::GetTabSel() const
{
	return(m_TabCtrl.GetCurSel());
}

inline int CMidiSetupDlg::GetRowSel() const
{
	return(m_RowSel);
}

inline bool CMidiSetupDlg::IsLearning() const
{
	return(m_Learn);
}

inline bool CMidiSetupDlg::IsShowingRowSel() const
{
	return(m_ShowRowSel);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIDISETUPDLG_H__0AC35BFF_87FD_4FA5_B639_E8A8D39DD0DB__INCLUDED_)
