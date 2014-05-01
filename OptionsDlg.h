// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29mar10	initial version
		01      04may11	convert from dialog to property sheet
		02		01dec11	add run while loading, frame memory limit
		03		23jan12	add check for updates

        options dialog
 
*/

#if !defined(AFX_OPTIONSDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
#define AFX_OPTIONSDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

#include "OptionsInfo.h"
#include "OptsEngineDlg.h"
#include "OptsViewDlg.h"
#include "OptsMidiDlg.h"

class COptionsDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsDlg)
// Construction
public:
	COptionsDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
	void	GetEngineOptions(CEngineOptions& Opts) const;
	void	SetEngineOptions(const CEngineOptions& Opts);
	void	GetInfo(COptionsInfo& Info) const;
	void	SetInfo(const COptionsInfo& Info);
	const COptionsInfo& GetPrevOpts() const;
	CSize	GetFrameSize() const;
	float	GetFrameRate() const;
	UINT	GetFrameTimeout() const;
	int		GetHistorySize() const;
	float	GetViewFreq() const;
	bool	GetCacheThumbs() const;
	CSize	GetThumbSize() const;
	int		GetMidiDevice() const;
	CString	GetMidiDeviceName() const;
	bool	SetMidiDeviceName(const CString& DevName);
	UINT	GetColorDepth() const;
	void	SetColorDepth(UINT ColorDepth);
	UINT	GetRandomSeed(bool& UseTime) const;
	bool	GetMonitorQuality() const;
	int		GetUndoLevels() const;
	bool	GetSaveChgsWarn() const;
	bool	GetLockFrameRate() const;
	bool	GetUseMMTimer() const;
	bool	GetRunWhileLoading() const;
	UINT	GetFrameMemoryLimit() const;
	bool	GetCheckForUpdates() const;

// Operations
	void	RestoreDefaults();
	const COptionsInfo&	GetDefaults() const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	virtual BOOL OnInitDialog();
	virtual W64INT DoModal();
	//}}AFX_VIRTUAL

// Implementation
protected:
// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	afx_msg void OnDestroy();
	afx_msg void OnResetAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
	static const ENGINE_OPTIONS	m_DefaultEngineOptions;
	static const OPTIONS_INFO	m_DefaultOptionsInfo;
	static const COptionsInfo	m_Defaults;

// Member data
	COptionsInfo	m_oi;			// current options
	COptionsInfo	m_PrevInfo;		// previous options
	COptsEngineDlg	m_EngineDlg;	// engine page
	COptsViewDlg	m_ViewDlg;		// view page
	COptsMidiDlg	m_MidiDlg;		// MIDI page
	CButton	m_ResetAll;				// reset all button
	int		m_CurPage;				// index of current page

// Helpers
	void	CreateResetAllButton();
};

inline void COptionsDlg::GetEngineOptions(CEngineOptions& Opts) const
{
	Opts = m_oi;
}

inline void COptionsDlg::SetEngineOptions(const CEngineOptions& Opts)
{
	CEngineOptions&	MyOpts = m_oi;
	MyOpts = Opts;
}

inline void COptionsDlg::GetInfo(COptionsInfo& Info) const
{
	Info = m_oi;
}

inline void COptionsDlg::SetInfo(const COptionsInfo& Info)
{
	m_oi = Info;
}

inline const COptionsInfo& COptionsDlg::GetPrevOpts() const
{
	return(m_PrevInfo);
}

inline CSize COptionsDlg::GetFrameSize() const
{
	return(m_oi.m_FrameSize);
}

inline float COptionsDlg::GetFrameRate() const
{
	return(m_oi.m_FrameRate);
}

inline UINT COptionsDlg::GetFrameTimeout() const
{
	return(m_oi.m_FrameTimeout);
}

inline int COptionsDlg::GetHistorySize() const
{
	return(m_oi.m_HistorySize);
}

inline float COptionsDlg::GetViewFreq() const
{
	return(m_oi.m_ViewFreq);
}

inline bool COptionsDlg::GetCacheThumbs() const
{
	return(m_oi.m_CacheThumbs != 0);
}

inline CSize COptionsDlg::GetThumbSize() const
{
	return(m_oi.m_ThumbSize);
}

inline int COptionsDlg::GetMidiDevice() const
{
	return(m_MidiDlg.GetMidiDevice());
}

inline UINT COptionsDlg::GetColorDepth() const
{
	return(m_oi.m_ColorDepth);
}

inline void COptionsDlg::SetColorDepth(UINT ColorDepth)
{
	m_oi.m_ColorDepth = ColorDepth;
}

inline UINT COptionsDlg::GetRandomSeed(bool& UseTime) const
{
	UseTime = m_oi.m_RandUseTime != 0;
	return(m_oi.m_RandSeed);
}

inline bool COptionsDlg::GetMonitorQuality() const
{
	return(m_oi.m_MonitorQuality);
}

inline int COptionsDlg::GetUndoLevels() const
{
	return(m_oi.m_UndoUnlimited ? -1 : m_oi.m_UndoLevels);
}

inline bool COptionsDlg::GetSaveChgsWarn() const
{
	return(m_oi.m_SaveChgsWarn);
}

inline bool COptionsDlg::GetLockFrameRate() const
{
	return(m_oi.m_LockFrameRate);
}

inline bool COptionsDlg::GetUseMMTimer() const
{
	return(m_oi.m_UseMMTimer);
}

inline bool COptionsDlg::GetRunWhileLoading() const
{
	return(m_oi.m_RunWhileLoading);
}

inline UINT COptionsDlg::GetFrameMemoryLimit() const
{
	return(m_oi.m_FrameMemoryLimit);
}

inline bool COptionsDlg::GetCheckForUpdates() const
{
	return(m_oi.m_CheckForUpdates);
}

inline const COptionsInfo& COptionsDlg::GetDefaults() const
{
	return(m_Defaults);
}

inline void AFXAPI DDX_Check(CDataExchange* pDX, int nIDC, bool& value)
{
	BOOL	v = value;
	DDX_Check(pDX, nIDC, v);
	value = v != 0;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDLG_H__84776AB1_689B_46EE_84E6_931C1542871D__INCLUDED_)
