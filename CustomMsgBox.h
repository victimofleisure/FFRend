// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date	comments
		00		20jul07	initial version
		
		message box with custom button text
		
*/

#ifndef CCUSTOMMSGBOX_INCLUDED
#define CCUSTOMMSGBOX_INCLUDED

class CCustomMsgBox {
public:
//
// Displays a message box with custom button text.
//
// Remarks: This function allows you to replace the text of one or more buttons
// in a Windows message box.  For example, to create a message box with an "OK"
// button and a "Help" button, use a style of MB_OKCANCEL and a Button argument
// of {0, "Help"}.  In this case a return value of IDCANCEL would mean the Help
// button was pressed.  The buttons are indexed as follows: OK, Cancel, Abort,
// Retry, Ignore, Yes, No.  Note that this function is not thread-safe, because
// it uses static data members; call it from the main thread only.
//
// Returns: The dialog box menu-item value; see MessageBox.
//
	static	int	MsgBox(
		LPCTSTR	Text,			// Message to be displayed in the message box.
		UINT	Type = MB_OK,	// Style of message box.
		const LPCTSTR *BtnText = NULL,	// Array of pointers to replacement
								// button text strings; if an element is NULL,
								// the corresponding button's text is unchanged.
		int		Buttons = 0		// Number of elements in the button text array.
	);

private:
	static	LRESULT	CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);

	static	HHOOK	m_Hook;		// Handle of windows hook.
	static	const LPCTSTR	*m_ButtonText;	// Array of button text strings.
	static	int		m_Buttons;	// Number of elements in the button text array.
};

#endif
