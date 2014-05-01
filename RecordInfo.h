// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		02aug07	initial version
		01		24may10	add archive version

		container for record information

*/

#ifndef CRECORDINFO_INCLUDED
#define CRECORDINFO_INCLUDED

typedef struct tagRECORD_INFO {
	// APPEND ONLY! This struct is included in the job control archive, so 
	// don't delete or reorder members, else preexisting job control files
	// will be garbled. Only simple types that can handle bitwise copy are
	// permitted here; objects must be added to CRecordInfo instead.
	SIZE	m_OutFrameSize;		// output frame size
	float	m_OutFrameRate;		// output frame rate
	int		m_BitCount;			// output bits per pixel
	int		m_Duration;			// recording length in seconds
	int		m_FrameCount;		// recording length in frames
	bool	m_Unlimited;		// if true, record until stopped by user
	bool	m_UseAviLength;		// if true, get duration from source AVI file
	bool	m_UseInpFrameSize;	// if true, force output frame size to match input
	bool	m_UseInpFrameRate;	// if true, force output frame rate to match input
	bool	m_QueueJob;			// if true, don't run job now, add to job control
} RECORD_INFO;

class CRecordInfo : public CObject, public RECORD_INFO {
public:
	DECLARE_SERIAL(CRecordInfo);

// Constants
	enum {
		ARCHIVE_VERSION = 1			// archive version number
	};

// Public data; members MUST be included in Copy and Serialize

// Construction
	CRecordInfo();
	CRecordInfo(const CRecordInfo& Info);
	CRecordInfo& operator=(const CRecordInfo& Info);

// Operations
	void	Serialize(CArchive& ar);
	RECORD_INFO&	GetBaseInfo();

protected:
// Helpers
	void	Copy(const CRecordInfo& Info);
};

inline CRecordInfo::CRecordInfo(const CRecordInfo& Info)
{
	Copy(Info);
}

inline CRecordInfo& CRecordInfo::operator=(const CRecordInfo& Info)
{
	Copy(Info);
	return(*this);
}

inline RECORD_INFO& CRecordInfo::GetBaseInfo()
{
	return(*this);	// automatic upcast to base struct
}

template<> inline void AFXAPI
SerializeElements<CRecordInfo>(CArchive& ar, CRecordInfo* pObj, int nCount)
{
    for (int i = 0; i < nCount; i++, pObj++)
        pObj->Serialize(ar);
}

#endif
