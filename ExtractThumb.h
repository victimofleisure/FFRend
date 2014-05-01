// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      17sep07	initial version

		extract thumbnail from file
 
*/

#ifndef CEXTRACTTHUMB_INCLUDED
#define	CEXTRACTTHUMB_INCLUDED

class CExtractThumb : WObject {
public:
// Construction
	CExtractThumb();
	~CExtractThumb();

// Operations
	bool	Create(LPCTSTR Folder, SIZE Size, DWORD ColorDepth);
	void	Destroy();
	HBITMAP	Extract(LPCTSTR FileName);

protected:
	LPMALLOC		m_pMalloc;	// memory allocator interface
	LPSHELLFOLDER	m_pDesktopFolder;	// desktop folder's shell interface
	LPITEMIDLIST	m_pFolderIdl;	// target folder's item ID list
	LPSHELLFOLDER	m_pItemFolder;	// target folder's shell interface
	SIZE	m_Size;			// thumbnail size, in pixels
	DWORD	m_ColorDepth;	// thumbnail color depth, in bits per pixel
};
	
#endif
