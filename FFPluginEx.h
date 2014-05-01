// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		27apr10	add MIDI
		02		03apr11	add GetFFInstance
		03		07may11	add get/set mod phase
		04		24nov11	add scroll position
		05		05jan12	add RewindOscillators
		06		11jan12	add methods to get synchronized modulations
		07		26mar12	add DisconnectOutput
		08		13apr12	standardize thread function
		09		06may12	add extended freeframe instance to avoid reentrance
		10		22may12	add UnsafeGetSyncMods
		11		01jun12	move DisconnectOutput into base class

		parallel FreeFrame plugin
 
*/

#pragma once

#include "Plugin.h"
#include "FFInstance.h"
#include "ILockRingBuf.h"
#include "FFProject.h"
#include "FFParm.h"

// freeframe orientations
#define	FF_ORIGIN_TOP_LEFT			1
#define	FF_ORIGIN_BOTTOM_LEFT		2

// macro for creating a freeframe unique ID
#define MAKE_FFUNIQUEID(c0, c1, c2, c3) MAKELONG(MAKEWORD(c0, c1), MAKEWORD(c2, c3))

class CFFEngine;

class CFFPluginEx : public CPlugin {
public:
// Construction
	CFFPluginEx();
	~CFFPluginEx();
	bool	Create(CFFEngine& Engine, LPCTSTR Path);
	bool	CreateFF(CFFEngine& Engine, LPCTSTR Path);
	bool	Launch();

// Types
	typedef CFFPlugInfo::FFPARM_INFO FFPARM_INFO;
	typedef CFFPlugInfo::FFPLUG_INFO FFPLUG_INFO;
	typedef CFFPlugInfo::FRANGE FRANGE;
	typedef	CFFPlugInfo::CParmInfoArray CParmInfoArray;

// Constants
	enum {	// scroll position types
		SPT_PARAMETER,
		SPT_MIDI_SETUP,
		SCROLL_POSITION_TYPES
	};

// Attributes
	CFFEngine	*GetEngine();
	CString	GetPath() const;
	UINT	GetUID() const;
	CString	GetFFName() const;
	UINT	GetFFUniqueID() const;	
	CString	GetFFUniqueIDString() const;
	CFFPlugin&	GetFFPlugin();
	CFFInstance&	GetFFInstance();
	bool	SetFrameSize(CSize FrameSize);
	bool	SetColorDepth(UINT FFColorDepth);
	bool	SetFrameProps(CSize FrameSize, UINT FFColorDepth);
	int		GetParmCount() const;
	CString	GetParmName(int ParmIdx) const;
	float	GetParmVal(int ParmIdx) const;
	void	SetParmVal(int ParmIdx, float Val);
	bool	GetModEnable(int ParmIdx) const;
	void	SetModEnable(int ParmIdx, bool Enable);
	int		GetModWave(int ParmIdx) const;
	void	SetModWave(int ParmIdx, int Wave);
	float	GetModWaveNorm(int ParmIdx) const;
	void	SetModWaveNorm(int ParmIdx, float NormWave);
	float	GetModFreq(int ParmIdx) const;
	void	SetModFreq(int ParmIdx, float Freq);
	float	GetModPulseWidth(int ParmIdx) const;
	void	SetModPulseWidth(int ParmIdx, float PW);
	bool	HasModRange(int ParmIdx) const;
	FRANGE	GetModRange(int ParmIdx) const;
	void	SetModRange(int ParmIdx, FRANGE Range);
	float	GetModPhase(int ParmIdx) const;
	void	SetModPhase(int ParmIdx, float Phase);
	bool	IsModulating(int ParmIdx) const;
	void	GetParmInfo(int ParmIdx, FFPARM_INFO& Info) const;
	void	SetParmInfo(int ParmIdx, const FFPARM_INFO& Info);
	void	GetInfo(CFFPlugInfo& Info);
	void	SetInfo(const CFFPlugInfo& Info);
	DWORD	GetInfo(FFPLUG_INFO*& pInfo);
	void	SetInfo(const FFPLUG_INFO& Info);
	void	GetParmMidiInfo(int ParmIdx, int PropIdx, CMidiInfo& Info) const;
	void	SetParmMidiInfo(int ParmIdx, int PropIdx, const CMidiInfo& Info);
	void	GetMidiInfo(int PropIdx, CMidiInfo& Info) const;
	void	SetMidiInfo(int PropIdx, const CMidiInfo& Info);
	bool	GetSoloBypass() const;
	void	SetSoloBypass(bool Enable);
	bool	IsClipPlayer() const;
	LPCTSTR	GetClipPath();
	CPoint	GetScrollPos(int TypeIdx) const;
	void	SetScrollPos(int TypeIdx, CPoint ScrollPos);
	void	GetSyncMods(UINT FrameCount, CParmInfoArray& ParmInfo);

// Operations
	bool	Work();
	void	UpdateSpeed();
	void	UpdateFrameRate();
	bool	OpenClip(LPCTSTR Path);
	bool	StopWork();
	bool	WaitForCommand();
	void	RewindOscillators();
	void	AttachHelper();
	void	DetachHelper();

protected:
// Types
	enum {
		CID_PARM_SET_VAL,
		CID_PARM_SET_MOD_ENABLE,
		CID_PARM_SET_MOD_WAVE,
		CID_PARM_SET_MOD_FREQ,
		CID_PARM_SET_MOD_PW,
		CID_PARM_SET_MOD_RANGE,
		CID_PARM_SET_MOD_PHASE,
		CID_UPDATE_SPEED,
		CID_UPDATE_FRAME_RATE,
		CID_OPEN_CLIP,
		CID_GET_CLIP_PATH,
		CID_STOP_WORK,
		CID_WAIT,
		CID_GET_SYNC_MODS,
	};
	struct CMD_PARM_SET_VAL {
		float	Val;
	};
	struct CMD_PARM_SET_MOD_ENABLE {
		bool	Enable;
	};
	struct CMD_PARM_SET_MOD_WAVE {
		int		Wave;
	};
	struct CMD_PARM_SET_MOD_FREQ {
		float	Freq;
	};
	struct CMD_PARM_SET_MOD_PW {
		float	PW;
	};
	struct CMD_PARM_SET_MOD_RANGE {
		FRANGE	Range;
	};
	struct CMD_PARM_SET_MOD_PHASE {
		float	Phase;
	};
	struct CMD_PARM {
		int		Idx;			// parameter index
		union {					// union of parameter command structs
			CMD_PARM_SET_VAL		SetVal;
			CMD_PARM_SET_MOD_ENABLE	SetModEnable;
			CMD_PARM_SET_MOD_WAVE	SetModWave;
			CMD_PARM_SET_MOD_FREQ	SetModFreq;
			CMD_PARM_SET_MOD_PW		SetModPW;
			CMD_PARM_SET_MOD_RANGE	SetModRange;
			CMD_PARM_SET_MOD_PHASE	SetModPhase;
		};
	};
	struct CMD_OPEN_CLIP {
		LPSTR	Path;			// path of clip to open
	};
	struct CMD_GET_SYNC_MODS {
		UINT	FrameCount;		// target frame count for modulations
		CParmInfoArray	*ParmInfo;	// pointer to destination parameter info
	};
	struct COMMAND {
		int		CmdID;			// command identifier
		union {					// union of command structs
			CMD_PARM		Parm;
			CMD_OPEN_CLIP	OpenClip;
			CMD_GET_SYNC_MODS	GetSyncMods;
		};
	};
	typedef CArrayEx<CFFParm, CFFParm&> CFFParmArray;
	typedef CILockRingBuf<COMMAND>	CCommandQueue;

// Member data
	CString	m_Path;	// path to plugin DLL
	UINT	m_UID;	// unique identifier; not same as freeframe uniqueID
	CString	m_FFName;	// freeframe plugin name; may differ from m_Name
	DWORD	m_FFUniqueID;	// freeframe uniqueID
	CFFPlugin	m_FFPlug;	// freeframe plugin
	CFFInstance	m_FFInst;	// freeframe instance
	CFFInstanceEx	m_FFInstEx;	// extended freeframe instance
	CFFParmArray	m_Parm;	// array of freeframe parameters
	CCommandQueue	m_CmdQueue;	// command queue
	CMidiInfo	m_MidiInfo[PLUGIN_MIDI_PROPS];	// MIDI information
	bool	m_SoloBypass;	// pre-solo bypass state
	bool	m_IsClipPlayer;	// true if we're a clip player
	bool	m_ClipOpened;	// true if most recent clip open succeeded
	CString	m_ClipPath;		// if we're a clip player, path of current clip
	CPoint	m_ScrollPos[SCROLL_POSITION_TYPES];	// array of scroll positions

// Helpers
	void	UnsafeUpdateSpeed();
	void	UnsafeUpdateFrameRate();
	bool	UnsafeOpenClip(LPCSTR Path);
	LPCSTR	UnsafeGetClipPath();
	void	BeginModalCommand();
	void	EndModalCommand();
	bool	WaitForModalDone();
	bool	Instantiate(CSize FrameSize, UINT FFColorDepth);
	bool	CheckPetePlugin(LPCTSTR DllName);
	void	UnsafeGetSyncMods(UINT FrameCount, CParmInfoArray &ParmInfo);
};

inline bool CFFPluginEx::Create(CFFEngine& Engine, LPCTSTR Path)
{
	return(CreateFF(Engine, Path) && Launch());
}

inline bool CFFPluginEx::Launch()
{
	int	inputs = m_NumInputs;	// assume set by Create
	m_NumInputs = 0;	// spoof SetNumInputs no-op test
	return(CPlugin::Create(*m_Engine, inputs));	// launch worker thread
}

inline CFFEngine *CFFPluginEx::GetEngine()
{
	return((CFFEngine *)m_Engine);
}

inline CString CFFPluginEx::GetPath() const
{
	return(m_Path);
}

inline UINT CFFPluginEx::GetUID() const
{
	return(m_UID);
}

inline CString CFFPluginEx::GetFFName() const
{
	return(m_FFName);
}

inline UINT CFFPluginEx::GetFFUniqueID() const
{
	return(m_FFUniqueID);
}

inline CFFPlugin& CFFPluginEx::GetFFPlugin()
{
	return(m_FFPlug);
}

inline CFFInstance& CFFPluginEx::GetFFInstance()
{
	return(m_FFInst);
}

inline int CFFPluginEx::GetParmCount() const
{
	return(m_Parm.GetSize());
}

inline CString CFFPluginEx::GetParmName(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_Name);
}

inline float CFFPluginEx::GetParmVal(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_Val);
}

inline bool CFFPluginEx::GetModEnable(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_ModEnable);
}

inline int CFFPluginEx::GetModWave(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_Osc.GetWaveform());
}

inline float CFFPluginEx::GetModWaveNorm(int ParmIdx) const
{
	return(CFFParm::NormModWave(m_Parm[ParmIdx].m_Osc.GetWaveform()));
}

inline void CFFPluginEx::SetModWaveNorm(int ParmIdx, float NormWave)
{
	SetModWave(ParmIdx, CFFParm::DenormModWave(NormWave));
}

inline float CFFPluginEx::GetModFreq(int ParmIdx) const
{
	return(float(m_Parm[ParmIdx].m_ModFreq));
}

inline float CFFPluginEx::GetModPulseWidth(int ParmIdx) const
{
	return(float(m_Parm[ParmIdx].m_Osc.GetPulseWidth()));
}

inline bool CFFPluginEx::HasModRange(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_HasModRange);
}

inline CFFPlugInfo::FRANGE CFFPluginEx::GetModRange(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_ModRange);
}

inline float CFFPluginEx::GetModPhase(int ParmIdx) const
{
	return(float(m_Parm[ParmIdx].m_Osc.GetPhase()));
}

inline bool CFFPluginEx::IsModulating(int ParmIdx) const
{
	return(m_Parm[ParmIdx].m_IsModulating);
}

inline void CFFPluginEx::GetParmMidiInfo(int ParmIdx, int PropIdx, CMidiInfo& Info) const
{
	ASSERT(PropIdx >= 0 && PropIdx < PARAM_MIDI_PROPS);
	Info = m_Parm[ParmIdx].m_MidiInfo[PropIdx];
}

inline void CFFPluginEx::SetParmMidiInfo(int ParmIdx, int PropIdx, const CMidiInfo& Info)
{
	ASSERT(PropIdx >= 0 && PropIdx < PARAM_MIDI_PROPS);
	m_Parm[ParmIdx].m_MidiInfo[PropIdx] = Info;
}

inline void CFFPluginEx::GetMidiInfo(int PropIdx, CMidiInfo& Info) const
{
	ASSERT(PropIdx >= 0 && PropIdx < PLUGIN_MIDI_PROPS);
	Info = m_MidiInfo[PropIdx];
}

inline void CFFPluginEx::SetMidiInfo(int PropIdx, const CMidiInfo& Info)
{
	ASSERT(PropIdx >= 0 && PropIdx < PLUGIN_MIDI_PROPS);
	m_MidiInfo[PropIdx] = Info;
}

inline bool CFFPluginEx::GetSoloBypass() const
{
	return(m_SoloBypass);
}

inline void CFFPluginEx::SetSoloBypass(bool Enable)
{
	m_SoloBypass = Enable;
}

inline bool CFFPluginEx::IsClipPlayer() const
{
	return(m_IsClipPlayer);
}

inline CPoint CFFPluginEx::GetScrollPos(int TypeIdx) const
{
	ASSERT(TypeIdx >= 0 && TypeIdx < SCROLL_POSITION_TYPES);
	return(m_ScrollPos[TypeIdx]);
}

inline void CFFPluginEx::SetScrollPos(int TypeIdx, CPoint ScrollPos)
{
	ASSERT(TypeIdx >= 0 && TypeIdx < SCROLL_POSITION_TYPES);
	m_ScrollPos[TypeIdx] = ScrollPos;
}
