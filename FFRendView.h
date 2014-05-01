// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		29apr11	add CancelDrag
		02		01may11	add load balance
        03      07may11	add sync oscillators
        04      18nov11	convert load balance from dialog to bar
		05		24nov11	pass previous slot to OnSelChange
		06		26mar12	add monitor source
		07		06may12	make SyncOscillators public
		08		23may12	make monitor source a slot index

        FFRend view
 
*/

// FFRendView.h : interface of the CFFRendView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FFRENDVIEW_H__C0B5DB06_28B0_44F8_8FB9_30C0B8B25941__INCLUDED_)
#define AFX_FFRENDVIEW_H__C0B5DB06_28B0_44F8_8FB9_30C0B8B25941__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RowView.h"
#include "DragTabCtrl.h"
#include "afxadv.h"	// for CRecentFileList
#include "Clipboard.h"
#include "Undoable.h"

class CMainFrame;
class CMainEngine;
class CFFPlugsRow;
class CFFRowView;

class CFFRendView : public CView, public CUndoable
{
protected: // create from serialization only
	CFFRendView();
	DECLARE_DYNCREATE(CFFRendView)

// Constants
	enum {	// reserved command ID ranges
		ID_MONITOR_SOURCE_FIRST = 0xc000,	// first monitor source command ID
		ID_MONITOR_SOURCE_LAST = 0xcfff,	// maximum monitor source command ID
		ID_PLUGIN_INPUT_FIRST = 0xd000,		// first plugin input command ID
		ID_PLUGIN_INPUT_LAST = 0xdfff,		// maximum plugin input command ID
	};

// Attributes
public:
	CFFRendDoc	*GetDocument();
	int		GetCurSel() const;
	int		GetInsPos() const;
	bool	HaveCurSel() const;
	bool	IsLoaded(int SlotIdx) const;
	bool	IsCurSelLoaded() const;
	void	SetRowInfo(int RowIdx, CFFPlugsRow& Row);
	CFFPlugsRow	*GetRow(int RowIdx);
	const CFFPlugsRow	*GetRow(int RowIdx) const;
	int		GetRows() const;
	int		GetSelectedRow() const;
	bool	IsValidRow(int RowIdx) const;
	CFFPluginEx	*GetCurPlugin();
	bool	CanPaste() const;
	void	SetPrevSel(int SlotIdx);
	void	SetParmVal(int SlotIdx, int ParmIdx, float Val);
	void	SetModEnable(int SlotIdx, int ParmIdx, bool Enable);
	void	SetModWave(int SlotIdx, int ParmIdx, int Wave);
	void	SetModFreq(int SlotIdx, int ParmIdx, float Freq);
	void	SetModPulseWidth(int SlotIdx, int ParmIdx, float PulseWidth);
	void	SetModRange(int SlotIdx, int ParmIdx, CFFPlugInfo::FRANGE Range);
	void	SetMonitorSource(int SlotIdx);

// Operations
public:
	bool	Insert(int SlotIdx, LPCTSTR Path);
	bool	InsertEmpty(int SlotIdx);
	bool	Delete(int SlotIdx);
	bool	Load(int SlotIdx, LPCTSTR Path);
	bool	LoadRecent(int MruIdx);
	bool	Unload(int SlotIdx);
	void	Bypass(int SlotIdx, bool Enable);
	void	Solo(int SlotIdx);
	void	UpdateRows();
	void	UpdateView();
	void	ShowPluginContextMenu(CPoint point, int TargetSlot);
	int		FindSlot(CPoint pt) const;
	void	AddMetaparm(int PageType, int PropIdx);
	void	Copy(int SlotIdx);
	bool	Cut(int SlotIdx);
	bool	Paste(int SlotIdx);
	bool	Move(int SrcSlotIdx, int DstSlotIdx);
	bool	Connect(int SrcPlugIdx, int DstSlotIdx, int DstInpIdx);
	bool	MakePluginPopup(CMenu& Popup, int BaseID, int CheckIdx) const;
	int		InsertInputPopups(CMenu *Menu, int Pos, int SlotIdx) const;
	void	CancelDrag();
	bool	SyncOscillators(bool Undoable);

// Handlers
	void	OnBypass(int SlotIdx, bool Enable);
	void	OnSelChange(int PrevSlotIdx, int SlotIdx);
	void	OnMonitorSourceChange(int SlotIdx);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFRendView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFFRendView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFFRendView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditDelete();
	afx_msg void OnEditInsert();
	afx_msg void OnEditInsertEmpty();
	afx_msg void OnEditModRangeEdit();
	afx_msg void OnEditModRangeGotoEnd();
	afx_msg void OnEditModRangeGotoStart();
	afx_msg void OnEditModRangeRemove();
	afx_msg void OnEditModRangeSetEnd();
	afx_msg void OnEditModRangeSetStart();
	afx_msg void OnEditPaste();
	afx_msg void OnEditSyncOscillators();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnMetaparmAddAllParams();
	afx_msg void OnMetaparmAddBypass();
	afx_msg void OnMetaparmAddModEnab();
	afx_msg void OnMetaparmAddModFreq();
	afx_msg void OnMetaparmAddModPW();
	afx_msg void OnMetaparmAddModWave();
	afx_msg void OnMetaparmAddParam();
	afx_msg void OnPluginBypass();
	afx_msg void OnPluginLoad();
	afx_msg void OnPluginMonitor();
	afx_msg void OnPluginProperties();
	afx_msg void OnPluginSolo();
	afx_msg void OnPluginUnload();
	afx_msg void OnSelchangeTabCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditModRangeRemove(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditModRangeSet(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSyncOscillators(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePluginBypass(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePluginLoad(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePluginMonitor(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePluginMru(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePluginSolo(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePluginUnload(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnPluginMruFile(UINT nID);
	afx_msg LRESULT OnTabCtrlDrag(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFFRowEdit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPluginInput(UINT nID);
	afx_msg void OnMonitorSource(UINT nID);
	DECLARE_MESSAGE_MAP()

// Types
	typedef CFFPlugInfo::FFPLUG_INFO FFPLUG_INFO;
	typedef CFFPlugInfo::FFPARM_INFO FFPARM_INFO;
	typedef CFFProject::CRouting CRouting;
	class CPluginUndoInfo : public CRefObj {
	public:
		CFFPlugInfo	m_PlugInfo;		// plugin info
		CRouting	m_Routing;		// routing
		CMetaparmArray	m_Metaparm;	// metaparameters
		int		m_MonitorSource;	// monitor source
	};
	class CClipUndoInfo : public CRefObj {
	public:
		CString	m_ClipPath;			// clip path
	};
	class CMidiSetupUndoInfo : public CRefObj {
	public:
		CFFProject::CMidiAssignList	m_MidiAssign;	// MIDI assignments
	};
	typedef struct tagOSC_PHASE {
		int		SlotIdx;	// slot index
		int		ParmIdx;	// parameter index
		float	Phase;		// oscillator phase
	} OSC_PHASE;
	typedef CArrayEx<OSC_PHASE, OSC_PHASE&> COscPhaseArray;
	class CUndoSyncOscillatorsInfo :  public CRefObj {
	public:
		COscPhaseArray	m_OscPhase;	// oscillator phases
	};

// Constants
	enum {	// row dialog columns
		COL_NAME,
		COL_SLIDER,
		COL_VALUE,
		COL_MOD_ENAB,
		COL_MOD_WAVE,
		COL_MOD_FREQ,
		COL_MOD_PW,
		COLUMNS,
	};
	static const CRowView::COLINFO m_ColInfo[COLUMNS];
	enum {
		TAB_CTRL_HEIGHT = 28,		// height of tab control
		TAB_CTRL_OVERLAP = 2,		// overlap of tab control onto row view header
		TAB_CTRL_TEXT_MAX = 64,		// maximum length of tab text, in characters
		CTX_NO_TARGET = INT_MAX,	// reserved value for no context menu target
		MAX_RECENT_PLUGINS = 4,		// maximum number of recently used plugins
	};
	static const int m_UndoTitleID[];	// undo title string resource IDs

// Member data
	CMainFrame	*m_Main;		// pointer to main frame
	CMainEngine	*m_Engine;		// pointer to engine
	CFFRowView	*m_View;		// pointer to row view
	CDragTabCtrl	m_TabCtrl;	// tab control
	CImageList	m_TabCtrlImgList;	// tab control image list
	CImageList	m_HdrImgList;	// row view header image list
	bool	m_InContextMenu;	// true if displaying context menu
	int		m_ContextTarget;	// context menu target slot, or CTX_NO_TARGET
	CRecentFileList	m_RecentPlugin;	// recently used plugins
	CClipboard	m_Clipboard;	// clipboard instance
	int		m_PrevSel;			// previous selection; for undo
	int		m_MoveSrcSlotIdx;	// move source slot index; for undo
	int		m_MoveDstSlotIdx;	// move destination slot index; for undo
	int		m_ConnDstInpIdx;	// connect destination input index; for undo
	int		m_MonSrcSlotIdx;	// slot index of monitor source, or -1 if none

// Overrides
	void	SaveUndoState(CUndoState& State);
	void	RestoreUndoState(const CUndoState& State);
	CString	GetUndoTitle(const CUndoState& State);

// Helpers
	void	GetInfo(int SlotIdx, CUndoState& State) const;
	void	SetInfo(int SlotIdx, const CUndoState& State);
	static	short	USLOT(int SlotIdx);
	void	NotifyUndoableEdit(int SlotIdx, WORD Code, UINT Flags = 0);
	void	CancelUndoableEdit(int SlotIdx, WORD Code);
	void	SetParmVal(int ParmIdx, float Val);
	void	SetModEnable(int ParmIdx, bool Enable);
	void	SetModWave(int ParmIdx, int Wave);
	void	SetModFreq(int ParmIdx, float Freq);
	void	SetModPulseWidth(int ParmIdx, float PulseWidth);
	void	SetModRange(int ParmIdx, CFFPlugInfo::FRANGE Range);
	bool	GetOscillatorPhases(COscPhaseArray& OscPhase);
	bool	SetOscillatorPhases(const COscPhaseArray& OscPhase);

// Aliases for undo value members
	CUNDOSTATE_VAL(	UVCurSel,		int,	p.x.i)
	CUNDOSTATE_VAL(	UVParmVal,		float,	p.y.f)
	CUNDOSTATE_VAL(	UVModEnable,	bool,	p.x.b)
	CUNDOSTATE_VAL(	UVModWave,		int,	p.x.i)
	CUNDOSTATE_VAL(	UVModFreq,		float,	p.x.f)
	CUNDOSTATE_VAL(	UVModPW,		float,	p.x.f)
	CUNDOSTATE_VAL(	UVModStart,		float,	p.x.f)
	CUNDOSTATE_VAL(	UVModEnd,		float,	p.y.f)
	CUNDOSTATE_VAL(	UVBypass,		bool,	p.x.b)
	CUNDOSTATE_VAL(	UVSolo,			bool,	p.x.b)
	CUNDOSTATE_VAL(	UVMoveSrc,		int,	p.x.i)
	CUNDOSTATE_VAL(	UVMoveDst,		int,	p.y.i)
	CUNDOSTATE_VAL(	UVConnSrc,		short,	p.x.s.hi)
	CUNDOSTATE_VAL(	UVConnInp,		short,	p.y.s.lo)
	CUNDOSTATE_VAL(	UVConnDst,		short,	p.y.s.hi)
	CUNDOSTATE_VAL(	UVInsert,		WORD,	p.y.w.lo)
	CUNDOSTATE_VAL(	UVSpeed,		float,	p.x.f)
	CUNDOSTATE_VAL(	UVThreadCount,	int,	p.x.i)
	CUNDOSTATE_VAL(	UVMonitorSrc,	int,	p.x.i)
};

class CFFRowView : public CRowView {
public:
	DECLARE_DYNCREATE(CFFRowView);
	CRowDlg	*CreateRow(int Idx);
	void	UpdateRow(int Idx);
	CFFRendView	*m_Parent;
};

inline int CFFRendView::GetCurSel() const
{
	return(m_InContextMenu ? m_ContextTarget : m_Engine->GetCurSel());
}

inline bool CFFRendView::HaveCurSel() const
{
	return(GetCurSel() >= 0);
}

inline bool CFFRendView::IsLoaded(int SlotIdx) const
{
	return(m_Engine->IsLoaded(SlotIdx));
}

inline bool CFFRendView::IsCurSelLoaded() const
{
	return(HaveCurSel() && IsLoaded(GetCurSel()));
}

inline int CFFRendView::GetInsPos() const
{
	return(HaveCurSel() ? GetCurSel() : m_Engine->GetSlotCount());
}

inline CFFPluginEx *CFFRendView::GetCurPlugin()
{
	return(IsCurSelLoaded() ? m_Engine->GetSlot(GetCurSel()) : NULL);
}

inline CFFPlugsRow *CFFRendView::GetRow(int RowIdx)
{
	return((CFFPlugsRow *)m_View->GetRow(RowIdx));
}

inline const CFFPlugsRow *CFFRendView::GetRow(int RowIdx) const
{
	return((CFFPlugsRow *)m_View->GetRow(RowIdx));
}

inline int CFFRendView::GetRows() const
{
	return(m_View->GetRows());
}

inline bool CFFRendView::IsValidRow(int RowIdx) const
{
	return(RowIdx >= 0 && RowIdx < GetRows());
}

inline bool CFFRendView::CanPaste() const
{
	return(m_Clipboard.HasData());
}

inline void CFFRendView::SetPrevSel(int SlotIdx)
{
	m_PrevSel = SlotIdx;
}

inline short CFFRendView::USLOT(int SlotIdx)
{
	ASSERT(SlotIdx >= SHRT_MIN && SlotIdx <= SHRT_MAX);
	return(static_cast<short>(SlotIdx));	// limited to 32K slots
}

inline void CFFRendView::NotifyUndoableEdit(int SlotIdx, WORD Code, UINT Flags)
{
	CUndoable::NotifyUndoableEdit(USLOT(SlotIdx), Code, Flags);
}

inline void CFFRendView::CancelUndoableEdit(int SlotIdx, WORD Code)
{
	CUndoable::CancelUndoableEdit(USLOT(SlotIdx), Code);
}

inline void CFFRendView::CancelDrag()
{
	m_TabCtrl.CancelDrag();
}

#ifndef _DEBUG  // debug version in FFRendView.cpp
inline CFFRendDoc* CFFRendView::GetDocument()
   { return (CFFRendDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFRENDVIEW_H__C0B5DB06_28B0_44F8_8FB9_30C0B8B25941__INCLUDED_)
