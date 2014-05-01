// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		24jun10	make OnStall virtual
        02      14sep10	add plugin helpers
        03      04mar11	make history size unsigned
		04		30apr11	add thread iterator
		05		01dec11	add frame memory limit
		06		01jun12	add FlushQueue

		parallel plugin engine
 
*/

#pragma once

#include "ArrayEx.h"
#include "Plugin.h"
#include "FrameQueue.h"

class CRenderer;

class CEngine : public WObject {
public:
// Constants
	enum {	// plugin types
		PT_EFFECT,
		PT_SOURCE,
		PLUGIN_TYPES
	};
	enum {	// copy preferences
		CPF_NONE,
		CPF_INPLACE,
		CPF_COPY,
		CPF_BOTH,
		COPY_PREFS
	};
	enum {	// mixer emulation functions
		MEF_NONE,
		MEF_SUM,
		MEF_AVG,
		MIXER_EMULATIONS
	};

// Types
	class CRun : public WObject {
	public:
		CRun(CEngine& Engine, bool Enable = FALSE);
		~CRun();
		operator bool() const;

	protected:
		CEngine&	m_Engine;	// reference to engine
		bool	m_PrevRun;		// true if previously running
		bool	m_Succeeded;	// true if run/stop succeeded
	};
	class CPause : public WObject {
	public:
		CPause(CEngine& Engine, bool Enable = TRUE);
		~CPause();
		operator bool() const;

	protected:
		CEngine&	m_Engine;
		bool	m_PrevPause;
		bool	m_Succeeded;
	};
	class CSlotChange : public CRun {
	public:
		CSlotChange(CEngine& Engine);
		~CSlotChange();

	protected:
		bool	m_PrevInChange;	// true if previously in slot change
	};
	class CExtendSlotChange : public WObject {
	public:
		CExtendSlotChange(CEngine& Engine);
		~CExtendSlotChange();

	protected:
		CEngine&	m_Engine;	// reference to engine
		bool	m_PrevRun;		// true if previously running
		bool	m_PrevInChange;	// true if previously in slot change
		bool	m_PrevExtend;	// true if previously extending slot change
	};
	class CThreadIter : public WObject {
	public:
		CThreadIter(CEngine& Engine);
		bool	End();
		CEngineThread&	operator*();
		int		GetPluginIndex() const;
		int		GetThreadIndex() const;

	protected:
		CEngine&	m_Engine;		// engine to iterate
		int		m_PlugIdx;			// current plugin index
		int		m_ThreadIdx;		// current thread index
		CEngineThread	*m_Thread;	// current thread pointer
	};

// Constants
	enum {
		FRAME_TIMEOUT = 5000,
		TIMER_NOOP = -1,
		RENDER_THREAD = -1,
		PLUGIN_QUEUE_SIZE = 1,
	};

// Construction
	CEngine(CRenderer& Renderer);
	virtual	~CEngine();
	bool	Create();
	bool	Destroy();

// Attributes
	CRenderer&	GetRenderer();
	bool	IsRunning() const;
	bool	IsPaused() const;
	bool	IsStalled() const;
	int		GetSlotCount() const;
	int		GetPluginCount() const;
	bool	IsValidSlot(int SlotIdx) const;
	bool	IsLoaded(int SlotIdx) const;
	CPlugin	*GetSlot(int SlotIdx);
	const CPlugin	*GetSlot(int SlotIdx) const;
	CPlugin&	GetPlugin(int PlugIdx);
	const CPlugin&	GetPlugin(int PlugIdx) const;
	WEvent&	GetStopEvent();
	CFrameQueue *GetFreeQueue();
	CFrameQueue	*GetRenderQueue();
	LPCTSTR	GetInputName(int SlotIdx) const;
	static LPCTSTR	GetQueueReturnName(int ReturnCode);
	bool	InSlotChange() const;
	void	SetBypass(int SlotIdx, bool Enable);
	bool	GetBypass(int SlotIdx) const;
	void	SetOptimizeRouting(bool Enable);
	UINT	GetHistorySize() const;
	bool	SetHistorySize(UINT Size);
	int		GetPriority() const;
	bool	SetPriority(int Priority);
	int		GetFrameCount() const;
	int		GetMaxFrameCount() const;
	int		GetFrameLength() const;
	bool	SetFrameLength(int Length);
	DWORD	GetFrameTimeout() const;
	void	SetFrameTimeout(DWORD Timeout);
	UINT	GetFrameMemoryLimit() const;
	bool	SetFrameMemoryLimit(UINT Size);
	int		GetThreadCount() const;

// Operations
	bool	RemoveAll();
	bool	Insert(int SlotIdx, CSlotPtr Slot);
	bool	Delete(int SlotIdx);
	bool	Load(int SlotIdx, CSlotPtr Slot);
	bool	Move(int Src, int Dst);
	int		PluginToSlot(int PlugIdx) const;
	int		SlotToPlugin(int SlotIdx) const;
	void	DumpStallInfo(FILE *fp, LPCTSTR Tag, const STALL_INFO& Info);
	bool	FlushQueue(CFrameQueue& Queue);

// Overrideables
	virtual	bool	Run(bool Enable);
	virtual	bool	Pause(bool Enable);
	virtual	void	OnError(int ErrorCode, LPCTSTR Context = NULL);
	virtual	void	DumpState(FILE *fp);
	virtual	CString	GetQueueName(const CFrameQueue *Queue, bool Scope = TRUE) const;
	virtual	void	OnStall(CEngineThread *Staller);
	virtual	void	PostCrawl();

protected:
// Constants
	static const LPCTSTR	m_QueueReturnName[CFrameQueue::RETURN_CODES];

// Types
	class CInputIter {
	public:
		CInputIter(const CSlotArray& Slot);
		bool	End();
		int&	operator*();

	protected:
		const CSlotArray&	m_Slot;	// plugin slot array to iterate over
		int		m_SlotIdx;			// current slot index
		int		m_InpIdx;			// current input index
		int		*m_Input;			// current input pointer
	};

// Member data
	CRenderer&	m_Renderer;		// output renderer
	bool	m_IsRunning;		// true if engine is running
	bool	m_IsPaused;			// true if engine is paused
	bool	m_InSlotChange;		// true if slot change is in progress
	bool	m_ExtendSlotChange;	// true if deferring end of slot change
	CSlotArray	m_Slot;			// array of pointers to ref-counted plugin slots
	CPluginArray	m_Plugin;	// array of pointers to plugins
	CFrameArray	m_Frame;		// array of pointers to frames
	CFrameQueue	m_FreeQueue;	// free frame pointer queue
	CFrameQueue	m_RenderQueue;	// render frame pointer queue
	WEvent	m_Stop;				// unblocks free and output queues
	long	m_StallCount;		// number of stalls
	bool	m_OptimizeRouting;	// true if optimizing routing
	UINT	m_HistorySize;		// size of process history
	int		m_Priority;			// plugin thread priority
	int		m_FrameLength;		// length of a frame in bytes
	DWORD	m_FrameTimeout;		// frame timeout, in milliseconds
	int		m_ThreadCount;		// number of processing threads
	UINT	m_FrameMemoryLimit;	// maximum memory allocated for frames, in MB
	int		m_MaxFrameCount;	// maximum number of frames

// Overridables
	virtual	void	OnBeginSlotChange();
	virtual	void	OnEndSlotChange();
	virtual	void	OnExtendSlotChange();
	virtual	void	PostSlotChange();
	virtual	bool	RunInit();

// Helpers
	bool	AllocFrames(int FrameCount);
	void	DestroyFrames();
	void	UpdateMaxFrameCount();
	friend class CSlotChange;
	friend class CExtendSlotChange;
};

inline CEngine::CRun::operator bool() const
{
	return(m_Succeeded);
}

inline CEngine::CPause::operator bool() const
{
	return(m_Succeeded);
}

inline CEngineThread& CEngine::CThreadIter::operator*()
{
	return(*m_Thread);
}

inline int CEngine::CThreadIter::GetPluginIndex() const
{
	return(m_PlugIdx);
}

inline int CEngine::CThreadIter::GetThreadIndex() const
{
	return(m_ThreadIdx);
}

inline CRenderer& CEngine::GetRenderer()
{
	return(m_Renderer);
}

inline int CEngine::GetSlotCount() const
{
	return(m_Slot.GetSize());
}

inline int CEngine::GetPluginCount() const
{
	return(m_Plugin.GetSize());
}

inline bool CEngine::IsValidSlot(int SlotIdx) const
{
	return(SlotIdx >= 0 && SlotIdx < GetSlotCount());
}

inline bool CEngine::IsLoaded(int SlotIdx) const
{
	return(!m_Slot[SlotIdx].IsEmpty());
}

inline bool CEngine::IsRunning() const
{
	return(m_IsRunning);
}

inline bool CEngine::IsPaused() const
{
	return(m_IsPaused);
}

inline bool CEngine::IsStalled() const
{
	return(m_StallCount > 0);
}

inline int CEngine::PluginToSlot(int PlugIdx) const
{
	return(m_Plugin[PlugIdx]->GetSlotIdx());
}

inline int CEngine::SlotToPlugin(int SlotIdx) const
{
	return(IsLoaded(SlotIdx) ? m_Slot[SlotIdx]->GetPluginIdx() : -1);
}

inline CPlugin *CEngine::GetSlot(int SlotIdx)
{
	return(m_Slot[SlotIdx]);
}

inline const CPlugin *CEngine::GetSlot(int SlotIdx) const
{
	return(m_Slot[SlotIdx]);
}

inline CPlugin& CEngine::GetPlugin(int PlugIdx)
{
	return(*m_Plugin[PlugIdx]);
}

inline const CPlugin& CEngine::GetPlugin(int PlugIdx) const
{
	return(*m_Plugin[PlugIdx]);
}

inline WEvent& CEngine::GetStopEvent()
{
	return(m_Stop);
}

inline CFrameQueue *CEngine::GetFreeQueue()
{
	return(&m_FreeQueue);
}

inline CFrameQueue *CEngine::GetRenderQueue()
{
	return(&m_RenderQueue);
}

inline LPCTSTR CEngine::GetQueueReturnName(int ReturnCode)
{
	ASSERT(ReturnCode >= 0 && ReturnCode < CFrameQueue::RETURN_CODES);
	return(m_QueueReturnName[ReturnCode]);
}

inline bool CEngine::InSlotChange() const
{
	return(m_InSlotChange);
}

inline int& CEngine::CInputIter::operator*()
{
	return(*m_Input);
}

inline bool CEngine::GetBypass(int SlotIdx) const
{
	return(m_Slot[SlotIdx]->GetBypass());
}

inline void CEngine::SetBypass(int SlotIdx, bool Enable)
{
	m_Slot[SlotIdx]->SetBypass(Enable);
}

inline void CEngine::SetOptimizeRouting(bool Enable)
{
	m_OptimizeRouting = Enable;
}

inline UINT CEngine::GetHistorySize() const
{
	return(m_HistorySize);
}

inline int CEngine::GetPriority() const
{
	return(m_Priority);
}

inline int CEngine::GetFrameCount() const
{
	return(m_Frame.GetSize());
}

inline int CEngine::GetMaxFrameCount() const
{
	return(m_MaxFrameCount);
}

inline int CEngine::GetFrameLength() const
{
	return(m_FrameLength);
}

inline DWORD CEngine::GetFrameTimeout() const
{
	return(m_FrameTimeout);
}

inline int CEngine::GetThreadCount() const
{
	return(m_ThreadCount);
}

inline UINT CEngine::GetFrameMemoryLimit() const
{
	return(m_FrameMemoryLimit);
}

#define STOP_ENGINE(Engine)							\
	CEngine::CRun	StopEngineObj(Engine);			\
	if (!StopEngineObj)								\
		return(FALSE);

#define SLOT_CHANGE(Engine)							\
	CEngine::CSlotChange	SlotChangeObj(Engine);	\
	if (!SlotChangeObj)								\
		return(FALSE);
