// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		26jul06	initial version
		01		07oct06	add modulation enable
		02		29oct06	add focus checkbox
		03		03nov06	add MIDI info
		04		22nov06	add undo
		05		01feb07	in SetModWaveNorm, use trunc instead of round
		06		23nov07	support Unicode
		07		15jan08	replace OnNotify with individual handlers
		08		19apr10	remove oscillator
		09		30may11	remove legacy OnCancel

		freeframe parameter row dialog

*/

#if !defined(AFX_FFPLUGSROW_H__22D183C2_B1F0_446A_8845_FDBD5E2F43F3__INCLUDED_)
#define AFX_FFPLUGSROW_H__22D183C2_B1F0_446A_8845_FDBD5E2F43F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FFPlugsRow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFFPlugsRow dialog

#include "RowDlg.h"
#include "SelectSliderCtrl.h"
#include "NumSpin.h"
#include "Oscillator.h"
#include "FocusCheckbox.h"
#include "FFPlugInfo.h"

class CFFPlugsDlg;

class CFFPlugsRow : public CRowDlg
{
	DECLARE_DYNAMIC(CFFPlugsRow);
// Construction
public:
	CFFPlugsRow();

// Types
	typedef CFFPlugInfo::FFPARM_INFO FFPARM_INFO;
	typedef CFFPlugInfo::FRANGE FRANGE;

// Attributes
	void	GetInfo(FFPARM_INFO& Info) const;
	void	SetInfo(const FFPARM_INFO& Info);
	void	SetName(LPCTSTR Name);
	float	GetVal() const;
	void	SetVal(float Val);
	bool	GetModEnable() const;
	void	SetModEnable(bool Enable);
	int		GetModWave() const;
	void	SetModWave(int Wave);
	float	GetModFreq() const;
	void	SetModFreq(float Freq);
	float	GetModPulseWidth() const;
	void	SetModPulseWidth(float PulseWidth);
	bool	SliderHasSel() const;
	FRANGE	GetModRange() const;
	void	GetModRange(float& Start, float& End) const;
	void	SetModRange(FRANGE Range);
	void	SetModRange(float Start, float End);

// Operations
	bool	EditModRange(LPCTSTR Caption);
	void	SetModRangeStart();
	void	SetModRangeEnd();
	void	RemoveModRange();
	void	GotoModRangeStart();
	void	GotoModRangeEnd();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFPlugsRow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CFFPlugsRow)
	enum { IDD = IDD_FF_PLUGS_ROW };
	CFocusCheckbox	m_ModEnabChk;
	CNumSpin	m_ModPWSpin;
	CNumEdit	m_ModPW;
	CNumSpin	m_ModFreqSpin;
	CComboBox	m_ModWave;
	CNumEdit	m_ModFreq;
	CNumSpin	m_ParmSpin;
	CNumEdit	m_ParmEdit;
	CSelectSliderCtrl	m_ParmSlider;
	CStatic	m_Name;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CFFPlugsRow)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeModWave();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnParmName();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnModEnab();
	afx_msg void OnChangedParmEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedModFreq(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangedModPW(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	enum {
		WAVEFORMS = COscillator::WAVEFORMS
	};
	static const int m_WaveID[WAVEFORMS];
	static const CEditSliderCtrl::INFO m_SliderInfo;

// Member data

// Overrides

// Helpers
	void	NotifyEdit(int CtrlID);
	void	SetValFromSlider();
	void	UpdateOscFreq();
};

inline float CFFPlugsRow::GetVal() const
{
	return(float(m_ParmSlider.GetVal()));
}

inline bool CFFPlugsRow::GetModEnable() const
{
	return(m_ModEnabChk.GetCheck() != 0);
}

inline int CFFPlugsRow::GetModWave() const
{
	return(m_ModWave.GetCurSel());
}

inline float CFFPlugsRow::GetModFreq() const
{
	return(float(m_ModFreq.GetVal()));
}

inline float CFFPlugsRow::GetModPulseWidth() const
{
	return(float(m_ModPW.GetVal()));
}

inline void CFFPlugsRow::SetValFromSlider()
{
	SetVal(float(m_ParmSlider.GetVal()));
}

inline bool CFFPlugsRow::SliderHasSel() const
{
	return(m_ParmSlider.HasSelection());
}

inline void CFFPlugsRow::GetModRange(float& Start, float& End) const
{
	m_ParmSlider.GetNormSelection(Start, End);
}

inline CFFPlugInfo::FRANGE CFFPlugsRow::GetModRange() const
{
	FRANGE	Range;
	GetModRange(Range.Start, Range.End);
	return(Range);
}

inline void CFFPlugsRow::SetModRange(FRANGE Range)
{
	SetModRange(Range.Start, Range.End);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFPLUGSROW_H__22D183C2_B1F0_446A_8845_FDBD5E2F43F3__INCLUDED_)
