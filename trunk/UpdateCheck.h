// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      23jan12	initial version

        check for updates
 
*/

#ifndef CUPDATECHECK_INCLUDED
#define CUPDATECHECK_INCLUDED

class CUpdateCheck : public WObject {
public:
// Types
	class CAppUpdateInfo : public WObject {
	public:
		ULARGE_INTEGER	m_Version;
		CString	m_URL;
	};

// Operations
	static	bool	Check(bool Explicit = TRUE);
	static	void	CheckAsync(HWND hWnd);
	static	void	OnAppUpdateInfo(WPARAM wParam, LPARAM lParam);

protected:
// Helpers
	static	bool	Check(ULARGE_INTEGER DownloadVersion, LPCTSTR DownloadURL, bool Explicit);
	static	bool	Reinstall(LPCTSTR InstallerPath);
	static	UINT	CheckWorker(LPVOID pParam);
};


#endif
