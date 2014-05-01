// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      21aug06	initial version
        01      28oct06	make GetHandle const
		02		23nov07	support Unicode
		03		27mar10	overload Open for CFile

        IPicture wrapper for loading image files
 
*/

#ifndef CPICTURE_INCLUDED
#define CPICTURE_INCLUDED

class CPicture : public WObject {
public:
// Construction
	CPicture();
	~CPicture();

// Attributes
	bool	IsOpen() const;
	HBITMAP GetHandle() const;

// Operations
	bool	Open(LPCTSTR Path);
	bool	Read(CFile& File);
	void	Close();

protected:
// Member data
	IPicture	*m_p;	// picture-loading interface
};

inline bool	CPicture::IsOpen() const
{
	return(m_p != NULL);
}

inline HBITMAP CPicture::GetHandle() const
{
	ASSERT(m_p != NULL);
	OLE_HANDLE	hp;
	if (m_p->get_Handle(&hp) == S_OK)
		return((HBITMAP)hp);
	return(NULL);
}

#endif
