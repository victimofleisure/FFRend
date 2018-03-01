// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      07aug06	initial version
		01		05nov06	add duration unit
		02		02aug07	add queue job checkbox
		03		06aug07	use output frame rate to convert seconds
		04		07aug07	add get/set info
		05		23nov07	support Unicode

        record dialog
 
*/

#if !defined(AFX_RECORDDLG_H__1A0DA86B_5701_4E0F_A0D3_31F902BE08FA__INCLUDED_)
#define AFX_RECORDDLG_H__1A0DA86B_5701_4E0F_A0D3_31F902BE08FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

#include "NumEdit.h"
#include "FrameSizeCtrl.h"

class CMainFrame;
class CRecordInfo;

class CRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecordDlg);
// Construction
public:
	CRecordDlg(CWnd* pParent = NULL);   // standard constructor
	~CRecordDlg();

// Attributes
	CSize	GetFrameSize() const;
	float	GetFrameRate() const;
	int		GetBitCount() const;
	int		GetDuration() const;
	int		GetFrameCount() const;
	bool	UseAviLength() const;
	bool	QueueJob() const;
	void	GetInfo(CRecordInfo& Info) const;
	void	SetInfo(const CRecordInfo& Info);

// Operations
	static	void	FrameToTime(int Frame, float FrameRate, CString& Time);
	static	int		TimeToFrame(LPCTSTR Time, float FrameRate);
	static	void	SecsToTime(int Secs, CString& Time);
	static	int		TimeToSecs(LPCTSTR Time);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CRecordDlg)
	enum { IDD = IDD_RECORD };
	CButton	m_QueueJobChk;
	CNumEdit	m_OutFrameRateEdit;
	CButton	m_UseInpFrameRateBtn;
	CButton	m_UseInpFrameSizeBtn;
	CEdit	m_DurationEdit;
	CStatic	m_InpFrameRateStat;
	CStatic	m_InpFrameSizeStat;
	CComboBox	m_BitCountCombo;
	CNumEdit	m_OutFrameWidth;
	CComboBox	m_OutFrameSizeCombo;
	CNumEdit	m_OutFrameHeight;
	int		m_DurationType;
	int		m_DurationUnit;
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CRecordDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFrameSize();
	afx_msg void OnKillfocusDuration();
	afx_msg void OnUseInpFrameSize();
	afx_msg void OnUseInpFrameRate();
	afx_msg void OnDurationType();
	afx_msg void OnDurationUnit();
	afx_msg void OnKillfocusOutFrameRate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Constants
  	enum {	// preset bit counts
		PBC_16,
		PBC_24,
		PBC_32,
		PBC_PRESETS
	};
	enum {
		DEF_FRAME_RATE = 25,	// default frame rate
		DEF_BIT_COUNT = 24,		// default bit count
		DEF_DURATION = 30,		// default duration
		DEF_FRAME_COUNT = DEF_DURATION * DEF_FRAME_RATE,	// default frame count
	};
	enum {	// duration types
		DT_CUSTOM,				// user enters a duration
		DT_UNLIMITED,			// record until user stops
		DT_AVI_LENGTH,			// get duration from AVI file
		DURATION_TYPES
	};
	enum {	// duration units
		DU_SECONDS,
		DU_FRAMES,
		DURATION_UNITS
	};
	static const int	m_PresetBitCount[PBC_PRESETS];
	static const SIZE	DEF_FRAME_SIZE;

// Member data
	// CRecordInfo members; REMEMBER to maintain Get/SetInfo in parallel
	CFrameSizeCtrl	m_OutFrameSize;	// output frame size control
	float	m_OutFrameRate;		// output frame rate
	int		m_BitCount;			// output bits per pixel
	int		m_Duration;			// recording length in seconds
	int		m_FrameCount;		// recording length in frames
	bool	m_Unlimited;		// if true, record until stopped by user
	bool	m_UseAviLength;		// if true, get duration from source AVI file
	bool	m_UseInpFrameSize;	// if true, force output frame size to match input
	bool	m_UseInpFrameRate;	// if true, force output frame rate to match input
	bool	m_QueueJob;			// if true, don't run job now, add to job control
	// members not in CRecordInfo
	CSize	m_InpFrameSize;		// frame size of plugin chain
	float	m_InpFrameRate;		// frame rate of plugin chain
	int		m_CurDuration;		// current recording length in seconds
	int		m_CurFrameCount;	// current recording length in frames
	float	m_CurOutFrameRate;	// current output frame rate

// Helpers
	void	UpdateUI();
	void	UpdateDuration();
	void	InitBitCountCombo();
	int		SecsToFrames(int Secs) const;
	int		FramesToSecs(int Frames) const;
};

inline CSize CRecordDlg::GetFrameSize() const
{
	return(m_OutFrameSize.m_Size);
}

inline float CRecordDlg::GetFrameRate() const
{
	return(m_OutFrameRate);
}

inline int CRecordDlg::GetBitCount() const
{
	return(m_BitCount);
}

inline int CRecordDlg::GetDuration() const
{
	return(m_Unlimited ? 0 : m_Duration);
}

inline int CRecordDlg::GetFrameCount() const
{
	return(m_Unlimited ? 0 : m_FrameCount);
}

inline bool CRecordDlg::UseAviLength() const
{
	return(m_UseAviLength);
}

inline int CRecordDlg::SecsToFrames(int Secs) const
{
	return(round(Secs * m_CurOutFrameRate));
}

inline int CRecordDlg::FramesToSecs(int Frames) const
{
	return(round(Frames / m_CurOutFrameRate));
}

inline bool CRecordDlg::QueueJob() const
{
	return(m_QueueJob);
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDDLG_H__1A0DA86B_5701_4E0F_A0D3_31F902BE08FA__INCLUDED_)
