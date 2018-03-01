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

// FFRendDoc.h : interface of the CFFRendDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FFRENDDOC_H__A68650FC_81B2_4C99_9E63_81B5C9E4CF42__INCLUDED_)
#define AFX_FFRENDDOC_H__A68650FC_81B2_4C99_9E63_81B5C9E4CF42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FFProject.h"

class CFFRendDoc : public CDocument, public CFFProject
{
protected: // create from serialization only
	CFFRendDoc();
	DECLARE_DYNCREATE(CFFRendDoc)

// Constants
	enum {	// check link actions
		CLA_DIALOG,		// missing files dialog
		CLA_MSGBOX,		// message box
		CLA_SILENT,		// don't display anything
	};

// Attributes
public:

// Operations
public:
	static	bool	CheckLinks(CFFProject& Project, int Action = CLA_DIALOG);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFRendDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFFRendDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFFRendDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Helpers
	void	SetProject(CFFProject& Project);
};

inline void CFFRendDoc::SetProject(CFFProject& Project)
{
	CFFProject&	proj = *this;	// upcast this to project
	proj = Project;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFRENDDOC_H__A68650FC_81B2_4C99_9E63_81B5C9E4CF42__INCLUDED_)
