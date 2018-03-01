// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      07aug06	initial version
		01		23nov07	support Unicode

        frame size control
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "FrameSizeCtrl.h"
#include "Persist.h"
#include "NumEdit.h"

#define RK_FRAME_WIDTH		"FrameWidth"
#define	RK_FRAME_HEIGHT		"FrameHeight"

const SIZE CFrameSizeCtrl::m_PresetFrameSize[PRESETS] = {
	{160, 120},
	{320, 240},
	{640, 480},
	{800, 600},
	{1024, 768}
};

CFrameSizeCtrl::CFrameSizeCtrl(CComboBox& Size, CNumEdit& Width, 
									CNumEdit& Height, LPCTSTR Tag) :
	m_SizeCombo(Size),
	m_WidthEdit(Width),
	m_HeightEdit(Height),
	m_Tag(Tag)
{
}

void CFrameSizeCtrl::Read(CSize DefSize)
{
	m_Size.cx = CPersist::GetInt(REG_SETTINGS, m_Tag + RK_FRAME_WIDTH, DefSize.cx);
	m_Size.cy = CPersist::GetInt(REG_SETTINGS, m_Tag + RK_FRAME_HEIGHT, DefSize.cy);
}

void CFrameSizeCtrl::Write()
{
	CPersist::WriteInt(REG_SETTINGS, m_Tag + RK_FRAME_WIDTH, m_Size.cx);
	CPersist::WriteInt(REG_SETTINGS, m_Tag + RK_FRAME_HEIGHT, m_Size.cy);
}

void CFrameSizeCtrl::InitCtrls()
{
	m_WidthEdit.SetFormat(CNumEdit::DF_INT);
	m_HeightEdit.SetFormat(CNumEdit::DF_INT);
	CString	s;
	for (int i = 0; i < PRESETS; i++) {
		s.Format(_T("%d x %d"), m_PresetFrameSize[i].cx, m_PresetFrameSize[i].cy);
		m_SizeCombo.AddString(s);
	}
	m_SizeCombo.AddString(LDS(IDS_OPTS_CUSTOM));
	SetSize(m_Size);
}

void CFrameSizeCtrl::OnOK()
{
	m_Size.cx = m_WidthEdit.GetIntVal();
	m_Size.cy = m_HeightEdit.GetIntVal();
}

void CFrameSizeCtrl::OnSelChange()
{
	int	sel = m_SizeCombo.GetCurSel();
	if (sel >= 0 && sel < PRESETS) {
		CSize	sz = m_PresetFrameSize[sel];
		m_WidthEdit.SetVal(sz.cx);
		m_HeightEdit.SetVal(sz.cy);
	}
	m_WidthEdit.EnableWindow(sel == PRESETS);
	m_HeightEdit.EnableWindow(sel == PRESETS);
}

void CFrameSizeCtrl::SetSize(CSize Size)
{
	m_WidthEdit.SetVal(Size.cx);
	m_HeightEdit.SetVal(Size.cy);
	int	sel = PRESETS;
	for (int i = 0; i < PRESETS; i++) {
		if (Size == m_PresetFrameSize[i])
			sel = i;
	}
	m_SizeCombo.SetCurSel(sel);
	m_WidthEdit.EnableWindow(sel == PRESETS);
	m_HeightEdit.EnableWindow(sel == PRESETS);
}

bool CFrameSizeCtrl::IsCustomSize() const
{
	return(m_SizeCombo.GetCurSel() == PRESETS);
}
