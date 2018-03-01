// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23dec06	initial version
		01		26dec06	visually group inputs by plugin
		02		26dec06	show empty plugin slots
		03		28dec06	add drag threshold
		04		29dec06	allow insert at end of list
		05		03jan07	add bypass checkbox
		06		10jan07	allow context menu to display status bar hints
		07		13jan07	add OnRoutingChange
		08		19jan07	add FastIsVisible
		09		21jan07	replace AfxGetMainWnd with GetThis
		10		31jan07	move FastIsVisible into base class
		11		07mar07	add FindSlot
		12		11jul07	in UpdateView, remove set focus
		13		23nov07	support Unicode
		14		01dec07	add monitor update handler
		15		29jan08	in CreateRow, remove unused local var
		16		29jan08	add static casts to fix warnings
		17		21apr10	port to refactored RowView
		18		29aug10	remove dirty view flag
		19		11nov11	fix keyboard-triggered context menu
		20		06apr12	add column for enable checkbox

        patch bar
 
*/

// PatchBar.cpp : implementation file
//

#include "stdafx.h"
#include "FFRend.h"
#include "PatchBar.h"
#include "PatchRow.h"
#include "FFPluginEx.h"
#include "FFRendDoc.h"
#include "FFRendView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPatchBar

IMPLEMENT_DYNCREATE(CPatchView, CRowView)
IMPLEMENT_DYNAMIC(CPatchBar, CMySizingControlBar);

const CRowView::COLINFO CPatchBar::m_ColInfo[COLUMNS] = {
	{IDC_PB_ENABLE,		0},
	{IDC_PB_INPUT,		IDS_PB_INPUT},
	{IDC_PB_SOURCE,		IDS_PB_SOURCE},
};

CPatchBar::CPatchBar()
{
	m_Engine = NULL;
	m_View = NULL;
}

CPatchBar::~CPatchBar()
{
}

void CPatchBar::UpdateView()
{
	if (FastIsVisible()) {
		int	slots = m_Engine->GetSlotCount();
		int	rows = 0;
		m_SlotToRow.SetSize(slots);
		int	SlotIdx;
		for (SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
			m_SlotToRow[SlotIdx] = rows;
			CFFPluginEx	*slot = m_Engine->GetSlot(SlotIdx);
			int	inputs = slot != NULL ? slot->GetNumInputs() : 1;
			rows += inputs;
		}
		m_Input.SetSize(rows);
		int	RowIdx = 0;
		for (SlotIdx = 0; SlotIdx < slots; SlotIdx++) {
			CFFPluginEx	*slot = m_Engine->GetSlot(SlotIdx);
			int	inputs = slot != NULL ? slot->GetNumInputs() : 1;
			for (int InpIdx = 0; InpIdx < inputs; InpIdx++) {
				INPUT_INFO&	info = m_Input[RowIdx];
				info.SlotIdx = SlotIdx;
				info.NumInputs = inputs;
				info.InpIdx = InpIdx;
				RowIdx++;
			}
		}
		m_View->CreateRows(rows);
	}
}

void CPatchBar::SetRowInfo(int RowIdx, CPatchRow& Row)
{
	if (m_Engine->GetSlotCount()) {
		INPUT_INFO&	info = m_Input[RowIdx];
		CFFPluginEx	*slot = m_Engine->GetSlot(info.SlotIdx);
		CString	name;
		Row.m_Source.ResetContent();
		int	ShowEnable;
		bool	IsEnabled, TakesInput;
		if (slot != NULL) {
			name = slot->GetName();
			if (info.NumInputs > 1) {
				TCHAR	suffix[3] = {' ', TCHAR('A' + info.InpIdx)};
				name += suffix;
			}
			name += ':';
			int	plugs = m_Engine->GetPluginCount();
			TakesInput = !slot->IsSource();
			int	SelSrc;
			if (TakesInput) {
				Row.m_Source.AddString(LDS(IDS_FF_INPUT_DEFAULT));
				for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++)
					Row.m_Source.AddString(m_Engine->GetPlugin(PlugIdx).GetName());
				SelSrc = slot->GetInputPlugin(info.InpIdx) + 1;
			} else {
				Row.m_Source.AddString(LDS(IDS_FF_INPUT_NONE));
				SelSrc = 0;
			}
			Row.m_Source.SetCurSel(SelSrc);
			ShowEnable = !info.InpIdx;	// for mixers, show checkbox on first row only
			IsEnabled = !slot->GetBypass();
		} else {	// empty slot
			ShowEnable = TRUE;
			IsEnabled = FALSE;
			TakesInput = FALSE;
		}
		Row.m_Enable.ShowWindow(ShowEnable ? SW_SHOW : SW_HIDE);
		Row.m_Enable.SetCheck(IsEnabled);
		Row.m_Enable.EnableWindow(slot != NULL);
		Row.m_Source.EnableWindow(TakesInput);	// disable drop list for source plugins
		Row.m_Input.SetWindowText(name);
		Row.m_SlotIdx = info.SlotIdx;
		Row.m_NumInputs = info.NumInputs;
		Row.m_InpIdx = info.InpIdx;
	}
}

int CPatchBar::FindSlot(CPoint pt) const
{
	ScreenToClient(&pt);	// assume pt is in screen coords
	int	RowIdx = m_View->FindRow(pt);
	if (RowIdx >= 0)
		return(m_Input[RowIdx].SlotIdx);
	return(-1);
}

void CPatchBar::OnBypass(int SlotIdx, bool Enable)
{
	if (FastIsVisible())
		GetRow(m_SlotToRow[SlotIdx])->m_Enable.SetCheck(!Enable);
}

void CPatchBar::OnSelChange(int SlotIdx)
{
}

void CPatchBar::MoveRow(int SrcRow, int DstRow)
{
	theApp.GetView()->Move(m_Input[SrcRow].SlotIdx, m_Input[DstRow].SlotIdx);
}

CRowDlg *CPatchView::CreateRow(int Idx)
{
	CPatchRow	*rp = new CPatchRow;
	rp->Create(IDD_PATCH_ROW);
	m_Parent->SetRowInfo(Idx, *rp);
	return(rp);
}

void CPatchView::UpdateRow(int Idx)
{
	CPatchRow	*rp = (CPatchRow *)GetRow(Idx);
	m_Parent->SetRowInfo(Idx, *rp);
	rp->Invalidate();
}

void CPatchView::OnDrop(int SrcRow, int DstRow)
{
	m_Parent->MoveRow(SrcRow, DstRow);
}

/////////////////////////////////////////////////////////////////////////////
// CPatchBar message map

BEGIN_MESSAGE_MAP(CPatchBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CPatchBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatchBar message handlers

int CPatchBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_Engine = &theApp.GetEngine();
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CPatchView);
	m_View = DYNAMIC_DOWNCAST(CPatchView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_PATCH_VIEW, NULL))
		return -1;
	m_View->m_Parent = this;
	m_View->SetNotifyWnd(this);
	m_View->SetAccel(NULL, theApp.GetMain());
	m_View->CreateCols(COLUMNS, m_ColInfo, IDD_PATCH_ROW);
		
	return 0;
}

void CPatchBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_View->MoveWindow(0, 0, cx, cy);
}

void CPatchBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		UpdateView();
}

void CPatchBar::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	m_View->FixContextMenuPos(point);
	CPoint	pt = point;
	ScreenToClient(&pt);
	int	DstRow = m_View->FindRow(pt);
	int	SlotIdx = DstRow >= 0 ? m_Input[DstRow].SlotIdx : -1;
	theApp.GetView()->ShowPluginContextMenu(point, SlotIdx);
}
