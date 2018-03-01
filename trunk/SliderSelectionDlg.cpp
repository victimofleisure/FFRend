// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      29jul06	initial version

        dialog for editing slider selection
 
*/

// SliderSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "SliderSelectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSliderSelectionDlg dialog


CSliderSelectionDlg::CSliderSelectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSliderSelectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSliderSelectionDlg)
	m_End = 0.0f;
	m_Start = 0.0f;
	//}}AFX_DATA_INIT
}


void CSliderSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSliderSelectionDlg)
	DDX_Text(pDX, IDC_SLIDER_SEL_END, m_End);
	DDV_MinMaxFloat(pDX, m_End, -1.f, 1.f);
	DDX_Text(pDX, IDC_SLIDER_SEL_START, m_Start);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSliderSelectionDlg, CDialog)
	//{{AFX_MSG_MAP(CSliderSelectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSliderSelectionDlg message handlers

BOOL CSliderSelectionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (!m_Caption.IsEmpty())
		SetWindowText(m_Caption);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
