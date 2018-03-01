// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
		01		29jan08	replace CDWordArray with CIdxArray
		02		01may10	refactor for engine

        metaparameter properties dialog
 
*/

#if !defined(AFX_METAPARMPROPSDLG_H__EDEBE80E_0935_42EA_AB21_55B9697CE044__INCLUDED_)
#define AFX_METAPARMPROPSDLG_H__EDEBE80E_0935_42EA_AB21_55B9697CE044__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetaparmPropsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetaparmPropsDlg dialog

#include "ArrayEx.h"
#include "NumEdit.h"
#include "NumSpin.h"
#include "Metaparm.h"
#include "MidiInfo.h"

class CMainFrame;

class CMetaparmPropsDlg : public CDialog
{
	DECLARE_DYNCREATE(CMetaparmPropsDlg)
// Construction
public:
	CMetaparmPropsDlg(CMetaparm& Parm, CWnd* pParent = NULL);

// Types
	typedef CMetaparm::TARGET TARGET;

// Operations
	static	void	MakeMetaparmName(const TARGET& Target, CString& Name);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaparmPropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMetaparmPropsDlg)
	enum { IDD = IDD_METAPARM_PROPS };
	CStatic	m_TargCapProp;
	CStatic	m_TargCapPlug;
	CStatic	m_TargCapParm;
	CNumSpin	m_RangeStartSpin;
	CNumSpin	m_RangeEndSpin;
	CNumEdit	m_RangeEndEdit;
	CNumEdit	m_RangeStartEdit;
	CComboBox	m_PropCombo;
	CComboBox	m_PluginCombo;
	CComboBox	m_ParamCombo;
	CEdit	m_NameEdit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMetaparmPropsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePlugin();
	afx_msg void OnSelchangeParam();
	afx_msg void OnSelchangeProp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef struct tagTARGET_CAPTION {	// target combo box captions
		int		PlugId;	// string resource ID of plugin caption
		int		ParmId;	// string resource ID of parameter caption
		int		PropId;	// string resource ID of property caption
	} TARGET_CAPTION;
	typedef CArrayEx<int, int> CIdxArray;

// Constants
	static const int m_ParamPropNameId[PARAM_MIDI_PROPS];
	static const TARGET_CAPTION m_TargCap[MIDI_PAGE_TYPES];

// Data members
	CMainFrame	*m_Frm;		// pointer to main frame
	CMetaparm&	m_Parm;		// reference to caller's metaparameter
	CIdxArray	m_PlugIdx;	// maps plugin combo box items to plugin indices
	int		m_PrevPageType;	// previous page type

// Helpers
	bool	IsAssigned() const;
	void	InitCombos();
	void	SetPlugin(int PlugSel);
	bool	GetTarget(TARGET& Target) const;
	void	UpdateUI();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METAPARMPROPSDLG_H__EDEBE80E_0935_42EA_AB21_55B9697CE044__INCLUDED_)
