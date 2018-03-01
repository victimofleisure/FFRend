// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      30jan07	initial version

        zlib compress buffers to a CFile
 
*/

#ifndef CZDEFLATE_INCLUDED
#define CZDEFLATE_INCLUDED

#include "zlib.h"

class CZDeflate : public WObject {
public:
// Construction
	CZDeflate(CFile& OutFile, UINT BufSize);
	~CZDeflate();

// Attributes
	UINT	GetComprLen() const;
	int		GetLastError() const;

// Operations
	bool	Init(int level);
	bool	Write(BYTE *buf, UINT len);
	bool	End();

protected:
// Data members
	CFile&		m_OutFile;	// reference to output file
	UINT		m_BufSize;	// size of output buffer in bytes
	CByteArray	m_OutBuf;	// output buffer
	z_stream	m_zs;		// zlib stream
	int			m_zerr;		// zlib most recent return code
};

inline UINT CZDeflate::GetComprLen() const
{
	return(m_zs.total_out);
}

inline int CZDeflate::GetLastError() const
{
	return(m_zerr);
}

#endif
