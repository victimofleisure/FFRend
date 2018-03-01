// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      24jun10	add check link actions
		02		30nov11	override SaveModified to take focus

        FFRend document
 
*/

// FFRendDoc.cpp : implementation of the CFFRendDoc class
//

#include "stdafx.h"
#include "FFRend.h"

#include "FFRendDoc.h"
#include "MissingFilesDlg.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFFRendDoc

IMPLEMENT_DYNCREATE(CFFRendDoc, CDocument)

BEGIN_MESSAGE_MAP(CFFRendDoc, CDocument)
	//{{AFX_MSG_MAP(CFFRendDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFRendDoc construction/destruction

CFFRendDoc::CFFRendDoc()
{
}

CFFRendDoc::~CFFRendDoc()
{
}

BOOL CFFRendDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	CFFProject	Project;	// empty project
	theApp.GetMain()->SetProject(Project);
	SetProject(Project);
	return TRUE;
}

bool CFFRendDoc::CheckLinks(CFFProject& Project, int Action)
{
	CStringArray	link;
	int	plugs = Project.m_PlugInfo.GetSize();
	link.SetSize(plugs);
	for (int i = 0; i < plugs; i++) {
		const CFFPlugInfo&	plug = Project.m_PlugInfo[i];
		link[i] = plug.m_Path;	// copy plugin path to search list
		if (!plug.m_ClipPath.IsEmpty())	// if plugin has a clip path
			link.Add(plug.m_ClipPath);	// append it to link list
	}
	if (!Project.m_VideoPath.IsEmpty())	// if project has a video path
		link.Add(Project.m_VideoPath);	// check it too
	if (Action == CLA_DIALOG) {	// if action is missing files dialog
		CString	Filter((LPCTSTR)IDS_PLUGIN_FILTER);
		CMissingFilesDlg	mfd(link, PLUGIN_EXT, Filter);
		if (mfd.Check() == IDCANCEL)
			return(FALSE);
		int	ClipIdx = plugs;
		for (int i = 0; i < plugs; i++) {
			CFFPlugInfo&	plug = Project.m_PlugInfo[i];
			plug.m_Path = link[i];	// update plugin path from link list
			if (!plug.m_ClipPath.IsEmpty())	// if plugin has a clip path
				plug.m_ClipPath = link[ClipIdx++];	// update it from link list
		}
		if (!Project.m_VideoPath.IsEmpty())	// if project has a video path
			Project.m_VideoPath = link[link.GetSize() - 1];	// update it too
	} else {	// action is message box or silent
		CString	msg;
		int	links = link.GetSize();
		for (int i = 0; i < links; i++) {
			if (link[i].GetLength() && !PathFileExists(link[i])) {
				if (msg.IsEmpty())
					msg = _T("Missing files:");
				msg += '\n' + link[i];
			}
		}
		if (!msg.IsEmpty()) {
			if (Action == CLA_MSGBOX)
				AfxMessageBox(msg);
			return(FALSE);
		}
	}
	return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CFFRendDoc serialization

void CFFRendDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFFRendDoc diagnostics

#ifdef _DEBUG
void CFFRendDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFFRendDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFFRendDoc commands

BOOL CFFRendDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	CFFProject	Project;	// empty project to avoid partial init
	if (!Project.Read(lpszPathName)) {
		CString	s;
		AfxFormatString1(s, IDS_DOC_BAD_FORMAT, lpszPathName);
		AfxMessageBox(s);
		return(FALSE);
	}
	if (!CheckLinks(Project))
		return(FALSE);
	theApp.GetMain()->SetProject(Project);
	SetProject(Project);
	return(TRUE);
}

BOOL CFFRendDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnSaveDocument(lpszPathName))
		return FALSE;
	CFFProject	Project;	// empty project to avoid partial init
	theApp.GetMain()->GetProject(Project);
	if (!Project.Write(lpszPathName))
		return(FALSE);
	SetProject(Project);
	return(TRUE);
}

BOOL CFFRendDoc::SaveModified() 
{
	// take focus; if numeric edit is in progress, edit control receives
	// kill focus message, saves change, and sends a change notification
	theApp.GetMain()->SetFocus();
	return CDocument::SaveModified();
}
