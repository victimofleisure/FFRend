// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23dec06	initial version
		01		28dec06	add drag threshold
		02		03jan07	add bypass checkbox
		03		10jan07	allow context menu to display status bar hints
		04		13jan07	add OnRoutingChange
		05		19jan07	add FastIsVisible
		06		31jan07	move FastIsVisible into base class
		07		07mar07	add FindSlot
		08		01dec07	add monitor update handler
		09		29jan08	replace CDWordArray with CIdxArray
		10		21apr10	port to refactored RowView
		11		29aug10	remove dirty view flag
		12		29apr11	add CancelDrag
		13		06apr12	add column for enable checkbox

        patch bar
 
*/

#if !defined(AFX_PATCHBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
#define AFX_PATCHBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatchBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPatchBar window

#include "MySizingControlBar.h"
#include "DragRowView.h"

class CFFEngine;
class CPatchView;
class CPatchRow;

class CPatchBar : public CMySizingControlBar
{
	DECLARE_DYNAMIC(CPatchBar);
// Construction
public:
	CPatchBar();

// Attributes
public:
	CPatchRow	*GetRow(int SlotIdx);
	int		GetRows() const;
	void	SetRowInfo(int RowIdx, CPatchRow& Row);

// Operations
public:
	void	UpdateView();
	void	Run(bool Enable);
	void	TimerHook();
	void	MoveRow(int SrcRow, int DstRow);
	int		FindSlot(CPoint pt) const;
	void	CancelDrag();

// Handlers
	void	OnBypass(int SlotIdx, bool Enable);
	void	OnSelChange(int SlotIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatchBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPatchBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPatchBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Types
	typedef CArrayEx<int, int> CIdxArray;
	typedef struct tagINPUT_INFO {
		int		SlotIdx;	// plugin's slot index
		int		NumInputs;	// number of inputs this plugin has
		int		InpIdx;		// input index, from 0 to NumInputs - 1
	} INPUT_INFO;
	typedef CArrayEx<INPUT_INFO, INPUT_INFO&> CInputArray;
	
// Constants
	enum {	// columns
		COL_ENABLE,
		COL_INPUT,
		COL_SOURCE,
		COLUMNS
	};
	static const CRowView::COLINFO m_ColInfo[COLUMNS];

// Member data
	CFFEngine	*m_Engine;		// pointer to engine
	CPatchView	*m_View;		// pointer to child view
	CIdxArray	m_SlotToRow;	// map slot indices to row indices
	CInputArray	m_Input;		// info about each input
};

class CPatchView : public CDragRowView {
public:
	DECLARE_DYNCREATE(CPatchView);
	CRowDlg	*CreateRow(int Idx);
	void	UpdateRow(int Idx);
	void	OnDrop(int SrcRow, int DstRow);
	CPatchBar	*m_Parent;
};

inline CPatchRow *CPatchBar::GetRow(int SlotIdx)
{
	return((CPatchRow *)m_View->GetRow(SlotIdx));
}

inline int CPatchBar::GetRows() const
{
	return(m_View->GetRows());
}

inline void CPatchBar::Run(bool Enable)
{
	UpdateView();
}

inline void CPatchBar::TimerHook()
{
	UpdateView();
}

inline void CPatchBar::CancelDrag()
{
	m_View->CancelDrag();
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATCHBAR_H__1E61CD05_C1F0_49C5_89EA_653327F7C1C7__INCLUDED_)
