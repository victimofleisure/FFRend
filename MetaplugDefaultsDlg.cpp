// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02feb07	initial version
		01		23nov07	support Unicode

        metaplugin defaults dialog
 
*/

// MetaplugDefaultsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MetaplugDefaultsDlg.h"
#include "Persist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetaplugDefaultsDlg dialog

#define RK_META_DEF_ABOUT		_T("MetaDefAbout")
#define RK_META_DEF_DESCRIP		_T("MetaDefDescrip")
#define RK_META_DEF_UNIQUE_ID	_T("MetaDefUniqueID")

IMPLEMENT_DYNAMIC(CMetaplugDefaultsDlg, CDialog);

CMetaplugDefaultsDlg::CMetaplugDefaultsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMetaplugDefaultsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMetaplugDefaultsDlg)
	//}}AFX_DATA_INIT
	m_About = CPersist::GetString(REG_SETTINGS, RK_META_DEF_ABOUT, _T("Copyleft"));
	m_Descrip = CPersist::GetString(REG_SETTINGS, RK_META_DEF_DESCRIP, _T("Metaplugin"));
	m_UniqueID = CPersist::GetString(REG_SETTINGS, RK_META_DEF_UNIQUE_ID, _T("MFFR"));
}

CMetaplugDefaultsDlg::~CMetaplugDefaultsDlg()
{
	CPersist::WriteString(REG_SETTINGS, RK_META_DEF_ABOUT, m_About);
	CPersist::WriteString(REG_SETTINGS, RK_META_DEF_DESCRIP, m_Descrip);
	CPersist::WriteString(REG_SETTINGS, RK_META_DEF_UNIQUE_ID, m_UniqueID);
}

void CMetaplugDefaultsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetaplugDefaultsDlg)
	DDX_Text(pDX, IDC_METAPLUG_ABOUT, m_About);
	DDX_Text(pDX, IDC_METAPLUG_DESCRIP, m_Descrip);
	DDX_Text(pDX, IDC_METAPLUG_UNIQUE_ID, m_UniqueID);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMetaplugDefaultsDlg, CDialog)
	//{{AFX_MSG_MAP(CMetaplugDefaultsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetaplugDefaultsDlg message handlers
