// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jan12	initial version

        wrapper for Gilles Vollant's minizip library
 
*/

#ifndef CMINIZIP_INCLUDED
#define CMINIZIP_INCLUDED

#include "unzip.h"

class CMinizip : public CObject {
public:
// Construction
	CMinizip();
	~CMinizip();

// Attributes
	bool	IsOpen() const;

// Operations
	bool	Open(LPCTSTR ZipPath);
	bool	Close();
	bool	Extract(LPCTSTR FileName, LPCTSTR DestPath, UINT BufSize = 4096);

protected:
// Data members
	unzFile	m_ZipFile;	// zip file instance
};

inline bool CMinizip::IsOpen() const
{
	return(m_ZipFile != NULL);
}

#endif
