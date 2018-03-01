// Copyleft 2009 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		21feb09	initial version
		01		06jan10	W64: in OnInitDialog, cast line array size to 32-bit

		single-threaded modeless message box

*/

// MsgBoxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "MsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxDlg dialog

IMPLEMENT_DYNAMIC(CMsgBoxDlg, CDialog);

UINT CMsgBoxDlg::m_InstCount;

CMsgBoxDlg::CMsgBoxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgBoxDlg)
	//}}AFX_DATA_INIT
	m_Type = 0;
	m_LineHeight = 0;
	m_AutoDelete = FALSE;
}

bool CMsgBoxDlg::CreateMsg(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	m_Text = lpText;
	m_Caption = lpCaption;
	m_Type = uType;
	if (!Create(IDD))
		return(FALSE);
	ShowWindow(SW_SHOW);
	return(TRUE);
}

void CMsgBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgBoxDlg)
	DDX_Control(pDX, IDOK, m_OkBtn);
	DDX_Control(pDX, IDC_MSG_BOX_ICON, m_Icon);
	DDX_Control(pDX, IDC_MSG_BOX_TEXT, m_TextCtrl);
	//}}AFX_DATA_MAP
}

bool CMsgBoxDlg::Do(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	CMsgBoxDlg	*pDlg = new CMsgBoxDlg;	// PostNcDestroy deletes this
	pDlg->m_Text = lpText;
	pDlg->m_Caption = lpCaption;
	pDlg->m_Type = uType;
	pDlg->m_AutoDelete = TRUE;
	if (!pDlg->Create(IDD)) {
		delete pDlg;
		return(FALSE);
	}
	pDlg->ShowWindow(SW_SHOW);
	return(TRUE);
}

CSize CMsgBoxDlg::GetMsgExtent(CWnd& Wnd, LPCTSTR Text, CStringArray *pTextLine)
{
	if (pTextLine != NULL)	// if caller passed line array
		pTextLine->RemoveAll();	// clear it
	CClientDC	dc(&Wnd);
	HGDIOBJ	PrevFont = dc.SelectObject(Wnd.GetFont());	// use window's font
	CString	s(Text);
	CSize	MsgExt = CSize(0, 0);
	int	pos = 0;
	int	TextLen = s.GetLength();
	while (pos < TextLen) {	// while characters remain
		CString	line = s.Mid(pos).SpanExcluding(_T("\n"));	// get next line
		if (pTextLine != NULL)	// if caller passed line array
			pTextLine->Add(line);	// add line to caller's array
		pos += line.GetLength() + 1;	// skip ahead to next line
		if (line.IsEmpty())	// if line is empty
			line = _T(" ");	// keep GetTabbedTextExtent happy
		// get line's extent, taking tabs into account
		CSize	sz = dc.GetTabbedTextExtent(line, 0, NULL);
		if (sz.cx > MsgExt.cx)	// if width exceeds maximum width
			MsgExt.cx = sz.cx;	// update maximum width
		MsgExt.cy += sz.cy;	// add height to total height
	}
	dc.SelectObject(PrevFont);	// restore DC's previous font
	return(MsgExt);	// return message extent
}

HMONITOR CMsgBoxDlg::GetWorkspaceRect(HWND hWnd, CRect& rc)
{
	CRect	wr;
	::GetWindowRect(hWnd, wr);
	// try to get workspace size from monitor API in case we're dual-monitor
	MONITORINFO	mi;
	mi.cbSize = sizeof(mi);	// crucial
	HMONITOR	hMon = MonitorFromRect(wr, MONITOR_DEFAULTTONEAREST);
	if (hMon != NULL && GetMonitorInfo(hMon, &mi)) {
		rc = mi.rcWork;
	} else {	// fall back to older API
		// get workspace size of primary monitor
		if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0)) {
			// get workspace failed; return screen size as a last resort
			rc = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN));
		}
	}
	return(hMon);
}

BEGIN_MESSAGE_MAP(CMsgBoxDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgBoxDlg)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgBoxDlg message handlers

BOOL CMsgBoxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//
	// use message box style to select a standard icon
	//
	LPCTSTR	IconName;
	static const int ICON_MASK = MB_ICONEXCLAMATION | MB_ICONINFORMATION 
		| MB_ICONQUESTION | MB_ICONSTOP;
	switch (m_Type & ICON_MASK) {	// mask off all bits except icon type
	case MB_ICONINFORMATION:
		IconName = IDI_ASTERISK;
		break;
	case MB_ICONQUESTION:
		IconName = IDI_QUESTION;
		break;
	case MB_ICONSTOP:
		IconName = IDI_HAND;
		break;
	default:
		IconName = IDI_EXCLAMATION;
	}
	HICON	hIcon = AfxGetApp()->LoadStandardIcon(IconName);
	if (hIcon != NULL)
		m_Icon.SetIcon(hIcon);	// set our icon
	//
	// set our window caption
	//
	if (m_Caption.IsEmpty())	// if no caption specified
		m_Caption = AfxGetApp()->m_pszAppName;	// default to app name
	SetWindowText(m_Caption);
	//
	// get text extent and resize text control to fit it
	//
	CRect	r;
	m_TextCtrl.GetWindowRect(r);
	ScreenToClient(r);	// convert to client coords
	CSize	OrigTextSize = r.Size();
	CSize	TextSize = GetMsgExtent(m_TextCtrl, m_Text, &m_TextLine);
	m_LineHeight = TextSize.cy / INT64TO32(m_TextLine.GetSize());
	TextSize.cx = max(TextSize.cx, OrigTextSize.cx);	// enforce minimum width
	if (TextSize.cy < OrigTextSize.cy) {	// if less than minimum height
		r.top += (OrigTextSize.cy - TextSize.cy) / 2;	// center text vertically
		TextSize.cy = OrigTextSize.cy;	// keep original height
	}
	r.BottomRight() = r.TopLeft() + TextSize;	// fit to text extent
	m_TextCtrl.MoveWindow(r);	// text control is only a placeholder; see OnPaint
	CSize	TextDelta = TextSize - OrigTextSize;	// save delta in client coords
	//
	// resize our window to fit around text control
	//
	GetWindowRect(r);
	ScreenToClient(r);	// convert to client coords
	CSize	OurOrigSize(r.Size());
	CSize	OurSize = r.Size() + TextDelta;
	r.BottomRight() = r.TopLeft() + OurSize;	// fit to text control
	ClientToScreen(r);	// assume we're top-level; convert back to screen coords
	//
	// center our window on main frame's monitor
	//
	CWnd	*pMain = AfxGetMainWnd();
	CRect	MonRect;
	if (pMain != NULL) {
		GetWorkspaceRect(pMain->m_hWnd, MonRect);	// handles multiple monitors
		CSize	sz(OurOrigSize.cx >> 1, OurOrigSize.cy >> 1);
		r -= r.TopLeft();
		r += MonRect.CenterPoint() - sz;	// + center - half our original size
	}
	//
	// use our instance count to cascade multiple instances
	//
	CPoint	CascadeOfs(GetSystemMetrics(SM_CXSIZE),
		GetSystemMetrics(SM_CYSIZE));	// same cascade offset MessageBox uses
	r += CSize(m_InstCount * CascadeOfs.x, m_InstCount * CascadeOfs.y);
	//
	// make sure we fit on monitor
	//
	if (pMain != NULL) {
		CPoint	delta = CPoint(max(r.right - MonRect.right, 0),
			max(r.bottom - MonRect.bottom, 0));
		r -= delta;
	}
	//
	// reposition and resize our window
	//
	MoveWindow(r);
	//
	// move button below text and center it horizontally
	//
	m_OkBtn.GetWindowRect(r);
	ScreenToClient(r);
	CRect	OurClientRect;
	GetClientRect(OurClientRect);
	CSize	BtnSize = r.Size();
	r.left = (OurClientRect.Width() - BtnSize.cx) / 2;
	r.right = r.left + BtnSize.cx;
	r.top += TextDelta.cy;
	r.bottom = r.top + BtnSize.cy;
	m_OkBtn.MoveWindow(r);
	//
	// increment our instance count
	//
	m_InstCount++;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMsgBoxDlg::PostNcDestroy() 
{
	if (m_AutoDelete)
		delete this;	// destroy our instance
	m_InstCount--;	// decrement our instance count
}

void CMsgBoxDlg::OnOK() 
{
	DestroyWindow();
}

void CMsgBoxDlg::OnCancel() 
{
	DestroyWindow();
}

void CMsgBoxDlg::OnDestroy() 
{
	if (m_InstCount > 1) {	// if another instance of us exists
		// try to find it and give it focus; this lets the user easily close
		// a series of modeless message boxes by holding down the Enter key
		CWnd	*pMain = AfxGetMainWnd();
		if (pMain != NULL) {
			CWnd	*wp = GetWindow(GW_HWNDFIRST);
			while (wp != NULL) {
				CMsgBoxDlg	*pMsgBox = DYNAMIC_DOWNCAST(CMsgBoxDlg, wp);
				// if window is a modeless message box, but isn't us
				if (pMsgBox != NULL && pMsgBox != this) {
					pMsgBox->SetFocus();	// give it focus
					break;
				}
				wp = wp->GetNextWindow();
			}
		}
	}
	CDialog::OnDestroy();
}

void CMsgBoxDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect	r;
	m_TextCtrl.GetWindowRect(r);
	ScreenToClient(r);
	CPoint	org = r.TopLeft();
	HGDIOBJ	PrevFont = dc.SelectObject(GetFont());	// use our current font
	dc.SetBkColor(GetSysColor(COLOR_3DFACE));
	for (int i = 0; i < m_TextLine.GetSize(); i++) {
		// it's crucial to specify nTabOrigin, otherwise the output text won't
		// match the extent returned by GetTabbedTextExtent; this is why we're
		// drawing the text here, instead of letting m_TextCtrl draw the text
		dc.TabbedTextOut(org.x, org.y, m_TextLine[i], 0, NULL, r.left);
		org.y += m_LineHeight;
	}
	dc.SelectObject(PrevFont);	// restore DC's previous font
}
