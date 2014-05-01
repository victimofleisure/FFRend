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

#include "stdafx.h"
#include "ZDeflate.h"

CZDeflate::CZDeflate(CFile& OutFile, UINT BufSize) :
	m_OutFile(OutFile), m_BufSize(BufSize)
{
	ZeroMemory(&m_zs, sizeof(m_zs));
	m_zerr = 0;
}

CZDeflate::~CZDeflate()
{
	deflateEnd(&m_zs);
}

bool CZDeflate::Init(int Level)
{
	if ((m_zerr = deflateInit(&m_zs, Level)) != Z_OK)
		return(FALSE);
	m_OutBuf.SetSize(m_BufSize);
	m_zs.next_out = m_OutBuf.GetData();
	m_zs.avail_out = m_BufSize;
	return(TRUE);
}

bool CZDeflate::Write(BYTE *buf, UINT len)
{
	m_zs.next_in = buf;
	m_zs.avail_in = len;
	while (m_zs.avail_in > 0) {
		if ((m_zerr = deflate(&m_zs, 0)) != Z_OK)
			return(FALSE);
		if (!m_zs.avail_out) {
			m_OutFile.Write(m_OutBuf.GetData(), m_BufSize);
			m_zs.next_out = m_OutBuf.GetData();
			m_zs.avail_out = m_BufSize;
		}
	}
	return(TRUE);
}

bool CZDeflate::End()
{
	while ((m_zerr = deflate(&m_zs, Z_FINISH)) == Z_OK) {
		m_OutFile.Write(m_OutBuf.GetData(), m_BufSize);
		m_zs.next_out = m_OutBuf.GetData();
		m_zs.avail_out = m_BufSize;
	}
	if (m_zerr != Z_STREAM_END)
		return(FALSE);
	if (m_zs.avail_out)
		m_OutFile.Write(m_OutBuf.GetData(), m_BufSize - m_zs.avail_out);
	return(TRUE);
}
