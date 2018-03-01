// Copyleft 2006 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*	
		chris korda

		revision history:
		rev		date	comments
		00		24jul06	initial version
		01		19dec06	add process frame copy
		02		03apr11	add attach/detach
		03		06may12	add extended instance

		wrapper for freeframe plugin instance

*/

#ifndef CFFINSTANCE_INCLUDED
#define CFFINSTANCE_INCLUDED

#include "FreeFrame.h"

#include "FFPlugin.h"

class CFFInstance : public WObject {
public:
// Construction
	CFFInstance();
	~CFFInstance();
	bool	Create(CFFPlugin *Plugin, const VideoInfoStruct& VideoInfo);
	bool	Destroy();

// Attributes
	bool	IsCreated() const;
	LPCSTR	GetParamDisplay(int ParamIdx) const;
	float	GetParam(int ParamIdx) const;
	bool	SetParam(const SetParameterStruct& sps);
	bool	SetParam(int ParamIdx, float Value);
	LPVOID	GetInstance() const;
	FF_Main_FuncPtr	GetFuncPtr() const;

// Operations
	bool	ProcessFrame(LPVOID Frame);
	bool	ProcessFrameCopy(const ProcessFrameCopyStruct& pfcs);
	bool	Attach(const CFFInstance& Instance);
	void	Detach();

protected:
// Member data
	FF_Main_FuncPtr	m_pff;	// pointer to the plugin's main function
	LPVOID	m_Instance;		// if we're created, plugin's instance ID

// Helpers
	LPVOID	CallFF(DWORD Code, LPVOID Arg) const;
};

inline bool CFFInstance::IsCreated() const
{
	return(m_Instance != NULL);
}

inline LPVOID CFFInstance::CallFF(DWORD Code, LPVOID Arg) const
{
	ASSERT(m_Instance != NULL);
	return(m_pff(Code, Arg, UINT(m_Instance)));
}

inline LPCSTR CFFInstance::GetParamDisplay(int ParamIdx) const
{
	return((LPCSTR)CallFF(FF_GETPARAMETERDISPLAY, LPVOID(ParamIdx)));
}

inline float CFFInstance::GetParam(int ParamIdx) const
{
	plugMainUnionTag	pmu;
	pmu.ivalue = (int)CallFF(FF_GETPARAMETER, LPVOID(ParamIdx));
	return(pmu.fvalue);
}

inline bool CFFInstance::SetParam(const SetParameterStruct& sps)
{
	return((int)CallFF(FF_SETPARAMETER, LPVOID(&sps)) != FF_FAIL);
}

inline bool CFFInstance::SetParam(int ParamIdx, float Value)
{
	SetParameterStruct	sps;
	sps.index = ParamIdx;
	sps.value = Value;
	return(SetParam(sps));
}

inline bool CFFInstance::ProcessFrame(LPVOID Frame)
{
	return((int)CallFF(FF_PROCESSFRAME, Frame) != FF_FAIL);
}

inline bool CFFInstance::ProcessFrameCopy(const ProcessFrameCopyStruct& pfcs)
{
	return((int)CallFF(FF_PROCESSFRAMECOPY, LPVOID(&pfcs)) != FF_FAIL);
}

inline LPVOID CFFInstance::GetInstance() const
{
	return(m_Instance);
}

inline FF_Main_FuncPtr CFFInstance::GetFuncPtr() const
{
	return(m_pff);
}

class CFFInstanceEx : public WObject {
public:
// Construction
	CFFInstanceEx();
	void	Create(CFFInstance& Instance, bool Bypass = FALSE);

// Attributes
	bool	SetParam(int ParamIdx, float Value);

protected:
// Member data
	FF_Main_FuncPtr	m_pff;	// pointer to the plugin's main function
	LPVOID	m_Instance;		// plugin's instance ID

// Helpers
	LPVOID	CallFF(DWORD Code, LPVOID Arg) const;
	static LPVOID __stdcall DummyFunc(DWORD, LPVOID, DWORD);
};

inline CFFInstanceEx::CFFInstanceEx()
{
	m_pff = NULL;
	m_Instance = NULL;
}

inline void CFFInstanceEx::Create(CFFInstance& Instance, bool Bypass)
{
	m_Instance = Instance.GetInstance();
	if (Bypass)
		m_pff = DummyFunc;
	else
		m_pff = Instance.GetFuncPtr();
}

inline LPVOID __stdcall CFFInstanceEx::DummyFunc(DWORD, LPVOID, DWORD)
{
	return(FF_SUCCESS);
}

inline LPVOID CFFInstanceEx::CallFF(DWORD Code, LPVOID Arg) const
{
	ASSERT(m_Instance != NULL);
	return(m_pff(Code, Arg, UINT(m_Instance)));
}

inline bool CFFInstanceEx::SetParam(int ParamIdx, float Value)
{
	SetParameterStruct	sps;
	sps.index = ParamIdx;
	sps.value = Value;
	return((int)CallFF(FF_SETPARAMETER, LPVOID(&sps)) != FF_FAIL);
}

#endif
