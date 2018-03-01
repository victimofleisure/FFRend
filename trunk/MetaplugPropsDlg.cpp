// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version
		01		02feb07	add defaults dialog
		02		23nov07	support Unicode
		03		07jun10	update for engine

        metaplugin properties dialog
 
*/

// MetaplugPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "MetaplugPropsDlg.h"
#include "Metaplugin.h"
#include "MainFrm.h"
#include "PathStr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMetaplugPropsDlg dialog

IMPLEMENT_DYNAMIC(CMetaplugPropsDlg, CDialog);

CMetaplugPropsDlg::CMetaplugPropsDlg(CMetaplugin& Metaplug, LPCTSTR FileName, CWnd* pParent /*=NULL*/)
	: CDialog(CMetaplugPropsDlg::IDD, pParent),
	m_Metaplug(Metaplug)
{
	//{{AFX_DATA_INIT(CMetaplugPropsDlg)
	m_About = _T("");
	m_Descrip = _T("");
	m_Type = 0;
	m_VersionMinor = 0;
	m_VersionMajor = 0;
	//}}AFX_DATA_INIT
	m_FileName = FileName;
	m_ModFlag = FALSE;
}

bool CMetaplugPropsDlg::EmbedCheck(CString& NonFreePlugs)
{
	CFFEngine&	eng = theApp.GetEngine();
	// stop engine before accessing FreeFrame instance directly via GetFFPlugin
	STOP_ENGINE(eng);
	int	plugs = eng.GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CFFPluginEx&	pr = eng.GetPlugin(PlugIdx);
		bool	CanEmbed = FALSE;	// assume failure
		PlugExtendedInfoStruct	peis;
		if (pr.GetFFPlugin().GetExtendedInfo(peis)) {
			CString	About(peis.About);
			if (About.Find(_T("Pete Warden")) >= 0)	// case-sensitive
				CanEmbed = TRUE;
			else {
				About.MakeLower();
				if (About.Find(_T("copyleft")) >= 0)	// NOT case-sensitive
					CanEmbed = TRUE;
			}
		}
		if (!CanEmbed) {
			CString	PluginPath(pr.GetPath());
			LPCTSTR	DllName = PathFindFileName(PluginPath);
			if (NonFreePlugs.Find(DllName) < 0)	// eliminate duplicates
				NonFreePlugs += CString("\n") + DllName;
		}
	}
	return(NonFreePlugs.IsEmpty() != 0);
}

void CMetaplugPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMetaplugPropsDlg)
	DDX_Control(pDX, IDC_METAPLUG_EMBED, m_Embed);
	DDX_Control(pDX, IDC_METAPLUG_NAME, m_Name);
	DDX_Control(pDX, IDC_METAPLUG_UNIQUE_ID, m_UniqueID);
	DDX_Text(pDX, IDC_METAPLUG_ABOUT, m_About);
	DDX_Text(pDX, IDC_METAPLUG_DESCRIP, m_Descrip);
	DDX_Radio(pDX, IDC_METAPLUG_TYPE, m_Type);
	DDX_Text(pDX, IDC_METAPLUG_VERSION_MINOR, m_VersionMinor);
	DDV_MinMaxUInt(pDX, m_VersionMinor, 0, 1000);
	DDX_Text(pDX, IDC_METAPLUG_VERSION_MAJOR, m_VersionMajor);
	DDV_MinMaxUInt(pDX, m_VersionMajor, 0, 1000);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMetaplugPropsDlg, CDialog)
	//{{AFX_MSG_MAP(CMetaplugPropsDlg)
	ON_BN_CLICKED(IDC_METAPLUG_EDIT_INPUTS, OnEditInputs)
	ON_BN_CLICKED(IDC_METAPLUG_DEFAULTS, OnDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMetaplugPropsDlg message handlers

BOOL CMetaplugPropsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_Name.SetWindowText(m_Metaplug.m_PluginName);
	m_UniqueID.SetWindowText(m_Metaplug.m_UniqueID);
	m_Descrip		= m_Metaplug.m_Description;
	m_About			= m_Metaplug.m_AboutText;
	m_Type			= m_Metaplug.m_PluginType;
	m_VersionMajor	= m_Metaplug.m_PluginMajorVersion;
	m_VersionMinor	= m_Metaplug.m_PluginMinorVersion;
	m_Embed.SetCheck(m_Metaplug.m_IsEmbedded);
	if (m_Metaplug.m_PluginName.IsEmpty()) {	// if no metaplugin name, set defaults
		// metaplugin name defaults to filename that was passed to ctor
		CPathStr	DefName = m_FileName;
		DefName.RemoveExtension();
		m_Name.SetWindowText(DefName.Left(FF_MAX_PLUGIN_NAME));	// truncate name to fit
		// other defaults come from registry, via defaults dialog
		m_UniqueID.SetWindowText(m_DefaultsDlg.m_UniqueID);
		m_Descrip	= m_DefaultsDlg.m_Descrip;
		m_About		= m_DefaultsDlg.m_About;
		m_ModFlag = TRUE;	// setting defaults counts as a change
	}
	UpdateData(FALSE);	// update controls
	m_Name.SetLimitText(FF_MAX_PLUGIN_NAME);
	m_UniqueID.SetLimitText(FF_MAX_UNIQUE_ID);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMetaplugPropsDlg::OnOK() 
{
	if (!m_Name.LineLength()) {
		AfxMessageBox(IDS_META_NO_PLUGIN_NAME);
		m_Name.SetFocus();
		return;	// don't close dialog
	}
	if (m_Embed.GetCheck()) {
		CString	NonFreePlugs;
		if (!EmbedCheck(NonFreePlugs)) {
			CString	msg((LPCTSTR)IDS_META_CANT_EMBED);
			msg += NonFreePlugs;
			AfxMessageBox(msg);
			m_Embed.SetFocus();
			return;	// don't close dialog
		}
	}
	CDialog::OnOK();	// get data from controls
	// update metaplugin data
	m_Name.GetWindowText(m_Metaplug.m_PluginName);
	m_UniqueID.GetWindowText(m_Metaplug.m_UniqueID);
	m_Metaplug.m_Description	= m_Descrip;
	m_Metaplug.m_AboutText		= m_About;
	m_Metaplug.m_PluginType		= m_Type;
	m_Metaplug.m_PluginMajorVersion	= m_VersionMajor;
	m_Metaplug.m_PluginMinorVersion	= m_VersionMinor;
	m_Metaplug.m_IsEmbedded		= m_Embed.GetCheck() != 0;
}

void CMetaplugPropsDlg::OnEditInputs() 
{
	AfxMessageBox(_T("Multi-input metaplugins aren't implemented yet."));
}

void CMetaplugPropsDlg::OnDefaults() 
{
	if (m_DefaultsDlg.DoModal() == IDOK) {
		m_UniqueID.SetWindowText(m_DefaultsDlg.m_UniqueID);
		m_Descrip	= m_DefaultsDlg.m_Descrip;
		m_About		= m_DefaultsDlg.m_About;
		UpdateData(FALSE);	// update dialog controls
		m_ModFlag = TRUE;
	}
}

BOOL CMetaplugPropsDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (IsWindowVisible()) {	// ignore change notifications during init
		WORD	cmd = HIWORD(wParam);
		WORD	id = LOWORD(wParam);
		// try to detect changes to dialog data by snooping control notifications;
		// controls that send change notifications other than those handled below
		// won't trip the modified flag
		switch (cmd) {
		case BN_CLICKED:	// buttons, radios, checkboxes
			switch (id) {
			case IDOK:	// pressing OK doesn't count as a change
			case IDC_METAPLUG_EDIT_INPUTS:	// handles modify itself
			case IDC_METAPLUG_DEFAULTS:		// handles modify itself
				break;
			default:
				m_ModFlag = TRUE;
			}
			break;
		case EN_CHANGE:		// edit controls
			m_ModFlag = TRUE;
			break;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
