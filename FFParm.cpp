// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      08mar10	initial version
		01		07may11	add set mod phase
		02		10jan12	add FastCalcVal

		FreeFrame parameter
 
*/

#include "stdafx.h"
#include "FFParm.h"

CFFParm::CFFParm()
{
	m_Val = 0;
	m_ModEnable = TRUE;
	m_IsModulating = FALSE;
	m_HasModRange = FALSE;
	m_ModRange.Start = -1;
	m_ModRange.End = -1;
}

void CFFParm::UpdateOsc()
{
	float	val = m_Val;
	if (m_HasModRange) {
		float	delta = m_ModRange.End - m_ModRange.Start;
		if (delta) {	// avoid divide by zero
			val = CLAMP(val, m_ModRange.Start, m_ModRange.End);	// clamp position to range
			val = (val - m_ModRange.Start) / delta;	// denormalize from range
		}
	}
	m_Osc.SetPhaseFromVal(val * 2 - 1);
}

void CFFParm::SetModPhase(float Phase)
{
	m_Osc.SetPhase(Phase);
	if (m_IsModulating)
		CalcVal();
}

float CFFParm::CalcVal()
{
	return(FastCalcVal());
}
