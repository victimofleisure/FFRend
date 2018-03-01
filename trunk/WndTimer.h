// Copyleft 2008 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      14feb08	initial version
		01		06jan10	W64: make 64-bit compatible
		02		02may11	add return values

		window timer
 
*/

#ifndef CWNDTIMER_INCLUDED
#define CWNDTIMER_INCLUDED

class CWndTimer : public WObject {
public:
// Types
	typedef void (CALLBACK EXPORT* TIMER_CALLBACK)(HWND, UINT, W64UINT, DWORD);

// Construction
	CWndTimer();
	~CWndTimer();
	bool	Create(HWND hWnd, UINT Event, UINT Period, bool Enable = TRUE, TIMER_CALLBACK Callback = NULL);
	void	Destroy();

// Attributes
	HWND	GetWnd() const;
	bool	SetPeriod(UINT Period);
	UINT	GetPeriod() const;
	UINT	GetEvent() const;
	bool	Run(bool Enable);
	bool	IsRunning() const;

protected:
// Member data
	W64UINT	m_Timer;	// timer instance
	HWND	m_hWnd;		// window handle
	UINT	m_Event;	// event ID
	UINT	m_Period;	// period in milliseconds
	TIMER_CALLBACK	m_Callback;	// timer callback function
};

inline HWND CWndTimer::GetWnd() const
{
	return(m_hWnd);
}

inline UINT CWndTimer::GetEvent() const
{
	return(m_Event);
}

inline UINT CWndTimer::GetPeriod() const
{
	return(m_Period);
}

inline bool CWndTimer::IsRunning() const
{
	return(m_Timer != 0);
}

#endif
