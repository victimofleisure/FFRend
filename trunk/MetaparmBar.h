// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01feb07	initial version
        01      23dec07	make GetRow public
		02		25dec07	add metaparameter groups
		03		09jan08	decorate group name to distinguish slaves
		04		01may10	port to refactored RowView
		05		29aug10	remove dirty view flag
		06		29apr11	add CancelDrag

		metaplugin parameter control bar
 
*/

#if !defined(AFX_METAPARMBAR_H__E6D980F6_93E8_4FB2_8334_291B04D7C9D1__INCLUDED_)
#define AFX_METAPARMBAR_H__E6D980F6_93E8_4FB2_8334_291B04D7C9D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MetaparmBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMetaparmBar dialog

#include "ArrayEx.h"
#include "DragRowView.h"
#include "MySizingControlBar.h"
#include "Metaplugin.h"
#include "MetaparmGroup.h"
#include "SmartPtr.h"
#include "Undoable.h"

class CMainFrame;
class CMidiEngine;
class CMetaparmRow;
class CMetaparmView;

class CMetaparmBar : public CMySizingControlBar, public CUndoable
{
	DECLARE_DYNAMIC(CMetaparmBar);
public:
// Construction
	CMetaparmBar(CMetaparmArray& Metaparm, CWnd* pParent = NULL);

// Types
	typedef CMetaparm::TARGET TARGET;

// Attributes
	const CMetaparm&	GetMetaparm(int ParmIdx) const;
	void	SetMetaparm(int ParmIdx, const CMetaparm& Parm);
	void	SetValue(int ParmIdx, float Value);
	CMetaparmRow	*GetRow(int ParmIdx) const;
	int		GetRows() const;
	int		GetCurRow() const;
	void	SetRowInfo(int RowIdx, CMetaparmRow& Row);

// Operations
	void	UpdateView();
	void	Add(const TARGET& Target);
	void	Insert(int ParmIdx);
	void	Insert(int ParmIdx, CMetaparm& Parm);
	void	Delete(int ParmIdx);
	void	Move(int SrcRow, int DstRow);
	void	Reset(int ParmIdx);
	void	EditProps(int ParmIdx);
	void	OnRowValChange(int ParmIdx, float Value);
	bool	EditGroup(int ParmIdx);
	void	Ungroup(int ParmIdx);
	void	OnDrop(int SrcRow, int DstRow);
	void	CancelDrag();

// Constants

// Overrides
	void	SaveUndoState(CUndoState& State);
	void	RestoreUndoState(const CUndoState& State);
	CString	GetUndoTitle(const CUndoState& State);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMetaparmBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// Dialog Data
	//{{AFX_DATA(CMetaparmBar)
	//}}AFX_DATA

// Generated message map functions
	//{{AFX_MSG(CMetaparmBar)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnInsert();
	afx_msg void OnDelete();
	afx_msg void OnProperties();
	afx_msg void OnGroup();
	afx_msg void OnUngroup();
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGroup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUngroup(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef CArrayEx<float, float&> CParmValArray;
	class CUndoInfo : public CRefObj {
	public:
		CMetaparm	m_Parm;			// metaparameter
		CSmartPtr<CMetaparmGroup>	m_Group;	// optional metaparameter group
		CSmartPtr<CParmValArray>	m_ParmVal;	// optional parameter values
	};

// Constants
	enum {	// columns
		COL_NAME,
		COL_SLIDER,
		COL_VALUE,
		COL_GROUP,
		COLUMNS
	};
	static const CDragRowView::COLINFO m_ColInfo[COLUMNS];

// Member data
	CMetaparmView	*m_View;	// point to row view
	CMainFrame	*m_Frm;			// pointer to main frame
	CMidiEngine	*m_Engine;		// pointer to engine
	CMetaparmArray&	m_Metaparm;	// reference to array of metaparameters
	int		m_CurRow;			// index of current row, for context menu or undo
	int		m_MoveSrcIdx;		// move source parameter index; for undo

// Helpers
	void	GetInfo(int ParmIdx, CUndoState& State) const;
	void	SetInfo(int ParmIdx, const CUndoState& State);
	void	LinkToPlugin(CMetaparm& Parm);
	void	UpdateMidi();
	void	UpdateSlaves(CMetaparm& Parm);
	CString	GetDecoratedGroupName(int ParmIdx);
	void	UpdateGroupNames();
	void	SetVal(int ParmIdx, float Val);
	void	SetParm(int ParmIdx, const CMetaparm& Parm);
	static	short	UPARM(int ParmIdx);
	void	NotifyUndoableEdit(int SlotIdx, WORD Code, UINT Flags = 0);
	void	CancelUndoableEdit(int SlotIdx, WORD Code);

// Aliases for undo value members
	CUNDOSTATE_VAL(	UValValue,		float,	p.x.f)
	CUNDOSTATE_VAL(	UValMoveSrc,	int,	p.x.i)
	CUNDOSTATE_VAL(	UValMoveDst,	int,	p.y.i)
	CUNDOSTATE_VAL(	UValInsert,		WORD,	p.y.w.lo)
};

class CMetaparmView : public CDragRowView {
public:
	DECLARE_DYNCREATE(CMetaparmView);
	CRowDlg	*CreateRow(int Idx);
	void	UpdateRow(int Idx);
	void	OnDrop(int SrcRow, int DstRow);
	CMetaparmBar	*m_Parent;
};

inline CMetaparmRow *CMetaparmBar::GetRow(int ParmIdx) const
{
	return((CMetaparmRow *)m_View->GetRow(ParmIdx));
}

inline int CMetaparmBar::GetRows() const
{
	return(m_Metaparm.GetSize());
}

inline const CMetaparm& CMetaparmBar::GetMetaparm(int ParmIdx) const
{
	return(m_Metaparm[ParmIdx]);
}

inline int CMetaparmBar::GetCurRow() const
{
	return(m_CurRow);
}

inline short CMetaparmBar::UPARM(int ParmIdx)
{
	ASSERT(ParmIdx >= SHRT_MIN && ParmIdx <= SHRT_MAX);
	return(static_cast<short>(ParmIdx));	// limited to 32K slots
}

inline void CMetaparmBar::NotifyUndoableEdit(int SlotIdx, WORD Code, UINT Flags)
{
	CUndoable::NotifyUndoableEdit(UPARM(SlotIdx), Code, Flags);
}

inline void CMetaparmBar::CancelUndoableEdit(int SlotIdx, WORD Code)
{
	CUndoable::CancelUndoableEdit(UPARM(SlotIdx), Code);
}

inline void CMetaparmBar::CancelDrag()
{
	m_View->CancelDrag();
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_METAPARMBAR_H__E6D980F6_93E8_4FB2_8334_291B04D7C9D1__INCLUDED_)
