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

#include "stdafx.h"
#include "ZInflate.h"

CZInflate::CZInflate(CFile& InpFile, UINT BufSize) :
	m_InpFile(InpFile), m_BufSize(BufSize)
{
	ZeroMemory(&m_zs, sizeof(m_zs));
	m_zerr = 0;
}

CZInflate::~CZInflate()
{
	inflateEnd(&m_zs);
}

bool CZInflate::Init()
{
	if ((m_zerr = inflateInit(&m_zs)) != Z_OK)
		return(FALSE);
	m_InpBuf.SetSize(m_BufSize);
	m_zs.next_in = m_InpBuf.GetData();
	m_zs.avail_in = m_InpFile.Read(m_InpBuf.GetData(), m_BufSize);
	return(TRUE);
}

bool CZInflate::Read(BYTE *buf, UINT len)
{
	m_zs.next_out = buf;
	m_zs.avail_out = len;
	while (m_zs.avail_out > 0) {
		if ((m_zerr = inflate(&m_zs, 0)) == Z_OK) {
			if (!m_zs.avail_in) {
				m_zs.next_in = m_InpBuf.GetData();
				m_zs.avail_in = m_InpFile.Read(m_InpBuf.GetData(), m_BufSize);
			}
		} else {
			if (m_zerr != Z_STREAM_END)
				return(FALSE);
		}
	}
	return(TRUE);
}
