// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      06apr10	initial version
        01      11jan11	compute depth in frames instead of hops
		02		03may11	access m_QueueSize directly

		parallel plugin routing spider
 
*/

#include "stdafx.h"
#include "Spider.h"
#include "Engine.h"
#include "Plugin.h"

CSpider::CSpider(CEngine& Engine) :
	m_Engine(Engine)
{
	m_Depth = 0;
	m_MixerPlugIdx = -1;
	m_MixerInpIdx = -1;
}

inline CPlugin&	CSpider::GetPlugin(int PlugIdx) const
{
	return(m_Engine.GetPlugin(PlugIdx));
}

inline LPCTSTR CSpider::GetName(int PlugIdx) const
{
	return(GetPlugin(PlugIdx).GetName());
}

bool CSpider::Crawl(bool OptimizeRouting)
{
	int	plugs = m_Engine.GetPluginCount();
	if (!plugs)
		return(FALSE);
	m_Node.SetSize(plugs);	// initializes nodes to zero
#ifdef SPIDER_NATTER
	_tprintf(_T("crawling...\n"));
#endif
	CrawlPlugin(plugs - 1);
#ifdef SPIDER_NATTER
	DumpNodes();
#endif
	if (OptimizeRouting) {
		// compensate for skew that results when a mixer is connected
		// to the same source by multiple routes of differing lengths
		DelayMixerRoutes();
	} else	// unoptimized
		m_Engine.PostCrawl();	// let engine play with routing
	return(TRUE);
}

void CSpider::DelayMixerRoutes() const
{
	int	plugs = m_Engine.GetPluginCount();
	for (int PlugIdx = 0; PlugIdx < plugs; PlugIdx++) {
		const CNode&	node = m_Node[PlugIdx];
		int	routes = node.m_MixerRoute.GetSize();
		if (routes > 1) {
			for (int RouteIdx = 0; RouteIdx < routes; RouteIdx++) {
				const CMixerRoute&	mr = node.m_MixerRoute[RouteIdx];
				int	OutputRouteLen = mr.m_RouteLen + 
					m_Node[mr.m_PlugIdx].m_MaxOutputRouteLen;
				int	InputDelay = node.m_MaxOutputRouteLen - OutputRouteLen;
				if (InputDelay > 0) {	// feedback can cause negative lengths
#ifdef SPIDER_NATTER
					_tprintf(_T("delay %s:%c %d\n"), GetName(mr.m_PlugIdx), 
						'A' + mr.m_InpIdx, InputDelay);
#endif
					// input delay is measured in frames; add one for output buffer,
					// or possible copy of input frame if plugin operates in place
					int	frames = InputDelay + 1;
					CPlugin&	Mixer = GetPlugin(mr.m_PlugIdx);
					Mixer.m_Input[mr.m_InpIdx].m_QueueSize = frames;
				}
			}
		}
	}
}

inline int CSpider::CNode::FindMixerRoute(int PlugIdx, int InpIdx) const
{
	int	routes = m_MixerRoute.GetSize();
	for (int RouteIdx = 0; RouteIdx < routes; RouteIdx++) {
		const CMixerRoute&	mr = m_MixerRoute[RouteIdx];
		if (PlugIdx == mr.m_PlugIdx && InpIdx == mr.m_InpIdx)
			return(RouteIdx);
	}
	return(-1);
}

bool CSpider::CrawlPlugin(int PlugIdx)
{
	CPlugin&	plug = GetPlugin(PlugIdx);
	CNode&	node = m_Node[PlugIdx];
#ifdef SPIDER_NATTER
	int	Indent = m_Depth;
	_tprintf(_T("%*s%s\n"), Indent, _T(""), plug.GetName());
#endif
	if (node.m_OnBranch)	// if we're already on this branch
		return(FALSE);	// feedback detected
	node.m_OnBranch = TRUE;
	node.m_Depth = m_Depth;
	plug.m_CanRender = TRUE;	// plugin is connected to output
	if (m_Depth > node.m_MaxOutputRouteLen)
		node.m_MaxOutputRouteLen = m_Depth;
	// if a nearest sink mixer exists on this branch, calculate length of route
	// to it, and add it to our sink mixer route list if it's not there already;
	// skip single-input effect plugins unless they don't have an input source
	if (m_MixerPlugIdx >= 0
	&& (plug.IsSource() || plug.IsMixer() || plug.GetInputSource(0) == NULL)) {
		int	RouteIdx = node.FindMixerRoute(m_MixerPlugIdx, m_MixerInpIdx);
		if (RouteIdx < 0) {	// if route not found
			CMixerRoute	mr;
			mr.m_PlugIdx = m_MixerPlugIdx;
			mr.m_InpIdx = m_MixerInpIdx;
			mr.m_RouteLen = m_Depth - m_Node[m_MixerPlugIdx].m_MaxOutputRouteLen;
			node.m_MixerRoute.Add(mr);	// add new route
		}
	}
	if (!plug.IsSource()) {	// no need to crawl source plugins
		int	PrevMixerPlugIdx = m_MixerPlugIdx;
		int	PrevMixerInpIdx = m_MixerInpIdx;
		int	Inputs = plug.GetNumInputs();
		for (int InpIdx = 0; InpIdx < Inputs; InpIdx++) {
			CPlugin	*SrcPlug = plug.GetInputSource(InpIdx);
			if (SrcPlug != NULL) {	// if input is connected
				if (plug.IsMixer()) {
					m_MixerPlugIdx = PlugIdx;
					m_MixerInpIdx = InpIdx;
				}
				int	SrcIdx = SrcPlug->m_PlugIdx;
				int	SrcDelay = SrcPlug->GetThreadCount() + 1;
				m_Depth += SrcDelay;
				bool	retc = CrawlPlugin(SrcIdx);	// recursive crawl
				m_Depth -= SrcDelay;
				if (!retc) {	// if feedback detected
					CNode&	SrcNode = m_Node[SrcIdx];
					if (!(node.m_FBSource || SrcNode.m_FBSource)) {
						int	RouteLen = node.m_Depth - SrcNode.m_Depth;
#ifdef SPIDER_NATTER
						_tprintf(_T("%*s  FEEDBACK at %s:%c from %s, len=%d\n"), 
							Indent, _T(""), plug.GetName(), 'A' + InpIdx, 
							SrcPlug->GetName(), RouteLen);
#endif
//***						plug.m_Input[InpIdx].m_PrimeFrames = RouteLen + 1;	// causes strobing
						plug.m_Input[InpIdx].m_PrimeFrames = max(RouteLen - 1, 1);
						node.m_FBSource = TRUE;
					}
				}
			}
		}
		m_MixerPlugIdx = PrevMixerPlugIdx;
		m_MixerInpIdx = PrevMixerInpIdx;
	}
	node.m_OnBranch = FALSE;
	return(TRUE);
}

void CSpider::DumpNodes() const
{
	int	nodes = m_Node.GetSize();
	_tprintf(_T("%d nodes:\n"), nodes);
	for (int NodeIdx = 0; NodeIdx < nodes; NodeIdx++) {
		const CNode&	node = m_Node[NodeIdx];
		_tprintf(_T("%s %d%s"), GetName(NodeIdx), node.m_MaxOutputRouteLen,
			node.m_FBSource ? _T(" FB") : _T(""));
		CPlugin&	plug = GetPlugin(NodeIdx);
		int	inputs = plug.GetNumInputs();
		for (int InpIdx = 0; InpIdx < inputs; InpIdx++) {
			const CPlugin::CInput	inp = plug.m_Input[InpIdx];
			if (inp.m_PrimeFrames)
				_tprintf(_T(" %c=%d"), 'A' + InpIdx, inp.m_PrimeFrames);
		}
		_tprintf(_T("\n"));
		int	routes = node.m_MixerRoute.GetSize();
		for (int MixerIdx = 0; MixerIdx < routes; MixerIdx++) {
			const CMixerRoute&	mr = node.m_MixerRoute[MixerIdx];
			_tprintf(_T("-> %s:%c %d\n"), GetName(mr.m_PlugIdx), 
				'A' + mr.m_InpIdx, mr.m_RouteLen);
		}
	}
}
