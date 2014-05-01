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
		02		07may11	add set mod phase
		03		10jan12	add FastCalcVal

		FreeFrame parameter
 
*/

#pragma once

#include "Oscillator.h"
#include "FFPlugInfo.h"

class CFFParm : public WObject {
public:
// Construction
	CFFParm();

// Types
	typedef CFFPlugInfo::FRANGE FRANGE;

// Member data
	CString	m_Name;			// parameter name
	float	m_Val;			// shadow of current value
	COscillator	m_Osc;		// modulating oscillator
	float	m_ModFreq;		// modulation frequency
	bool	m_ModEnable;	// true if modulation is enabled
	bool	m_IsModulating;	// true if oscillator is running
	bool	m_HasModRange;	// true if a modulation range exists
	FRANGE	m_ModRange;		// modulation range
	CMidiInfo	m_MidiInfo[PARAM_MIDI_PROPS];	// MIDI information

// Attributes
	void	SetVal(float Val);
	void	SetModEnable(bool Enable);
	void	SetModFreq(float Freq, float Speed);
	void	SetModWave(int Wave);
	void	SetModPulseWidth(float PW);
	void	SetModRange(FRANGE Range);
	void	SetModPhase(float Phase);

// Operations
	float	CalcVal();
	float	FastCalcVal();
	void	UpdateOsc();
	static	float	NormModWave(int Wave);
	static	int		DenormModWave(float NormWave);

protected:
// Helpers
	void	UpdateModEnable();
};

inline void CFFParm::SetVal(float Val)
{
	m_Val = Val;
	if (m_IsModulating)
		UpdateOsc();
}

inline float CFFParm::FastCalcVal()
{
	float	val = float((m_Osc.GetVal() + 1) / 2);
	if (m_HasModRange) {
		float	delta = m_ModRange.End - m_ModRange.Start;
		val = delta * val + m_ModRange.Start;
	}
	m_Val = val;
	return(val);
}

inline void CFFParm::UpdateModEnable()
{
	bool	IsMod = m_ModEnable && m_Osc.GetFreq() != 0;
	if (!m_IsModulating && IsMod)
		UpdateOsc();
	m_IsModulating = IsMod;
}

inline void CFFParm::SetModEnable(bool Enable)
{
	m_ModEnable = Enable;
	UpdateModEnable();
}

inline void CFFParm::SetModFreq(float Freq, float Speed)
{
	m_ModFreq = Freq;
	m_Osc.SetFreq(Freq * Speed);
	UpdateModEnable();
}

inline float CFFParm::NormModWave(int Wave)
{
	return(float(Wave) / COscillator::WAVEFORMS);
}

inline int CFFParm::DenormModWave(float NormWave)
{
	int Wave = trunc(NormWave * COscillator::WAVEFORMS);
	return(min(Wave, COscillator::WAVEFORMS - 1));
}

inline void CFFParm::SetModWave(int Wave)
{
	m_Osc.SetWaveform(Wave);
}

inline void CFFParm::SetModPulseWidth(float PW)
{
	m_Osc.SetPulseWidth(PW);
}

inline void CFFParm::SetModRange(FRANGE Range)
{
	m_ModRange = Range;
	m_HasModRange = Range.Start >= 0;
	if (m_HasModRange && m_IsModulating)
		UpdateOsc();
}
