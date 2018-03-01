// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		27apr10	initial version

		MIDI types

*/

#pragma once

typedef union tagMIDI_MSG {
	DWORD	dw;
	struct {
		BYTE	cmd;
		BYTE	p1;
		BYTE	p2;
	} s;
} MIDI_MSG;

enum {
	MIDI_CHANS = 16,
	MIDI_PARMS = 128
};

enum {	// MIDI channel messages
	MC_NOTE_OFF		= 0x80,
	MC_NOTE_ON		= 0x90,
	MC_KEY_AFT		= 0xa0,
	MC_CTRL_CHG		= 0xb0,
	MC_PROG_CHG		= 0xc0,
	MC_CHAN_AFT		= 0xd0,
	MC_PITCH_BEND	= 0xe0
};
