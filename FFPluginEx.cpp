// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		15sep10	add plugin helpers
		02		31mar11	in SetFrameProps, recreate helpers
		03		02may11	add thread count
		04		07may11	add get/set mod phase
		05		17may11	process copy can set output refs w/o interlock
		06		24may11	in-place case can also set refs w/o interlock
		07		01jun11	in SetInfo, create helpers after setting parameters
		08		24nov11	add scroll position
		09		05jan12	in Work, post-increment oscillator clock
		10		05jan12	add RewindOscillators
		11		10jan12	add FastCalcVal
		12		11jan12	add methods to get synchronized modulations
		13		26mar12	add DisconnectOutput
		14		26mar12	in StopWork, wait for stop
		15		13apr12	standardize thread function
		16		06may12	add extended freeframe instance to avoid reentrance
		17		07may12	clip player now returns fail if clip path won't open
		18		22may12	in GetSyncMods, handle thread stopped case
		19		31may12	in Work, use QOUTPUT to allow monitoring
		20		01jun12	move DisconnectOutput into base class

		parallel FreeFrame plugin
 
*/

#include "stdafx.h"
#include "Resource.h"
#include "FFPluginEx.h"
#include "FFEngine.h"
#include "Benchmark.h"
#include "Renderer.h"
#include "shlwapi.h"

static const UINT	FFID_PlayerFF		= MAKE_FFUNIQUEID('P', 'L', 'Y', 'R');
static const UINT	FFID_PlayerFFLB		= MAKE_FFUNIQUEID('P', 'L', 'L', 'B');
static const UINT	FFID_PeteMixer		= MAKE_FFUNIQUEID('L', 'K', 'E', 'Y');
static const UINT	FFID_PeteRadialBlur	= MAKE_FFUNIQUEID('R', 'B', 'L', 'R');
static const UINT	FFID_PeteSpiralBlur	= MAKE_FFUNIQUEID('S', 'P', 'R', 'L');
static const UINT	FFID_PeteTimeBlur	= MAKE_FFUNIQUEID('T', 'B', 'L', 'R');

enum {	// secret API between FFRend and PlayerFF
	FFREND_PLAYER_CLIP_PATH = 666,
	FFREND_PLAYER_QUIET_SET_BANK,
	FFREND_PLAYER_QUIET_SET_CLIP,
	FFREND_PLAYER_SKIP_INITIAL_OPEN,
	FFREND_PLAYER_DO_INITIAL_OPEN,
};

CFFPluginEx::CFFPluginEx()
{
	m_UID = 0;
	m_FFUniqueID = 0;
	m_SoloBypass = FALSE;
	m_IsClipPlayer = FALSE;
	m_ClipOpened = FALSE;
	ZeroMemory(m_ScrollPos, sizeof(m_ScrollPos));
}

CFFPluginEx::~CFFPluginEx()
{
	// clean up any remaining queued commands since they could contain pointers
	COMMAND	cmd;
	while (m_CmdQueue.Pop(cmd)) {
		switch (cmd.CmdID) {
		case CID_OPEN_CLIP:
			free(cmd.OpenClip.Path);	// avoid potential leak
			break;
		}
	}
	if (GetHelperCount())
		DestroyHelpers();	// before we destroy freeframe plugin
}

bool CFFPluginEx::Instantiate(CSize FrameSize, UINT FFColorDepth)
{
	if (!m_FFPlug.GetPluginCaps(FFColorDepth)) {
		m_Engine->OnError(FFERR_UNSUPP_BIT_DEPTH, m_Path);
		return(FALSE);
	}
	VideoInfoStruct	vis;
	vis.frameWidth = FrameSize.cx;
	vis.frameHeight = FrameSize.cy;
	vis.bitDepth = FFColorDepth;
	vis.orientation = FF_ORIGIN_TOP_LEFT;
	if (!m_FFInst.Create(&m_FFPlug, vis)) {
		m_Engine->OnError(FFERR_CANT_INSTANTIATE, m_Path);
		return(FALSE);
	}
	m_FFInstEx.Create(m_FFInst);
	return(TRUE);
}

bool CFFPluginEx::CreateFF(CFFEngine& Engine, LPCTSTR Path)
{
	m_Engine = &Engine;	// set engine pointer first
	m_Path = Path;
	if (!m_FFPlug.Load(Path)) {
		Engine.OnError(FFERR_CANT_LOAD_PLUGIN, Path);
		return(FALSE);
	}
	if (!m_FFPlug.GetPluginName(m_FFName)) {
		Engine.OnError(FFERR_CANT_GET_PLUGIN_NAME, Path);
		return(FALSE);
	}
	m_Name = m_FFName;	// name is freename name for now
	PlugInfoStruct	pis;
	if (!m_FFPlug.GetInfo(pis)) {
		Engine.OnError(FFERR_CANT_GET_PLUGIN_INFO, Path);
		return(FALSE);
	}
	// unique ID is char[4] with no terminator; cast to DWORD for efficiency
	m_FFUniqueID = *(DWORD *)pis.uniqueID;
	switch (m_FFUniqueID) {
	case FFID_PlayerFF:
	case FFID_PlayerFFLB:
		// if source plugin named PlayerFF* and version > 2, assume clip player
		if (pis.pluginType != FF_EFFECT	&& m_FFName.Left(8) == _T("PlayerFF")) {
			PlugExtendedInfoStruct	peis;
			if (m_FFPlug.GetExtendedInfo(peis) && peis.PluginMajorVersion >= 2) {
				m_IsClipPlayer = TRUE;
				// If we're opening a clip, tell the clip player to skip its initial
				// open; this avoids pointless overhead and unexpected entries in the
				// recent clip list. Note that to prevent the initial open attribute
				// from sticking, it must be set in either case, because the player
				// stores it in a static variable shared by all player instances.
				int	InitialOpenMsg = GetEngine()->OpeningClip() ?
					FFREND_PLAYER_SKIP_INITIAL_OPEN : FFREND_PLAYER_DO_INITIAL_OPEN;
				m_FFPlug.GetPluginCaps(InitialOpenMsg);	// misuse of capability query
			}
		}
		break;
	case FFID_PeteMixer:
		if (!CheckPetePlugin(_T("PeteMixer.dll")))
			return(FALSE);
		break;
	case FFID_PeteRadialBlur:
		if (!CheckPetePlugin(_T("PeteRadialBlur.dll")))
			return(FALSE);
		break;
	case FFID_PeteSpiralBlur:
		if (!CheckPetePlugin(_T("PeteSpiralBlur.dll")))
			return(FALSE);
		break;
	case FFID_PeteTimeBlur:
		if (!CheckPetePlugin(_T("PeteTimeBlur.dll")))
			return(FALSE);
		break;
	}
	if (!Instantiate(Engine.GetFrameSize(), Engine.GetFFColorDepth()))
		return(FALSE);
	int parms = m_FFPlug.GetNumParams();
	m_Parm.SetSize(parms);
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		CFFParm&	parm = m_Parm[ParmIdx];
		if (!m_FFPlug.GetParamName(ParmIdx, parm.m_Name)) {
			Engine.OnError(FFERR_CANT_GET_PARAM_NAME, Path);
			return(FALSE);
		}
		parm.m_Val = m_FFPlug.GetParamDefault(ParmIdx);
		float	FrameRate = GetEngine()->GetFrameRate();
		parm.m_Osc.SetTimerFreq(FrameRate);
	}
	m_Path = Path;
	static const UINT	COMMAND_QUEUE_SIZE = 64;
	m_CmdQueue.Create(COMMAND_QUEUE_SIZE);
	m_IsSource = pis.pluginType != FF_EFFECT;
	m_ProcessCopy = m_FFPlug.GetPluginCaps(FF_CAP_PROCESSFRAMECOPY) != 0;
	m_CopyPref = m_FFPlug.GetPluginCaps(FF_CAP_COPYORINPLACE);
	m_NumInputs = m_FFPlug.GetPluginCaps(FF_CAP_MAXIMUMINPUTFRAMES);
	m_NumInputs = max(m_NumInputs, 1);
	m_UID = Engine.GetUID();
	return(TRUE);
}

bool CFFPluginEx::CheckPetePlugin(LPCTSTR DllName)
{
	if (!_tcsicmp(DllName, PathFindFileName(m_Path))) {
		CFile	f;
		if (f.Open(m_Path, CFile::modeRead | CFile::shareDenyNone)) {
			CFileStatus	fstat;
			f.GetStatus(fstat);
			static const CTime	DiscoveryDate(2006, 12, 24, 0, 0, 0);
			if (fstat.m_mtime < DiscoveryDate) {	// if buggy version
				m_Engine->OnError(FFERR_BAD_PETE_PLUGIN, DllName);
				return(FALSE);
			}
		}
	}
	return(TRUE);
}

CString CFFPluginEx::GetFFUniqueIDString() const
{
	static const int FFID_LEN = 4;
	char	buf[FFID_LEN + 1];
	memcpy(buf, &m_FFUniqueID, FFID_LEN);
	buf[FFID_LEN] = 0;
	return(CString(buf));
}

void CFFPluginEx::GetParmInfo(int ParmIdx, CFFPlugInfo::FFPARM_INFO& Info) const
{
	Info.ModRange = GetModRange(ParmIdx);
	Info.Val = GetParmVal(ParmIdx);
	Info.ModEnab = GetModEnable(ParmIdx);
	Info.ModWave = static_cast<short>(GetModWave(ParmIdx));
	Info.ModFreq = GetModFreq(ParmIdx);
	Info.ModPW = GetModPulseWidth(ParmIdx);
	memcpy(Info.MidiInfo, m_Parm[ParmIdx].m_MidiInfo, 
		PARAM_MIDI_PROPS * sizeof(CMidiInfo));
}

void CFFPluginEx::SetParmInfo(int ParmIdx, const CFFPlugInfo::FFPARM_INFO& Info)
{
	SetModRange(ParmIdx, Info.ModRange);
	SetParmVal(ParmIdx, Info.Val);
	SetModEnable(ParmIdx, Info.ModEnab);
	SetModWave(ParmIdx, Info.ModWave);
	SetModFreq(ParmIdx, Info.ModFreq);
	SetModPulseWidth(ParmIdx, Info.ModPW);
	memcpy(m_Parm[ParmIdx].m_MidiInfo, Info.MidiInfo,
		PARAM_MIDI_PROPS * sizeof(CMidiInfo));
}

void CFFPluginEx::GetInfo(CFFPlugInfo& Info)
{
	Info.m_Path = m_Path;
	if (m_IsClipPlayer)
		Info.m_ClipPath = GetClipPath();
	else
		Info.m_ClipPath.Empty();
	Info.m_Bypass = m_Bypass;
	Info.m_Threads = GetThreadCount();
	int	parms = GetParmCount();
	Info.m_Parm.SetSize(parms);
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++)
		GetParmInfo(ParmIdx, Info.m_Parm[ParmIdx]);
	memcpy(Info.m_MidiInfo, m_MidiInfo, PLUGIN_MIDI_PROPS * sizeof(CMidiInfo));
}

void CFFPluginEx::SetInfo(const CFFPlugInfo& Info)
{
	m_Path = Info.m_Path;
	m_Bypass = Info.m_Bypass;
	if (m_IsClipPlayer && !Info.m_ClipPath.IsEmpty()) {
		OpenClip(Info.m_ClipPath);	// open clip by its path
		if (!IsRunning()) {	// if direct access is ok
			// set bank and clip parameters quietly, without opening a clip
			m_FFInst.SetParam(FFREND_PLAYER_QUIET_SET_BANK, Info.m_Parm[0].Val);
			m_FFInst.SetParam(FFREND_PLAYER_QUIET_SET_CLIP, Info.m_Parm[1].Val);
		}
	}
	int	parms = Info.m_Parm.GetSize();
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++)
		SetParmInfo(ParmIdx, Info.m_Parm[ParmIdx]);
	SetThreadCount(Info.m_Threads);	// create helpers after setting parameters
	memcpy(m_MidiInfo, Info.m_MidiInfo, PLUGIN_MIDI_PROPS * sizeof(CMidiInfo));
}

DWORD CFFPluginEx::GetInfo(FFPLUG_INFO*& pInfo)
{
	CFFPlugInfo	InfoOb;
	GetInfo(InfoOb);
	DWORD	Size = InfoOb.GetInfoSize();
	pInfo = (FFPLUG_INFO *)new BYTE[Size];	// caller is responsible for deleting
	InfoOb.GetInfo(*pInfo);
	return(Size);
}

void CFFPluginEx::SetInfo(const FFPLUG_INFO& Info)
{
	CFFPlugInfo	InfoOb;
	InfoOb.SetInfo(Info);
	SetInfo(InfoOb);
}

bool CFFPluginEx::SetFrameProps(CSize FrameSize, UINT FFColorDepth)
{
	ASSERT(!IsRunning());	// thread must be stopped
	LPCTSTR	ClipPath = m_IsClipPlayer ? GetClipPath() : NULL;
	if (!Instantiate(FrameSize, FFColorDepth))	// re-instantiate plugin
		return(FALSE);
	// parameters have reverted to default values and must be reset
	int	parms = GetParmCount();
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++)
		m_FFInst.SetParam(ParmIdx, m_Parm[ParmIdx].m_Val);
	if (ClipPath != NULL)	// if we're a clip player
		OpenClip(ClipPath);	// current clip must also be reset
	int	helpers = GetHelperCount();
	if (helpers)
		CreateHelpers(helpers);	// recreate helpers
	return(TRUE);
}

bool CFFPluginEx::SetFrameSize(CSize FrameSize)
{
	return(SetFrameProps(FrameSize, GetEngine()->GetFFColorDepth()));
}

bool CFFPluginEx::SetColorDepth(UINT FFColorDepth)
{
	return(SetFrameProps(GetEngine()->GetFrameSize(), FFColorDepth));
}

void CFFPluginEx::SetParmVal(int ParmIdx, float Val)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_VAL;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetVal.Val = Val;
		m_CmdQueue.Push(cmd);
	} else {	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetVal(Val);
		m_FFInst.SetParam(ParmIdx, Val);
	}
}

void CFFPluginEx::SetModEnable(int ParmIdx, bool Enable)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_MOD_ENABLE;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetModEnable.Enable = Enable;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetModEnable(Enable);
}

void CFFPluginEx::SetModWave(int ParmIdx, int Wave)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_MOD_WAVE;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetModWave.Wave = Wave;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetModWave(Wave);
}

void CFFPluginEx::SetModFreq(int ParmIdx, float Freq)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_MOD_FREQ;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetModFreq.Freq = Freq;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetModFreq(Freq, GetEngine()->GetSpeed());
}

void CFFPluginEx::SetModPulseWidth(int ParmIdx, float PW)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_MOD_PW;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetModPW.PW = PW;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetModPulseWidth(PW);
}

void CFFPluginEx::SetModRange(int ParmIdx, FRANGE Range)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_MOD_RANGE;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetModRange.Range = Range;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetModRange(Range);
}

void CFFPluginEx::SetModPhase(int ParmIdx, float Phase)
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_PARM_SET_MOD_PHASE;
		cmd.Parm.Idx = ParmIdx;
		cmd.Parm.SetModPhase.Phase = Phase;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		m_Parm[ParmIdx].SetModPhase(Phase);
}

void CFFPluginEx::UpdateSpeed()
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_UPDATE_SPEED;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		UnsafeUpdateSpeed();
}

void CFFPluginEx::UnsafeUpdateSpeed()
{
	int	parms = GetParmCount();
	float	Speed = GetEngine()->GetSpeed();
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		CFFParm&	parm = m_Parm[ParmIdx];
		parm.SetModFreq(parm.m_ModFreq, Speed);
	}
}

void CFFPluginEx::UpdateFrameRate()
{
	if (IsRunning()) {
		COMMAND	cmd;
		cmd.CmdID = CID_UPDATE_FRAME_RATE;
		m_CmdQueue.Push(cmd);
	} else	// thread stopped: safe to access shared data
		UnsafeUpdateFrameRate();
}

void CFFPluginEx::UnsafeUpdateFrameRate()
{
	int	parms = GetParmCount();
	double	FrameRate = GetEngine()->GetFrameRate();
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		CFFParm&	parm = m_Parm[ParmIdx];
		parm.m_Osc.SetTimerFreq(FrameRate);
	}
}

inline void CFFPluginEx::BeginModalCommand()
{
	GetEngine()->GetModalDoneEvent().Reset();
}

inline void CFFPluginEx::EndModalCommand()
{
	GetEngine()->GetModalDoneEvent().Set();
}

inline bool CFFPluginEx::WaitForModalDone()
{
	return(GetEngine()->WaitForModalDone());
}

bool CFFPluginEx::OpenClip(LPCTSTR Path)
{
#ifdef UNICODE
	USES_CONVERSION;
	LPCSTR	ANSIPath = W2A(Path);	// convert path string from Unicode to ANSI
#else
	LPCSTR	ANSIPath = Path;	// path string is already ANSI
#endif
	if (IsRunning()) {
		BeginModalCommand();	// reset modal state before queuing command
		COMMAND	cmd;
		cmd.CmdID = CID_OPEN_CLIP;
		cmd.OpenClip.Path = _strdup(ANSIPath);	// thread must free path string
		m_CmdQueue.Push(cmd);
		if (!WaitForModalDone())	// wait for command to complete
			return(FALSE);	// wait timed out or failed
		return(m_ClipOpened);	// wait succeeded: thread set m_ClipOpened
	} else	// thread stopped: safe to access shared data
		return(UnsafeOpenClip(ANSIPath));
}

LPCTSTR CFFPluginEx::GetClipPath()
{
	if (IsRunning()) {
		BeginModalCommand();	// reset modal state before queuing command
		COMMAND	cmd;
		cmd.CmdID = CID_GET_CLIP_PATH;
		m_CmdQueue.Push(cmd);
		if (!WaitForModalDone())	// wait for command to complete
			return(NULL);	// wait timed out or failed
		// wait succeeded: thread set m_ClipPath
	} else	// thread stopped: safe to access shared data
		m_ClipPath = UnsafeGetClipPath();
	return(m_ClipPath);
}

bool CFFPluginEx::UnsafeOpenClip(LPCSTR Path)
{
	ASSERT(m_IsClipPlayer);
	ASSERT(Path != NULL);
	float	val;
	// cast const char pointer to float
	*((DWORD *)&val) = DWORD(Path);
	return(m_FFInstEx.SetParam(FFREND_PLAYER_CLIP_PATH, val));
}

LPCSTR CFFPluginEx::UnsafeGetClipPath()
{
	ASSERT(m_IsClipPlayer);
	float	val = m_FFInst.GetParam(FFREND_PLAYER_CLIP_PATH);
	// cast float to const char pointer
	LPCSTR	Path = LPCSTR(*((DWORD *)&val));
	return(Path);
}

bool CFFPluginEx::StopWork()
{
	if (!IsRunning())
		return(TRUE);	// already stopped
	BeginModalCommand();	// reset modal state before queuing command
	COMMAND	cmd;
	cmd.CmdID = CID_STOP_WORK;
	m_CmdQueue.Push(cmd);
	if (!WaitForModalDone())	// wait for command to complete
		return(FALSE);
	return(CEngineThread::Run(FALSE));
}

bool CFFPluginEx::WaitForCommand()
{
	if (IsRunning()) {
		BeginModalCommand();	// reset modal state before queuing command
		COMMAND	cmd;
		cmd.CmdID = CID_WAIT;
		m_CmdQueue.Push(cmd);
		if (!WaitForModalDone())	// wait for command to complete
			return(FALSE);
	}
	return(TRUE);
}

void CFFPluginEx::RewindOscillators()
{
	ASSERT(!m_Engine->IsRunning());	// engine must be stopped for stable frame counts
	UINT	FramesRendered = m_Engine->GetRenderer().GetFrameCounter();
	// our frame count is short by one, because on engine stop, Work exits via
	// queue read/write before incrementing our frame count; add an extra frame
	int	FramesPending = m_FrameCounter - FramesRendered + 1;
	int	parms = GetParmCount();
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		CFFParm&	parm = m_Parm[ParmIdx];
		if (parm.m_IsModulating) {
			double	PrevClock = parm.m_Osc.GetClock() - FramesPending;
			parm.m_Osc.SetClock(PrevClock);
			//parm.CalcVal();	// rewind UI parameter value to match
		}
	}
}

void CFFPluginEx::GetSyncMods(UINT FrameCount, CParmInfoArray& ParmInfo)
{
	if (IsRunning()) {	// if our thread is running
		COMMAND	cmd;
		cmd.CmdID = CID_GET_SYNC_MODS;
		cmd.GetSyncMods.FrameCount = FrameCount;	// target frame count for modulations
		cmd.GetSyncMods.ParmInfo = &ParmInfo;	// pointer to destination parameter info
		m_CmdQueue.Push(cmd);	// post request for synchronized modulations
	} else	// our thread is stopped; we can't render, or engine is stopped or paused
		UnsafeGetSyncMods(FrameCount, ParmInfo);	// access members directly
}

void CFFPluginEx::UnsafeGetSyncMods(UINT FrameCount, CParmInfoArray& ParmInfo)
{
	// access is only safe if we're stopped, or from our worker thread
	ASSERT(!IsRunning() || GetCurrentThreadId() == GetID());
	int	FramesPending;
	if (m_Engine->IsRunning()) {	// if engine is running
		// oscillators are one clock ahead of current frame, due to post-increment
		FramesPending = m_FrameCounter - FrameCount + 1;	// add extra clock
		if (m_Engine->IsPaused())	// if engine is paused
			FramesPending++;	// add another clock because Work pauses in queue
								// read/write before incrementing our frame count
	} else {	// engine is stopped
		// engine stop calls RewindOscillators so oscillators are already in sync,
		// but they're still a clock ahead of current frame due to post-increment
		FramesPending = 1;	// rewind by one clock
	}
	// rewind modulated parameters so they correspond to last rendered frame
	int	parms = GetParmCount();
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		CFFParm&	parm = m_Parm[ParmIdx];
		if (parm.m_IsModulating) {
			CFFParm	tp;	// only copy members needed by CalcVal
			tp.m_Osc = parm.m_Osc;	// copy oscillator; binary copy is fine
			tp.m_HasModRange = parm.m_HasModRange;
			tp.m_ModRange = parm.m_ModRange;
			double	PrevClock = tp.m_Osc.GetClock() - FramesPending;
			PrevClock = max(PrevClock, 0);	// in case oscillators were reset
			tp.m_Osc.SetClock(PrevClock);
			ParmInfo[ParmIdx].Val = tp.CalcVal();
		}
	}
	GetEngine()->AcknowledgeBroadcast();
}

void CFFPluginEx::AttachHelper()
{
	m_Helper[0].GetFFInstance().Attach(m_FFInst);
	m_FFInstEx.Create(m_FFInst, TRUE);	// bypass to avoid reentrance
}

void CFFPluginEx::DetachHelper()
{
	m_Helper[0].GetFFInstance().Detach();
	m_FFInstEx.Create(m_FFInst);
}

UINT CPlugin::ThreadFunc(LPVOID Arg)
{
	ENGINE_THREAD_INIT(CFFPluginEx);
	bool	UseTime;
	UINT	RandSeed = pThread->GetEngine()->GetRandomSeed(UseTime);
	if (UseTime) {
		// other plugins may be launched this tick; differentiate via thread ID
		srand(GetTickCount() + pThread->GetThreadID());
	} else {
		if (RandSeed != 1)	// if default seed, don't waste time setting it
			srand(RandSeed);
	}
	while (pThread->Work());
	ENGINE_THREAD_EXIT();
}

bool CFFPluginEx::Work()
{
	int	FrameLength = m_Engine->GetFrameLength();
	while (1) {
		COMMAND	Cmd;
		while (m_CmdQueue.Pop(Cmd)) {
			switch (Cmd.CmdID) {
			case CID_PARM_SET_VAL:
				m_Parm[Cmd.Parm.Idx].SetVal(Cmd.Parm.SetVal.Val);
				m_FFInstEx.SetParam(Cmd.Parm.Idx, Cmd.Parm.SetVal.Val);
				break;
			case CID_PARM_SET_MOD_ENABLE:
				m_Parm[Cmd.Parm.Idx].SetModEnable(Cmd.Parm.SetModEnable.Enable);
				break;
			case CID_PARM_SET_MOD_WAVE:
				m_Parm[Cmd.Parm.Idx].SetModWave(Cmd.Parm.SetModWave.Wave);
				break;
			case CID_PARM_SET_MOD_FREQ:
				m_Parm[Cmd.Parm.Idx].SetModFreq(Cmd.Parm.SetModFreq.Freq, 
					GetEngine()->GetSpeed());
				break;
			case CID_PARM_SET_MOD_PW:
				m_Parm[Cmd.Parm.Idx].SetModPulseWidth(Cmd.Parm.SetModPW.PW);
				break;
			case CID_PARM_SET_MOD_RANGE:
				m_Parm[Cmd.Parm.Idx].SetModRange(Cmd.Parm.SetModRange.Range);
				break;
			case CID_PARM_SET_MOD_PHASE:
				m_Parm[Cmd.Parm.Idx].SetModPhase(Cmd.Parm.SetModPhase.Phase);
				break;
			case CID_UPDATE_SPEED:
				UnsafeUpdateSpeed();
				break;
			case CID_UPDATE_FRAME_RATE:
				UnsafeUpdateFrameRate();
				break;
			case CID_OPEN_CLIP:
				m_ClipOpened = UnsafeOpenClip(Cmd.OpenClip.Path);
				free(Cmd.OpenClip.Path);	// string was allocated via strdup
				EndModalCommand();
				break;
			case CID_GET_CLIP_PATH:
				m_ClipPath = UnsafeGetClipPath();
				EndModalCommand();
				break;
			case CID_STOP_WORK:
				EndModalCommand();
				return(WaitForStart());
			case CID_WAIT:
				EndModalCommand();
				break;
			case CID_GET_SYNC_MODS:
				UnsafeGetSyncMods(Cmd.GetSyncMods.FrameCount, *Cmd.GetSyncMods.ParmInfo);
				break;
			default:
				NODEFAULTCASE;
			}
		}
		int	parms = m_Parm.GetSize();
		for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
			CFFParm&	parm = m_Parm[ParmIdx];
			if (parm.m_IsModulating) {	// if parameter is being modulated
				float	ParmVal = parm.FastCalcVal();	// update parameter value
				m_FFInstEx.SetParam(ParmIdx, ParmVal);	// set freeframe parameter
				parm.m_Osc.TimerHook();	// post-increment oscillator clock
			}
		}
		if (m_UseProcessCopy) {	// if using process copy
			// read input frames
			int	InpIdx;
			for (InpIdx = 0; InpIdx < m_NumInputs; InpIdx++) {
				CInput&	inp = m_Input[InpIdx];
				if (inp.m_HasOutput) {	// if input is connected
					QREAD(&m_InputQueue[InpIdx], inp.m_Frame);
				} else {	// null input
					QREAD(m_Engine->GetFreeQueue(), inp.m_Frame);
					inp.m_Frame->RefCount = 1;
					ZeroMemory(inp.m_Frame->Buf, FrameLength);
					if (inp.m_PrimeFrames) {	// if feedback priming
						inp.m_PrimeFrames--;
						if (!inp.m_PrimeFrames)
							inp.m_HasOutput = TRUE;
					}
				}
				m_InFrameBuf[InpIdx] = inp.m_Frame->Buf;
			}
			if (GetHelperCount()) {
				if (!Delegate())	// delegate to helpers
					return(!m_KillFlag);
			} else {	// no helpers
				QREAD(m_Engine->GetFreeQueue(), m_OutFrame);	// get output frame
				if (!m_Bypass) {	// if not bypassed
					ProcessFrameCopyStruct	pfcs;
					pfcs.InputFrames = m_InFrameBuf.GetData();
					pfcs.numInputFrames = m_NumInputs;
					pfcs.OutputFrame = m_OutFrame->Buf;
					AddProcessHistorySample(TRUE);
					m_FFInst.ProcessFrameCopy(pfcs);	// process frame
					AddProcessHistorySample(FALSE);
				} else	// bypassed; copy first input to output
					memcpy(m_OutFrame->Buf, m_InFrameBuf[0], FrameLength);
				// write output frame to output queues
				int	outs = m_OutputQueue.GetSize();
				ASSERT(outs > 0);	// if no outputs, we shouldn't be running
				m_OutFrame->RefCount = outs;	// set output refs
				for (int OutIdx = 0; OutIdx < outs; OutIdx++) {
					QOUTPUT(m_OutputQueue[OutIdx], m_OutFrame);
				}
				m_OutFrame = NULL;
				// free input frames
				for (InpIdx = 0; InpIdx < m_NumInputs; InpIdx++) {
					CInput&	inp = m_Input[InpIdx];
					if (!InterlockedDecrement(&inp.m_Frame->RefCount)) {
						QWRITE(m_Engine->GetFreeQueue(), inp.m_Frame);
					}
					inp.m_Frame = NULL;	// for monitoring
				}
			}
		} else {	// in place
			// read input frame
			CInput&	inp = m_Input[0];	// single input only
			if (inp.m_HasOutput) {	// if input is connected
				QREAD(&m_InputQueue[0], inp.m_Frame);
				if (inp.m_Frame->RefCount > 1) {	// if frame is shared
					PFRAME	Source = inp.m_Frame;
					QREAD(m_Engine->GetFreeQueue(), inp.m_Frame);
					memcpy(&inp.m_Frame->Buf, &Source->Buf, FrameLength);	// copy frame
					if (!InterlockedDecrement(&Source->RefCount)) {
						QWRITE(m_Engine->GetFreeQueue(), Source);
					}
				}
			} else {	// null input
				QREAD(m_Engine->GetFreeQueue(), inp.m_Frame);
				ZeroMemory(inp.m_Frame->Buf, FrameLength);
				if (inp.m_PrimeFrames) {	// if feedback priming
					inp.m_PrimeFrames--;
					if (!inp.m_PrimeFrames)
						inp.m_HasOutput = TRUE;
				}
			}
			if (GetHelperCount()) {
				if (!Delegate())	// delegate to helpers
					return(!m_KillFlag);
			} else {
				if (!m_Bypass) {	// if not bypassed
					AddProcessHistorySample(TRUE);
					m_FFInst.ProcessFrame(inp.m_Frame->Buf);	// process frame
					AddProcessHistorySample(FALSE);
				}
				int	outs = m_OutputQueue.GetSize();
				ASSERT(outs > 0);	// if no outputs, we shouldn't be running
				inp.m_Frame->RefCount = outs;	// set output refs
				for (int OutIdx = 0; OutIdx < outs; OutIdx++) {
					QOUTPUT(m_OutputQueue[OutIdx], inp.m_Frame);
				}
				inp.m_Frame = NULL;	// for accurate monitoring
			}
		}
		m_FrameCounter++;
	}
}
