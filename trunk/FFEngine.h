// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		01may11	add get/set load balance
		02		07may11	add sync oscillators
		03		18nov11	remove get/set load balance
		04		01dec11	make run while loading optional
		05		05jan12	override Run to rewind oscillators on stop
		06		11jan12	add pending acks for message broadcasts
		07		13apr12	add FindPluginByUID
		08		06may12	make FindPluginByUID const
		09		22may12	give GetSyncMods a return value

		parallel FreeFrame plugin engine
 
*/

#pragma once

#include "Engine.h"
#include "EngineTypes.h"
#include "FFProject.h"
#include "FFPluginEx.h"

// enumerate derived engine errors
#define ENGERR_DEF(x) x,
enum {
	ENGERR_LAST = ENGINE_ERRORS - 1,	// append to base errors
	#include "FFEngineErrs.h"
	FFENGINE_ERRORS		// total count including base and derived errors
};

// for compatibility with ParaPET shared code, unused here
enum {	// frame buffer field indices
	FRAME_CONTENT,	// user-defined data
	FRAME_TIME,		// time in frames
};

class CFFEngine : public CEngine {
public:
// Construction
	CFFEngine(CRenderer& Renderer);
	~CFFEngine();

// Types
	typedef CFFProject::ROUTE ROUTE;
	typedef CFFProject::CRouting CRouting;

// Attributes
	CFFPluginEx	*GetSlot(int SlotIdx);
	const CFFPluginEx	*GetSlot(int SlotIdx) const;
	CFFPluginEx&	GetPlugin(int PlugIdx);
	const CFFPluginEx&	GetPlugin(int PlugIdx) const;
	void	GetRouting(int SlotIdx, CRouting& Routing) const;
	void	SetRouting(const CRouting& Routing);
	void	GetProject(CFFProject& Project);
	bool	SetProject(const CFFProject& Project);
	CSize	GetFrameSize() const;
	bool	SetFrameSize(CSize FrameSize);
	UINT	GetColorDepth() const;
	UINT	GetFFColorDepth() const;
	bool	SetColorDepth(UINT ColorDepth);
	bool	SetFrameProps(CSize FrameSize, UINT ColorDepth);
	float	GetFrameRate() const;
	void	SetFrameRate(float Freq);
	int		GetCurSel() const;
	CFFPluginEx	*GetCurPlugin();
	float	GetSpeed() const;
	void	SetSpeed(float Speed);
	UINT	GetUID();
	bool	InSolo() const;
	LPCTSTR	GetClipPath(int SlotIdx);
	WEvent&	GetModalDoneEvent();
	bool	OpeningClip() const;
	void	SetRandomSeed(UINT Seed, bool UseTime);
	UINT	GetRandomSeed(bool& UseTime);
	bool	IsConnected(int Src, int Dst, int InpIdx);
	bool	GetRunWhileLoading() const;
	void	SetRunWhileLoading(bool Enable);

// Operations
	bool	Create();
	CFFPluginEx	*CreatePlugin(LPCTSTR Path);
	bool	Insert(int SlotIdx, LPCTSTR Path);
	bool	InsertEmpty(int SlotIdx);
	bool	Insert(int SlotIdx, const CFFPlugInfo& Info);
	bool	Delete(int SlotIdx);
	bool	Load(int SlotIdx, LPCTSTR Path);
	bool	Load(int SlotIdx, const CFFPlugInfo& Info);
	bool	Unload(int SlotIdx);
	bool	Move(int Src, int Dst);
	void	Solo(int SlotIdx);
	void	EndSolo();
	CFFPluginEx	*CreatePlayer(LPCTSTR Path);
	bool	InsertPlayer(int SlotIdx, LPCTSTR Path);
	bool	LoadPlayer(int SlotIdx, LPCTSTR Path);
	bool	OpenClip(int SlotIdx, LPCTSTR Path);
	bool	Connect(int Src, int Dst, int InpIdx);
	bool	SyncOscillators();
	bool	WaitForModalDone();
	void	AcknowledgeBroadcast();
	int		FindPluginByUID(UINT TargetUID) const;

// Overrideables
	virtual	void	SetCurSel(int SlotIdx);
	virtual	void	Bypass(int SlotIdx, bool Enable);

// Overrides
	virtual	bool	Run(bool Enable);

protected:
// Types
	typedef struct tagNAME_DEF {
		int		RefCount;	// how many times name is used
		int		FirstIdx;	// plugin index of first usage
	} NAME_DEF;

// Constants

// Member data
	CSize	m_FrameSize;	// frame dimensions in pixels
	UINT	m_ColorDepth;	// color depth in bits per pixel
	UINT	m_FFColorDepth;	// Freeframe color depth enumeration
	int		m_CurSel;		// slot index of currently selected plugin
	float	m_Speed;		// master speed
	float	m_FrameRate;	// frame rate
	long	m_CurUID;		// unique indentifier for plugins
	WEvent	m_ModalDone;	// set when modal command is completed
	long	m_PendingAcks;	// number of plugins that haven't acknowledged
	bool	m_InSolo;		// true if we're in solo mode
	bool	m_OpeningClip;	// true if we're opening a clip
	bool	m_RandUseTime;	// if true, use time as random seed
	bool	m_RunWhileLoading;	// if true, run while loading project
	UINT	m_RandSeed;		// random seed if use time is false

// Overrides
	virtual	void	OnEndSlotChange();

// Helpers
	bool	GetSyncMods(UINT FrameCount, CFFProject::CFFPlugInfoArray& Info);
};

inline CFFPluginEx *CFFEngine::GetSlot(int SlotIdx)
{
	CPlugin	*plug = m_Slot[SlotIdx];
	return((CFFPluginEx *)plug);
}

inline const CFFPluginEx *CFFEngine::GetSlot(int SlotIdx) const
{
	const CPlugin	*plug = m_Slot[SlotIdx];
	return((CFFPluginEx *)plug);
}

inline CFFPluginEx& CFFEngine::GetPlugin(int PlugIdx)
{
	CPlugin	*plug = m_Plugin[PlugIdx];
	return(*((CFFPluginEx *)plug));
}

inline const CFFPluginEx& CFFEngine::GetPlugin(int PlugIdx) const
{
	CPlugin	*plug = m_Plugin[PlugIdx];
	return(*((CFFPluginEx *)plug));
}

inline CSize CFFEngine::GetFrameSize() const
{
	return(m_FrameSize);
}

inline UINT CFFEngine::GetColorDepth() const
{
	return(m_ColorDepth);
}

inline UINT CFFEngine::GetFFColorDepth() const
{
	return(m_FFColorDepth);
}

inline float CFFEngine::GetFrameRate() const
{
	return(m_FrameRate);
}

inline int CFFEngine::GetCurSel() const
{
	return(m_CurSel);
}

inline CFFPluginEx *CFFEngine::GetCurPlugin()
{
	return(m_CurSel >= 0 && IsLoaded(m_CurSel) ? GetSlot(m_CurSel) : NULL);
}

inline float CFFEngine::GetSpeed() const
{
	return(m_Speed);
}

inline UINT CFFEngine::GetUID()
{
	return(++m_CurUID);
}

inline bool CFFEngine::InSolo() const
{
	return(m_InSolo);
}

inline WEvent& CFFEngine::GetModalDoneEvent()
{
	return(m_ModalDone);
}

inline bool CFFEngine::OpeningClip() const
{
	return(m_OpeningClip);
}

inline CFFPluginEx *CFFEngine::CreatePlayer(LPCTSTR Path)
{
	m_OpeningClip = TRUE;	// tell plugin we're opening a clip
	CFFPluginEx	*plug = CreatePlugin(Path);
	m_OpeningClip = FALSE;	// reset state
	return(plug);
}

inline bool CFFEngine::InsertPlayer(int SlotIdx, LPCTSTR Path)
{
	m_OpeningClip = TRUE;	// tell plugin we're opening a clip
	bool	retc = Insert(SlotIdx, Path);
	m_OpeningClip = FALSE;	// reset state
	return(retc);
}

inline bool CFFEngine::LoadPlayer(int SlotIdx, LPCTSTR Path)
{
	m_OpeningClip = TRUE;	// tell plugin we're opening a clip
	bool	retc = Load(SlotIdx, Path);
	m_OpeningClip = FALSE;	// reset state
	return(retc);
}

inline void CFFEngine::SetRandomSeed(UINT Seed, bool UseTime)
{
	m_RandSeed = Seed;
	m_RandUseTime = UseTime;
}

inline UINT CFFEngine::GetRandomSeed(bool& UseTime)
{
	UseTime = m_RandUseTime;
	return(m_RandSeed);
}

inline bool CFFEngine::OpenClip(int SlotIdx, LPCTSTR Path)
{
	ASSERT(IsValidSlot(SlotIdx) && IsLoaded(SlotIdx));
	return(GetSlot(SlotIdx)->OpenClip(Path));
}

inline LPCTSTR CFFEngine::GetClipPath(int SlotIdx)
{
	ASSERT(IsValidSlot(SlotIdx) && IsLoaded(SlotIdx));
	return(GetSlot(SlotIdx)->GetClipPath());
}

inline bool CFFEngine::IsConnected(int Src, int Dst, int InpIdx)
{
	ASSERT(IsValidSlot(Dst) && IsLoaded(Dst));
	return(GetSlot(Dst)->GetInputSlot(InpIdx) == Src);
}

inline bool CFFEngine::Connect(int Src, int Dst, int InpIdx)
{
	ASSERT(IsValidSlot(Dst) && IsLoaded(Dst));
	return(GetSlot(Dst)->SetInputSlot(InpIdx, Src));
}

inline bool CFFEngine::GetRunWhileLoading() const
{
	return(m_RunWhileLoading);
}

inline void CFFEngine::SetRunWhileLoading(bool Enable)
{
	m_RunWhileLoading = Enable;
}

inline bool CFFEngine::WaitForModalDone()
{
	return(WaitForSingleObject(m_ModalDone, m_FrameTimeout) == WAIT_OBJECT_0);
}

inline void CFFEngine::AcknowledgeBroadcast()
{
	if (!InterlockedDecrement(&m_PendingAcks))
		m_ModalDone.Set();
}
