// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      02jun10	initial version
        01      06may12	add monitor source
		02		23may12	make monitor source a slot index
        03      01jun12	add events; use own timer

		automated undo test
 
*/

#include "stdafx.h"
#include "FFRend.h"
#include "UndoTest.h"
#include "UndoCodes.h"
#include "FFRendDoc.h"
#include "FFRendView.h"

static CUndoTest gUndoTest(1);	// one and only instance, initially running

const LPCTSTR CUndoTest::m_TestPlug[] = {
	_T("C:\\temp\\ff plugins\\whorld\\Radar.dll"),
	_T("C:\\temp\\ff plugins\\whorld\\WhorldFF.dll"),
	_T("C:\\temp\\ff plugins\\whorld\\PlayerFF.dll"),
	_T("C:\\temp\\ff plugins\\whorld\\ContrastPS.dll"),
	_T("C:\\temp\\ff plugins\\whorld\\BoolMix.dll"),
	_T("C:\\temp\\ff plugins\\whorld\\CharGen.dll"),
	_T("C:\\temp\\ff plugins\\whorld\\Invert.dll"),
};
const int CUndoTest::m_TestPlugCount = sizeof(m_TestPlug) / sizeof(LPCTSTR);

const LPCTSTR CUndoTest::m_TestClip[] = {
	_T("C:\\temp\\avi files\\01_24_04-med.avi"),
	_T("C:\\temp\\avi files\\Boat Ride to Punta Sal (xvid).avi"),
	_T("C:\\temp\\avi files\\earth1.avi"),
	_T("C:\\temp\\avi files\\kissinggirls.avi"),
	_T("C:\\temp\\avi files\\Night Traffic.avi"),
	_T("C:\\temp\\avi files\\tint.avi"),
};
const int CUndoTest::m_TestClipCount = sizeof(m_TestClip) / sizeof(LPCTSTR);

enum {	// events
	EVT_FIRST = 0x3fffffff,
	EVT_PAUSE,
	EVT_FILES_BAR,
	EVT_GRAPH_BAR,
	EVT_HISTORY_BAR,
	EVT_LOAD_BALANCE_BAR,
	EVT_PATCH_BAR,
	EVT_METAPARAMS_BAR,
	EVT_MIDI_SETUP_BAR,
	EVT_MONITOR_BAR,
	EVT_QUEUES_BAR,
};

const CUndoTest::EDIT_INFO CUndoTest::m_EditInfo[] = {
	{UCODE_SELECTION,			3},
	{UCODE_PARM,				3},
	{UCODE_MOD_ENABLE,			2},
	{UCODE_MOD_WAVE,			3},
	{UCODE_MOD_FREQ,			3},
	{UCODE_MOD_PW,				2},
	{UCODE_MOD_RANGE,			2},
	{UCODE_BYPASS,				2},
	{UCODE_SOLO,				1},
	{UCODE_CUT,					2},
	{UCODE_PASTE,				2},
	{UCODE_INSERT,				2},
	{UCODE_INSERT_EMPTY,		1},
	{UCODE_DELETE,				2},
	{UCODE_LOAD,				2},
	{UCODE_UNLOAD,				2},
	{UCODE_MOVE,				2},
	{UCODE_CONNECT,				2},
	{UCODE_OPEN_CLIP,			2},
	{UCODE_MASTER_SPEED,		.5f},
	{UCODE_MIDI_SETUP,			3},
	{UCODE_METAPARM_VALUE,		3},
	{UCODE_METAPARM_INSERT,		3},
	{UCODE_METAPARM_DELETE,		3},
	{UCODE_METAPARM_DRAG,		3},
	{UCODE_METAPARM_PROPS,		3},
	{UCODE_METAPARM_GROUP,		0},
	{UCODE_METAPARM_UNGROUP,	0},
	{UCODE_LOAD_BALANCE,		2},
	{UCODE_SYNC_OSCILLATORS,	1},
	{UCODE_MONITOR_SOURCE,		3},
	{EVT_PAUSE,					2},
	{EVT_FILES_BAR,				.1f},
	{EVT_GRAPH_BAR,				.1f},
	{EVT_HISTORY_BAR,			.1f},
	{EVT_LOAD_BALANCE_BAR,		.1f},
	{EVT_PATCH_BAR,				.1f},
	{EVT_METAPARAMS_BAR,		.1f},
	{EVT_MIDI_SETUP_BAR,		.1f},
	{EVT_MONITOR_BAR,			2},
	{EVT_QUEUES_BAR,			.1f},
};
const int CUndoTest::m_EditInfoCount = sizeof(m_EditInfo) / sizeof(EDIT_INFO);

const LPCTSTR CUndoTest::m_StateName[STATES] = {
	_T("Stop"), 
	_T("Edit"), 
	_T("Undo"), 
	_T("Redo"),
};

#define LOG_TO_FILE 1
#if LOG_TO_FILE
#define PRINTF LogPrintf
#else
#if UNDO_NATTER
#define PRINTF _tprintf
#else
#define PRINTF sizeof
#endif
#endif

CUndoTest::CUndoTest(bool InitRunning)
{
	Init();
	m_InitRunning = InitRunning;
	SetTimer(NULL, 0, TIMER_PERIOD, TimerProc);
}

CUndoTest::~CUndoTest()
{
}

void CUndoTest::Init()
{
	m_InitRunning = FALSE;
	m_LogFile = NULL;
	m_Main = NULL;
	m_View = NULL;
	m_Engine = NULL;
	m_UndoMgr = NULL;
	m_Metaparm = NULL;
	m_State = STOP;
	m_Passes = 20;
	m_PassEdits = 250;
	m_PassUndos = 50;
	m_MaxEdits = INT_MAX;
	m_MaxPlugins = 50;
	m_PassesDone = 0;
	m_EditsDone = 0;
	m_UndosToDo = 0;
	m_UndosDone = 0;
	m_StepsDone = 0;
	m_RandSeed = 666;
	m_Clock = 0;
}

int CUndoTest::LogPrintf(LPCTSTR Format, ...)
{
	if (m_LogFile == NULL)
		return(-1);
	va_list arglist;
	va_start(arglist, Format);
	int	retc = _vftprintf(m_LogFile, Format, arglist);
	va_end(arglist);
	fflush(m_LogFile);
	return(retc);
}

int	CUndoTest::Random(int Vals)
{
	if (Vals <= 0)
		return(-1);
	int	i = trunc(rand() / double(RAND_MAX) * Vals);
	return(min(i, Vals - 1));
}

int CUndoTest::GetRandEdit() const
{
	return(m_UndoCode[Random(m_UndoCode.GetSize())]);
}

int CUndoTest::GetRandSlot() const
{
	return(Random(m_Engine->GetSlotCount()));
}

int CUndoTest::GetRandInsertSlot() const
{
	return(Random(m_Engine->GetSlotCount() + 1));
}

int CUndoTest::GetRandLoadedSlot() const
{
	int	plugs = m_Engine->GetPluginCount();
	if (!plugs)
		return(-1);
	return(m_Engine->PluginToSlot(Random(plugs)));
}

bool CUndoTest::GetRandMonitorSlot(int& SlotIdx) const
{
	int	plugs = m_Engine->GetPluginCount();
	if (!plugs)
		return(FALSE);
	SlotIdx = m_Engine->MonitorPluginToSlot(Random(plugs + 1) - 1);
	return(TRUE);
}

int CUndoTest::GetRandEffectSlot() const
{
	int	plugs = m_Engine->GetPluginCount();
	CArrayEx<int, int>	effect;
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		if (!m_Engine->GetPlugin(PlugIdx).IsSource())
			effect.Add(PlugIdx);
	}
	if (!effect.GetSize())
		return(-1);	// no effect plugins
	return(m_Engine->PluginToSlot(effect[Random(effect.GetSize())]));
}

LPCTSTR	CUndoTest::GetRandPlugin() const
{
	return(m_TestPlug[Random(m_TestPlugCount)]);
}

LPCTSTR	CUndoTest::GetRandClip() const
{
	return(m_TestClip[Random(m_TestClipCount)]);
}

int CUndoTest::GetRandParm(int SlotIdx) const
{
	int	parms = m_Engine->GetSlot(SlotIdx)->GetParmCount();
	return(Random(parms));
}

float CUndoTest::GetRandNormVal() const
{
	return(float(round(rand() / double(RAND_MAX))));
}

int CUndoTest::GetRandMetaparm() const
{
	return(Random(m_Metaparm->GetRows()));
}

int CUndoTest::GetRandInsertMetaparm() const
{
	return(Random(m_Metaparm->GetRows() + 1));
}

int CUndoTest::GetRandAssignedMetaparm() const
{
	int	parms = m_Metaparm->GetRows();
	CArrayEx<int, int>	amp;
	for (int ParmIdx = 0; ParmIdx < parms; ParmIdx++) {
		if (m_Metaparm->GetMetaparm(ParmIdx).IsAssigned())
			amp.Add(ParmIdx);
	}
	if (!amp.GetSize())
		return(-1);	// no assigned metaparameters
	return(amp[Random(amp.GetSize())]);
}

bool CUndoTest::GetRandMetaparmTarget(CMetaparm::TARGET& Target) const
{
	int	SlotIdx;
	if ((SlotIdx = GetRandLoadedSlot()) < 0)
		return(FALSE);
	Target.SlotIdx = SlotIdx;
	Target.ParmIdx = GetRandParm(SlotIdx);
	Target.PropIdx = Random(PARAM_MIDI_PROPS);
	return(TRUE);
}

void CUndoTest::ShowBar(CMySizingControlBar& Bar, int Show)
{
	if (Show < 0)
		Show = !Bar.FastIsVisible();
	CString	name;
	Bar.GetWindowText(name);
	PRINTF(_T("%s Bar %d\n"), name, Show);
	m_Main->ShowControlBar(&Bar, Show, 0);
}

#define SHOW_BAR(bar, show) ShowBar(m_Main->Get##bar(), show);

void CUndoTest::InitBars()
{
	SHOW_BAR(FilesBar, 0);
	SHOW_BAR(GraphBar, 0);
	SHOW_BAR(HistoryBar, 0);
	SHOW_BAR(LoadBalanceBar, 0);
	SHOW_BAR(MetaparmBar, 0);
	SHOW_BAR(MidiSetupBar, 0);
	SHOW_BAR(MonitorBar, 0);
	SHOW_BAR(PatchBar, 0);
	SHOW_BAR(QueueBar, 0);
}

#define TOGGLE_BAR(tag, bar) case EVT_##tag##_BAR: ShowBar(m_Main->Get##bar()); return(EVENT);

int CUndoTest::ApplyEdit(int UndoCode)
{
	int	SlotIdx, ParmIdx, PropIdx, MetaparmIdx, Src, Dst, InpIdx;
	bool	Enab;
	LPCTSTR	Path;
	switch (UndoCode) {
	case UCODE_SELECTION:
		if ((SlotIdx = GetRandSlot()) < 0)
			return(DISABLED);
		PRINTF(_T("SetCurSel %d\n"), SlotIdx);
		m_Engine->SetCurSel(SlotIdx);
		break;
	case UCODE_PARM:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		ParmIdx = GetRandParm(SlotIdx);
		PRINTF(_T("SetParmVal %d,%d\n"), SlotIdx, ParmIdx);
		m_View->SetParmVal(SlotIdx, ParmIdx, GetRandNormVal());
		break;
	case UCODE_MOD_ENABLE:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		ParmIdx = GetRandParm(SlotIdx);
		PRINTF(_T("SetModEnable %d,%d\n"), SlotIdx, ParmIdx);
		m_View->SetModEnable(SlotIdx, ParmIdx, Random(2) != 0);
		break;
	case UCODE_MOD_WAVE:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		ParmIdx = GetRandParm(SlotIdx);
		PRINTF(_T("SetModWave %d,%d\n"), SlotIdx, ParmIdx);
		m_View->SetModWave(SlotIdx, ParmIdx, Random(COscillator::WAVEFORMS));
		break;
	case UCODE_MOD_FREQ:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		ParmIdx = GetRandParm(SlotIdx);
		PRINTF(_T("SetModFreq %d,%d\n"), SlotIdx, ParmIdx);
		m_View->SetModFreq(SlotIdx, ParmIdx, GetRandNormVal());
		break;
	case UCODE_MOD_PW:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		ParmIdx = GetRandParm(SlotIdx);
		PRINTF(_T("SetModPulseWidth %d,%d\n"), SlotIdx, ParmIdx);
		m_View->SetModPulseWidth(SlotIdx, ParmIdx, GetRandNormVal());
		break;
	case UCODE_MOD_RANGE:
		{
			if ((SlotIdx = GetRandLoadedSlot()) < 0)
				return(DISABLED);
			ParmIdx = GetRandParm(SlotIdx);
			PRINTF(_T("SetModRange %d,%d\n"), SlotIdx, ParmIdx);
			CFFParm::FRANGE	range = {GetRandNormVal(), GetRandNormVal()};
			m_View->SetModRange(SlotIdx, ParmIdx, range);
		}
		break;
	case UCODE_BYPASS:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		PRINTF(_T("Bypass %d\n"), SlotIdx);
		m_View->Bypass(SlotIdx, !m_Engine->GetBypass(SlotIdx));
		break;
	case UCODE_SOLO:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		PRINTF(_T("Solo %d\n"), SlotIdx);
		m_View->Solo(SlotIdx);
		break;
	case UCODE_CUT:
		if ((SlotIdx = GetRandSlot()) < 0)
			return(DISABLED);
		PRINTF(_T("Cut %d\n"), SlotIdx);
		if (!m_View->Cut(SlotIdx))
			return(ABORT);
		break;
	case UCODE_PASTE:
		if (!m_View->CanPaste())
			return(DISABLED);
		SlotIdx = GetRandInsertSlot();
		PRINTF(_T("Paste %d\n"), SlotIdx);
		if (!m_View->Paste(SlotIdx))
			return(ABORT);
		break;
	case UCODE_INSERT:
		SlotIdx = GetRandInsertSlot();
		Path = GetRandPlugin();
		PRINTF(_T("Insert %d '%s'\n"), SlotIdx, Path);
		if (!m_View->Insert(SlotIdx, Path))
			return(ABORT);
		break;
	case UCODE_INSERT_EMPTY:
		SlotIdx = GetRandInsertSlot();
		PRINTF(_T("InsertEmpty %d\n"), SlotIdx);
		if (!m_View->InsertEmpty(SlotIdx))
			return(ABORT);
		break;
	case UCODE_DELETE:
		if ((SlotIdx = GetRandSlot()) < 0)
			return(DISABLED);
		PRINTF(_T("Delete %d\n"), SlotIdx);
		if (!m_View->Delete(SlotIdx))
			return(ABORT);
		break;
	case UCODE_LOAD:
		if ((SlotIdx = GetRandSlot()) < 0)
			return(DISABLED);
		Path = GetRandPlugin();
		PRINTF(_T("Load %d '%s'\n"), SlotIdx, Path);
		if (!m_View->Load(SlotIdx, Path))
			return(ABORT);
		break;
	case UCODE_UNLOAD:
		if ((SlotIdx = GetRandSlot()) < 0)
			return(DISABLED);
		PRINTF(_T("Unload %d\n"), SlotIdx);
		if (!m_View->Unload(GetRandSlot()))
			return(ABORT);
		break;
	case UCODE_MOVE:
		if (m_Engine->GetSlotCount() < 2)	// need at least two slots
			return(DISABLED);
		Src = GetRandSlot();
		do {
			Dst = GetRandSlot();
		} while (Dst == Src);	// source and destination must differ
		PRINTF(_T("Move %d %d\n"), Src, Dst);
		if (!m_View->Move(Src, Dst))
			return(ABORT);
		break;
	case UCODE_CONNECT:
		if ((Dst = GetRandEffectSlot()) < 0)
			return(DISABLED);
		// constrain source < destination to avoid feedback
		Src = Random(m_Engine->GetSlot(Dst)->GetPluginIdx());
		InpIdx = Random(m_Engine->GetSlot(Dst)->GetNumInputs());
		if (Src == m_Engine->GetSlot(Dst)->GetInputPlugin(InpIdx))
			return(DISABLED);	// already connected
		PRINTF(_T("Connect %d -> %d:%d\n"), Src, Dst, InpIdx);
		if (!m_View->Connect(Src, Dst, InpIdx))
			return(ABORT);
		break;
	case UCODE_OPEN_CLIP:
		SlotIdx = GetRandSlot();
		Path = GetRandClip();
		PRINTF(_T("OpenClip %d '%s'\n"), SlotIdx, Path);
		if (!m_Main->OpenClip(SlotIdx, Path))
			return(ABORT);
		break;
	case UCODE_MASTER_SPEED:
		m_Main->NotifyEdit(0, UCODE_MASTER_SPEED, CUndoable::UE_COALESCE);
		m_Engine->SetSpeedNorm(float(Random(1000)) / 1000);
		break;
	case UCODE_MIDI_SETUP:
		{
			if ((SlotIdx = GetRandLoadedSlot()) < 0)
				return(DISABLED);
			// our method isn't undoable, so explicit notify is required
			theApp.GetMain()->GetUndoManager().NotifyEdit(
				0, UCODE_MIDI_SETUP, CUndoable::UE_COALESCE);
			CMidiInfo	info;
			info.m_Event = WORD(Random(MIDI_EVENT_TYPES));
			info.m_Chan = BYTE(Random(MIDI_CHANS));
			info.m_Ctrl = BYTE(Random(MIDI_PARMS));
			ParmIdx = GetRandParm(SlotIdx);
			PropIdx = Random(PARAM_MIDI_PROPS);
			PRINTF(_T("MIDISetup %d %d %d\n"), SlotIdx, ParmIdx, PropIdx);
			m_Engine->AssignMidi(SlotIdx, ParmIdx, PropIdx, info);
			CMidiSetupBar&	msb = m_Main->GetMidiSetupBar();
			if (msb.FastIsVisible()) {	// if bar is visible, update UI
				int	PlugIdx = m_Engine->SlotToPlugin(SlotIdx);
				if (msb.GetDlg().GetTabSel() != PlugIdx)	// if tab changed
					msb.GetDlg().SelectTab(PlugIdx);
				else	// same tab
					msb.GetDlg().UpdateMidiInfo();
			}
		}
		break;
	case UCODE_METAPARM_VALUE:
		if ((MetaparmIdx = GetRandAssignedMetaparm()) < 0)
			return(DISABLED);
		PRINTF(_T("MetaparmValue %d\n"), MetaparmIdx);
		m_Metaparm->OnRowValChange(MetaparmIdx, GetRandNormVal());
		break;
	case UCODE_METAPARM_INSERT:
		{
			CMetaparm::TARGET	targ;
			if (!GetRandMetaparmTarget(targ))
				return(DISABLED);
			PRINTF(_T("MetaparmAdd %d %d %d\n"), 
				targ.SlotIdx, targ.ParmIdx, targ.PropIdx);
			m_Metaparm->Add(targ);
		}
		break;
	case UCODE_METAPARM_DELETE:
		if ((MetaparmIdx = GetRandMetaparm()) < 0)
			return(DISABLED);
		PRINTF(_T("MetaparmDelete %d\n"), MetaparmIdx);
		// our method isn't undoable, so explicit notify is required
		m_UndoMgr->NotifyEdit(WORD(MetaparmIdx), UCODE_METAPARM_DELETE);
		m_Metaparm->Delete(MetaparmIdx);
		break;
	case UCODE_METAPARM_DRAG:
		if (m_Metaparm->GetRows() < 2)	// need at least two parms
			return(DISABLED);
		Src = GetRandMetaparm();
		do {
			Dst = GetRandMetaparm();
		} while (Dst == Src);	// source and destination must differ
		PRINTF(_T("MetaparmDrag %d %d\n"), Src, Dst);
		m_Metaparm->OnDrop(Src, Dst);	// undoable; Move method isn't
		break;
	case UCODE_METAPARM_PROPS:
		{
			if ((MetaparmIdx = GetRandMetaparm()) < 0)
				return(DISABLED);
			CMetaparm	parm(m_Metaparm->GetMetaparm(MetaparmIdx));
			if (!GetRandMetaparmTarget(parm.m_Target))
				return(DISABLED);
			PRINTF(_T("MetaparmProps %d\n"), MetaparmIdx);
			// our method isn't undoable, so explicit notify is required
			m_UndoMgr->NotifyEdit(WORD(MetaparmIdx), UCODE_METAPARM_PROPS);
			m_Metaparm->SetMetaparm(MetaparmIdx, parm);
		}
		break;
	case UCODE_METAPARM_GROUP:
	case UCODE_METAPARM_UNGROUP:
		return(DISABLED);
	case UCODE_LOAD_BALANCE:
		if ((SlotIdx = GetRandLoadedSlot()) < 0)
			return(DISABLED);
		Dst = Random(3) + 1;
		PRINTF(_T("LoadBalance %d %d\n"), SlotIdx, Dst);
		m_UndoMgr->NotifyEdit(WORD(SlotIdx), UCODE_LOAD_BALANCE);
		m_Engine->GetSlot(SlotIdx)->SetThreadCount(Dst);
		break;
	case UCODE_SYNC_OSCILLATORS:
		PRINTF(_T("SyncOscillators\n"));
		m_View->SyncOscillators(TRUE);	// undoable
		break;
	case UCODE_MONITOR_SOURCE:
		if (!GetRandMonitorSlot(SlotIdx))
			return(DISABLED);
		PRINTF(_T("MonitorSource %d\n"), SlotIdx);
		m_View->SetMonitorSource(SlotIdx);
		break;
	case EVT_PAUSE:
		Enab = !m_Main->IsPaused();
		PRINTF(_T("Pause %d\n"), Enab);
		m_Main->Pause(Enab);
		return(EVENT);
	TOGGLE_BAR(FILES, FilesBar)
	TOGGLE_BAR(GRAPH, GraphBar)
	TOGGLE_BAR(HISTORY, HistoryBar)
	TOGGLE_BAR(LOAD_BALANCE, LoadBalanceBar)
	TOGGLE_BAR(METAPARAMS, MetaparmBar)
	TOGGLE_BAR(MIDI_SETUP, MidiSetupBar)
	TOGGLE_BAR(MONITOR, MonitorBar)
	TOGGLE_BAR(PATCH, PatchBar)
	TOGGLE_BAR(QUEUES, QueueBar)
	default:
		NODEFAULTCASE;
	}
	return(SUCCESS);
}

bool CUndoTest::Create()
{
	ASSERT(m_Main == NULL);
	m_Main = theApp.GetMain();
	m_View = m_Main->GetView();
	m_Engine = &theApp.GetEngine();
	m_UndoMgr = &m_Main->GetUndoManager();
	m_Metaparm = &m_Main->GetMetaparmBar(); 
	InitBars();
	if (!m_ProgressDlg.Create())
		return(FALSE);
	m_Main->EnableWindow();	// reenable parent window
	int	Steps;
	if (m_Passes > 1)
		Steps = (m_Passes - 1) * (m_PassEdits + m_PassUndos * 2);
	else
		Steps = 0;
	Steps += m_PassEdits + m_PassEdits * m_Passes;
	m_ProgressDlg.SetRange(0, Steps);
#if LOG_TO_FILE
	CString	LogName;
	LogName.Format(_T("UndoTest%s.log"),
		CTime::GetCurrentTime().Format(_T("_%Y_%m_%d_%H_%M_%S")));
	m_LogFile = _tfopen(LogName, _T("wc"));	// commit flag
#endif
	return(TRUE);
}

void CUndoTest::Destroy()
{
	if (m_LogFile != NULL)
		fclose(m_LogFile);
	Init();	// reset defaults
}

void CUndoTest::SetState(int State)
{
	if (State == m_State)
		return;
	CString	s;
	s.Format(_T("Undo Test - Pass %d of %d - %s"), 
		m_PassesDone + 1, m_Passes, m_StateName[State]);
	m_ProgressDlg.SetWindowText(s);
	m_State = State;
}

bool CUndoTest::Run(bool Enable)
{
	if (Enable == IsRunning())
		return(TRUE);
	if (Enable) {	// if running
		if (!Create())
			return(FALSE);
		srand(m_RandSeed);
		// build array of undo codes
		m_UndoCode.RemoveAll();
		for (int i = 0; i < m_EditInfoCount; i++) {
			// set probability of edits by duplicating them
			int	dups = round(m_EditInfo[i].Probability * 10);
			for (int j = 0; j < dups; j++)
				m_UndoCode.Add(m_EditInfo[i].UndoCode);
		}
		SetState(EDIT);
	} else {	// stopping
		SetState(STOP);
		m_ProgressDlg.DestroyWindow();
		if (!m_ProgressDlg.Canceled()) {
			if (!m_Engine->GetSlotCount()) {
				CString	s;
				bool	pass = m_PassesDone >= m_Passes;
				s.Format(_T("UndoTest %s: seed=%d edits=%d passes=%d"),
					pass ? _T("pass") : _T("FAIL"),
					m_RandSeed, m_EditsDone, m_PassesDone);
				PRINTF(_T("%s\n"), s);
				AfxMessageBox(s, pass ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
			} else {
				AfxMessageBox(_T("Initial and final states don't match."));
			}
		}
		Destroy();
	}
	return(TRUE);
}

bool CUndoTest::DoRandEvent(int SeqNum)
{
	int	code = GetRandEdit();
	if (code < EVT_FIRST)
		return(FALSE);
	PRINTF(_T("%d: "), SeqNum);
	return(ApplyEdit(code) == EVENT);
}

VOID CALLBACK CUndoTest::TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	gUndoTest.OnTimer();
}

void CUndoTest::OnTimer()
{
	if (m_InitRunning) {
		if (theApp.GetMain() == NULL)
			return;
		Run(TRUE);
		m_InitRunning = FALSE;
	}
	if (!IsRunning())
		return;
	if (m_ProgressDlg.Canceled())
		Run(FALSE);
	m_Clock++;
	switch (m_State) {
	case EDIT:
		{
			int	code, retc = DISABLED;
			if (m_Engine->GetPluginCount() < m_MaxPlugins) {
				int	tries = 0;
				while (retc == DISABLED || retc == EVENT) {
					code = GetRandEdit();
					if (!tries || retc == EVENT)
						PRINTF(_T("%d: "), m_EditsDone);
					retc = ApplyEdit(code); 
					tries++;
				} 
			} else	// too many plugins
				retc = ApplyEdit(UCODE_DELETE);	// delete one
			if (retc == ABORT) {	// if edit failed
				Run(FALSE);
			} else {	// edit succeeded
				m_EditsDone++;
				if (m_EditsDone < m_MaxEdits) {
					if (!(m_EditsDone % m_PassEdits)) {	// if undo boundary
						// for final pass, undo all the way back, else
						// do number of undos specified by m_PassUndos 
						m_UndosToDo = LastPass() ? INT_MAX : m_PassUndos;
						m_UndosDone = 0;
						SetState(UNDO);	// start undoing
					}
				} else {	// too many edits
					Run(FALSE);
				}
			}
		}
		break;
	case UNDO:
		if (m_UndoMgr->CanUndo() && m_UndosDone < m_UndosToDo) {
			PRINTF(_T("%d: Undo %s\n"), m_UndosDone,
				m_UndoMgr->GetUndoTitle());
			m_UndoMgr->Undo();
			m_UndosDone++;
			DoRandEvent(m_UndosDone);
		} else {	// undos completed
			if (LastPass()) {
				m_PassesDone++;
				Run(FALSE);
			} else {	// not final pass
				SetState(REDO);	// start redoing
			}
		}
		break;
	case REDO:
		if (m_UndoMgr->CanRedo()) {
			PRINTF(_T("%d: Redo %s\n"), m_UndosDone,
				m_UndoMgr->GetRedoTitle());
			m_UndoMgr->Redo();
			m_UndosDone--;
			DoRandEvent(m_UndosDone);
		} else {	// redos completed
			m_PassesDone++;
			SetState(EDIT);	// resume editing
		}
		break;
	}
	if (m_State != STOP) {
		m_StepsDone++;
		// access progress control directly to avoid pumping message
		CWnd	*pProgCtrl = m_ProgressDlg.GetDlgItem(IDC_PROGRESS);
		pProgCtrl->SendMessage(PBM_SETPOS, m_StepsDone);
	}
}
