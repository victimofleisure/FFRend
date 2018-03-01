// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
		00		03nov06	initial version
		01		22dec07	add CMidiAssign ctor
		02		29jan08	add SetMsg to fix warnings
		03		27apr10	rename plugin index to slot index

		MIDI assignment information
 
*/

#include "stdafx.h"
#include "MidiInfo.h"

const CMidiInfo	CMidiInfo::m_Default;

const int CMidiInfo::CmdToEvent[] = {
	0, 0, 0, 0, 0, 0, 0, 0,	// unused
	-1,			// note off
	MET_NOTE,	// note on
	-1,			// key aftertouch
	MET_CTRL,	// control change
	-1,			// program change
	-1,			// channel aftertouch
	MET_PITCH,	// pitch bend
	-1,			// system
};

CMidiInfo::CMidiInfo()
{
	m_Range = 1;
	m_Event = 0;
	m_Chan = 0;
	m_Ctrl = 0;
}

CMidiInfo::CMidiInfo(float Range, int Event, int Chan, int Ctrl)
{
	m_Range = Range;
	SetMsg(Event, Chan, Ctrl);
}

CMidiAssign::CMidiAssign()
{
	m_SlotIdx = SPI_INVALID;
	m_ParmIdx = 0;
	m_PropIdx = 0;
}

