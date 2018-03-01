// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version

        zlib uncompress a CFile to buffers
 
*/

#ifndef CZINFLATE_INCLUDED
#define CZINFLATE_INCLUDED

#include "zlib.h"

class CZInflate : public WObject {
public:
// Construction
	CZInflate(CFile& InpFile, UINT BufSize);
	~CZInflate();

// Attributes
	int		GetLastError() const;

// Operations
	bool	Init();
	bool	Read(BYTE *buf, UINT len);

protected:
// Data members
	CFile&		m_InpFile;	// reference to input file
	UINT		m_BufSize;	// size of input buffer in bytes
	CByteArray	m_InpBuf;	// input buffer
	z_stream	m_zs;		// zlib stream
	int			m_zerr;		// zlib most recent return code
};

inline int CZInflate::GetLastError() const
{
	return(m_zerr);
}

#endif
