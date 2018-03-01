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

#ifndef CFRAMESIZECTRL_INCLUDED
#define	CFRAMESIZECTRL_INCLUDED

class CNumEdit;

class CFrameSizeCtrl : public WObject {
public:
// Construction
	CFrameSizeCtrl(CComboBox& Size, CNumEdit& Width, CNumEdit& Height, LPCTSTR Tag);

// Constants
	enum {	// preset frame sizes
		PFS_160_120,
		PFS_320_240,
		PFS_640_480,
		PFS_800_600,
		PFS_1024_768,
		PRESETS
	};
	static const SIZE	m_PresetFrameSize[PRESETS];

// Public data
	CSize	m_Size;
	CString		m_Tag;

// Attributes
	void	SetSize(CSize Size);
	bool	IsCustomSize() const;

// Operations
	void	Read(CSize DefSize);
	void	Write();
	void	InitCtrls();
	void	OnOK();
	void	OnSelChange();

protected:
// Member data
	CComboBox&	m_SizeCombo;
	CNumEdit&	m_WidthEdit;
	CNumEdit&	m_HeightEdit;
};

#endif
