// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01dec11	initial version

        advanced engine options dialog
 
*/

// AdvancedEngineOptsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "AdvancedEngineOptsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAdvancedEngineOptsDlg dialog


CAdvancedEngineOptsDlg::CAdvancedEngineOptsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvancedEngineOptsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvancedEngineOptsDlg)
	m_RunWhileLoading = FALSE;
	m_FrameMemoryLimit = 0;
	m_FrameMemoryUsed = 0;
	//}}AFX_DATA_INIT
}

void CAdvancedEngineOptsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvancedEngineOptsDlg)
	DDX_Check(pDX, IDC_AEOP_RUN_WHILE_LOADING, m_RunWhileLoading);
	DDX_Text(pDX, IDC_AEOP_FRAME_MEMORY_LIMIT, m_FrameMemoryLimit);
	DDV_MinMaxUInt(pDX, m_FrameMemoryLimit, 1, 4096);
	DDX_Text(pDX, IDC_AEOP_FRAME_MEMORY_USED, m_FrameMemoryUsed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdvancedEngineOptsDlg, CDialog)
	//{{AFX_MSG_MAP(CAdvancedEngineOptsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedEngineOptsDlg message handlers

BOOL CAdvancedEngineOptsDlg::OnInitDialog() 
{
	CEngine&	eng = theApp.GetEngine();
	int	FrameLengthKB = eng.GetFrameLength() >> 10;	// math in KB
	m_FrameMemoryUsed = (eng.GetFrameCount() * FrameLengthKB) >> 10;
	
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
