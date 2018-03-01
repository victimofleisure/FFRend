// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
        01      14sep10	add helpers
		02		11jan11	add priming accessors
        03      04mar11	make history size unsigned
		04		16apr11	add UsingProcessCopy
		05		13apr12	standardize thread function
		06		01jun12	add DisconnectOutput

		parallel plugin
 
*/

#pragma once

#include "EngineThread.h"
#include "PluginHelper.h"

class CEngine;

class CPlugin : public CEngineThread {
public:
// Types
	class CInput {
	public:
		CInput();
		PFRAME	m_Frame;			// pointer to input frame in process
		int		m_SlotIdx;			// input slot index, or -1 for default
		bool	m_HasOutput;		// true if connected to an output queue
		int		m_PrimeFrames;		// number of frames of priming needed
		int		m_QueueSize;		// maximum number of queued input frames
	};
	typedef CArrayEx<CInput, CInput&> CInputArray;

// Construction
	CPlugin();
	~CPlugin();
	bool	Create(CEngine& Engine, int NumInputs);
	bool	Destroy();

// Attributes
	int		GetSlotIdx() const;
	void	SetSlotIdx(int SlotIdx);
	int		GetPluginIdx() const;
	void	SetPluginIdx(int PlugIdx);
	CFrameQueue	*GetInputQueue(int InpIdx);
	CFrameQueue	*GetOutputQueue(int OutIdx);
	void	SetOutputQueue(int OutIdx, CFrameQueue *Queue);
	int		GetOutputCount() const;
	bool	IsSource() const;
	bool	IsMixer() const;
	bool	CanRender() const;
	bool	SetSource(bool Enable);
	int		GetNumInputs() const;
	bool	SetNumInputs(int Inputs);
	int		GetInputSlot(int InpIdx) const;
	void	GetInputSlot(int InpIdx, int*& SlotIdx);
	bool	SetInputSlot(int InpIdx, int SlotIdx);
	int		GetInputPlugin(int InpIdx) const;
	bool	SetInputPlugin(int InpIdx, int PlugIdx);
	CPlugin	*GetInputSource(int InpIdx);
	void	GetInput(int InpIdx, CInput& Input) const;
	volatile PFRAME	GetInputFrame(int InpIdx) const;
	volatile PFRAME	GetOutputFrame() const;
	bool	GetProcessCopy() const;
	bool	SetProcessCopy(bool Enable);
	bool	UsingProcessCopy() const;
	int		GetCopyPref() const;
	bool	SetCopyPref(int Type);
	bool	GetBypass() const;
	void	SetBypass(bool Enable);
	int		GetInputQueueSize(int InpIdx);
	bool	SetInputQueueSize(int InpIdx, int Size);
	int		GetInputPriming(int InpIdx);
	bool	SetInputPriming(int InpIdx, int Frames);
	int		GetProcessDelay() const;
	void	SetProcessDelay(int Delay);
	void	SetTimeout(DWORD Timeout);
	int		GetThreadCount() const;
	bool	SetThreadCount(int Threads);
	int		GetHelperCount() const;
	CPluginHelper&	GetHelper(int HelpIdx);
	bool	SetHistorySize(UINT Size);

// Operations
	void	ResetQueues();
	void	ConnectInput(int InpIdx, CPlugin& Plugin);
	void	ConnectOutput(CFrameQueue& Queue);
	bool	DisconnectOutput(CFrameQueue& Queue);
	int		FindOutput(const CFrameQueue& Queue) const;
	bool	Run(bool Enable);
	bool	Pause(bool Enable);
	void	DumpState(FILE *fp);

protected:
// Types
	typedef CArrayEx<CFrameQueue, CFrameQueue&> CFrameQueueArray;
	typedef CArrayEx<CFrameQueue*, CFrameQueue*> CFrameQueuePtrArray;

// Member data
	CFrameQueueArray	m_InputQueue;	// array of input frame queues
	CFrameQueuePtrArray	m_OutputQueue;	// array of pointers to output frame queues
	CInputArray	m_Input;				// array of inputs
	CFrameBufArray	m_InFrameBuf;		// array of pointers to input frame buffers
	PFRAME	m_OutFrame;			// output frame pointer
	int		m_SlotIdx;			// our index in engine's slot array
	int		m_PlugIdx;			// our index in engine's plugin array
	bool	m_Bypass;			// true if we're in bypass mode
	bool	m_IsSource;			// true if source; false if effect
	bool	m_ProcessCopy;		// true if process copy supported
	bool	m_UseProcessCopy;	// true if we're using process copy
	int		m_NumInputs;		// number of inputs
	int		m_CopyPref;			// copy preference
	bool	m_CanRender;		// true if connected to render queue
	int		m_ProcessDelay;		// processing delay in milliseconds
	CPluginHelperArray	m_Helper;	// array of helper threads
	int		m_ThreadCount;		// number of processing threads
	int		m_NextHelper;		// index of next helper thread

// Helpers
	static	UINT	ThreadFunc(LPVOID Arg);
	bool	CreateHelpers(int Helpers);
	bool	DestroyHelpers();
	bool	Delegate();
	friend class CSpider;
};

typedef CRefPtr<CPlugin> CSlotPtr;
typedef CArrayEx<CSlotPtr, CSlotPtr&> CSlotArray;
typedef CArrayEx<CPlugin *, CPlugin *> CPluginArray;

inline int CPlugin::GetSlotIdx() const
{
	return(m_SlotIdx);
}

inline void CPlugin::SetSlotIdx(int SlotIdx)
{
	m_SlotIdx = SlotIdx;
}

inline int CPlugin::GetPluginIdx() const
{
	return(m_PlugIdx);
}

inline void CPlugin::SetPluginIdx(int PlugIdx)
{
	m_PlugIdx = PlugIdx;
}

inline CFrameQueue *CPlugin::GetInputQueue(int InpIdx)
{
	return(&m_InputQueue[InpIdx]);
}

inline CFrameQueue *CPlugin::GetOutputQueue(int OutIdx)
{
	return(m_OutputQueue[OutIdx]);
}

inline void CPlugin::SetOutputQueue(int OutIdx, CFrameQueue *Queue)
{
	m_OutputQueue[OutIdx] = Queue;
}

inline void CPlugin::ConnectOutput(CFrameQueue& Queue)
{
	m_OutputQueue.Add(&Queue);
}

inline int CPlugin::GetOutputCount() const
{
	return(m_OutputQueue.GetSize());
}

inline bool CPlugin::IsSource() const
{
	return(m_IsSource);
}

inline bool CPlugin::IsMixer() const
{
	return(m_NumInputs > 1);
}

inline bool CPlugin::CanRender() const
{
	return(m_CanRender);
}

inline bool CPlugin::GetProcessCopy() const
{
	return(m_ProcessCopy);
}

inline bool CPlugin::UsingProcessCopy() const
{
	return(m_UseProcessCopy);
}

inline int CPlugin::GetNumInputs() const
{
	return(m_NumInputs);
}

inline int CPlugin::GetInputQueueSize(int InpIdx)
{
	return(m_Input[InpIdx].m_QueueSize);
}

inline int CPlugin::GetInputPriming(int InpIdx)
{
	return(m_Input[InpIdx].m_PrimeFrames);
}

inline int CPlugin::GetInputSlot(int InpIdx) const
{
	return(m_Input[InpIdx].m_SlotIdx);
}

inline void CPlugin::GetInputSlot(int InpIdx, int*& SlotIdx)
{
	SlotIdx = &m_Input[InpIdx].m_SlotIdx;
}

inline void CPlugin::GetInput(int InpIdx, CInput& Input) const
{
	Input = m_Input[InpIdx];
}

inline volatile PFRAME CPlugin::GetInputFrame(int InpIdx) const
{
	return(m_Input[InpIdx].m_Frame);
}

inline volatile PFRAME CPlugin::GetOutputFrame() const
{
	return(m_OutFrame);
}

inline int CPlugin::GetCopyPref() const
{
	return(m_CopyPref);
}

inline bool CPlugin::GetBypass() const
{
	return(m_Bypass);
}

inline int CPlugin::GetProcessDelay() const
{
	return(m_ProcessDelay);
}

inline void CPlugin::SetProcessDelay(int Delay)
{
	m_ProcessDelay = Delay;
}

inline int CPlugin::GetThreadCount() const
{
	return(m_ThreadCount);
}

inline int CPlugin::GetHelperCount() const
{
	return(m_Helper.GetSize());
}

inline CPluginHelper& CPlugin::GetHelper(int HelpIdx)
{
	return(m_Helper[HelpIdx]);
}
