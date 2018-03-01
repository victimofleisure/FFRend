// Copyleft 2011 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      01may11	initial version
		01		30may11	remove undo
		02		18nov11	convert from dialog to bar
		03		13jan12	in TimerHook, check for zero delta

		load balance bar
 
*/

// LoadBalanceBar.cpp : implementation file
//

#include "stdafx.h"
#include "ParaPET.h"
#include "LoadBalanceBar.h"
#include "UndoCodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadBalanceBar dialog

IMPLEMENT_DYNCREATE(CLoadBalanceView, CRowView);
IMPLEMENT_DYNAMIC(CLoadBalanceBar, CMySizingControlBar);

const CRowView::COLINFO CLoadBalanceBar::m_ColInfo[COLS] = {
	{IDC_LBR_PLUGIN_NAME,		IDS_LBR_PLUGIN_NAME},
	{IDC_LBR_THREAD_COUNT_EDIT,	IDS_LBR_THREAD_COUNT},
	{IDC_LBR_CPU_PERCENT_BAR,	IDS_LBR_CPU_PERCENT},
	{IDC_LBR_CPU_TIME,			IDS_LBR_CPU_TIME},
};

CLoadBalanceBar::CLoadBalanceBar()
{
	//{{AFX_DATA_INIT(CLoadBalanceBar)
	//}}AFX_DATA_INIT
	m_View = NULL;
	m_Engine = NULL;
	m_InitRect.SetRectEmpty();
	m_PrevTicks = 0;
}

inline static UINT UDiv64(ULONGLONG Dividend, UINT Divisor, UINT& Remainder)
{
	UINT	Quotient;
	__asm {
		mov		eax, dword ptr Dividend			// low dword
		mov		edx, dword ptr Dividend + 4		// high dword
		mov		ecx, Divisor
		div		ecx
		mov		Quotient, eax
		mov		Remainder, edx
	};
	return(Quotient);
}

void CLoadBalanceBar::TimerHook()
{
	int	plugs = m_Engine->GetPluginCount();
	int	threads = m_Engine->GetThreadCount();
	m_RunTime.SetSize(threads);
	DWORD	*pRunTime = m_RunTime.GetData();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CPlugin&	plug = m_Engine->GetPlugin(PlugIdx);
		int	PlugThreads = plug.GetThreadCount();
		for (int ThrIdx = 0; ThrIdx < PlugThreads; ThrIdx++) {
			CEngineThread	*pThr;
			if (PlugThreads > 1)
				pThr = &plug.GetHelper(ThrIdx);
			else
				pThr = &plug;
			ULONGLONG	delta;
			if (!pThr->GetRunDelta(delta))
				break;
			UINT	rem, t;
			t = UDiv64(delta, 10000, rem);	// convert to milliseconds
			*pRunTime++ = t;
		}
	}
	DWORD	ticks = GetTickCount();
	if (m_PrevTicks && ticks >= m_PrevTicks) {
		int		delta = ticks - m_PrevTicks;
		if (delta) {	// avoid divide by zero
			for (int RowIdx = 0; RowIdx < threads; RowIdx++) {
				int	t = m_RunTime[RowIdx];
				CLoadBalanceRow	*rp = GetRow(RowIdx);
				rp->SetCPUPercent(t * 100 / delta);
				rp->SetCPUTime(t);
			}
		}
	}
	m_PrevTicks = ticks;
}

void CLoadBalanceBar::PopulateView()
{
	int	threads = m_Engine->GetThreadCount();
	m_View->CreateRows(threads);	// can take a while if CPU overloaded
	m_ThreadPlugin.SetSize(threads);
	int	plugs = m_Engine->GetPluginCount();
	int	RowIdx = 0;
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		CPlugin&	plug = m_Engine->GetPlugin(PlugIdx);
		int	PlugThreads = plug.GetThreadCount();
		for (int ThrIdx = 0; ThrIdx < PlugThreads; ThrIdx++) {
			m_ThreadPlugin[RowIdx] = PlugIdx;
			CString	name(plug.GetName());
			if (PlugThreads > 1) {
				CString	s;
				s.Format(_T("[%d]"), ThrIdx);
				name += s;
			}
			CLoadBalanceRow	*rp = GetRow(RowIdx);
			rp->SetPluginName(name + ':');
			rp->EnableThreadCountEdit(!ThrIdx);
			if (!ThrIdx)	// if plugin's first thread
				rp->SetThreadCount(PlugThreads);
			else
				rp->BlankThreadCount();
			RowIdx++;
		}
	}
}

void CLoadBalanceBar::OnChangedThreadCount(int RowIdx)
{
	int	PlugIdx = m_ThreadPlugin[RowIdx];
	CPlugin&	plug = m_Engine->GetPlugin(PlugIdx);
	theApp.GetMain()->NotifyEdit(WORD(plug.GetSlotIdx()), UCODE_LOAD_BALANCE);
	CLoadBalanceRow	*rp = GetRow(RowIdx);
	plug.SetThreadCount(rp->GetThreadCount());
	PopulateView();
}

CRowDlg *CLoadBalanceView::CreateRow(int Idx)
{
	CLoadBalanceRow	*rp = new CLoadBalanceRow;
	rp->Create(IDD_LOAD_BALANCE_ROW);
	return(rp);
}

BEGIN_MESSAGE_MAP(CLoadBalanceBar, CMySizingControlBar)
	//{{AFX_MSG_MAP(CLoadBalanceBar)
	ON_WM_CREATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadBalanceBar message handlers

int CLoadBalanceBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMySizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CLoadBalanceView);
	m_View = DYNAMIC_DOWNCAST(CLoadBalanceView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
    CRect r(0, 0, 0, 0);	// arbitrary initial size
    if (!m_View->Create(NULL, NULL, dwStyle, r, this, IDC_LOAD_BALANCE_VIEW, NULL))
		return -1;
		
	m_View->m_Parent = this;
	m_View->SetNotifyWnd(this);
	m_View->CreateCols(COLS, m_ColInfo, IDD_LOAD_BALANCE_ROW);
	m_View->SetAccel(NULL, theApp.GetMain());
	m_Engine = &theApp.GetEngine();
	PopulateView();
	TimerHook();

	return 0;
}

void CLoadBalanceBar::OnSize(UINT nType, int cx, int cy) 
{
	CMySizingControlBar::OnSize(nType, cx, cy);
	if (m_IsSizeValid)
		m_View->MoveWindow(0, 0, cx, cy);
}

void CLoadBalanceBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMySizingControlBar::OnWindowPosChanged(lpwndpos);
	if (lpwndpos->flags & SWP_SHOWWINDOW)
		PopulateView();
}
