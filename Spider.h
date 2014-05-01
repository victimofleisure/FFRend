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

		parallel plugin routing spider
 
*/

#pragma once

#include "ArrayEx.h"

class CEngine;
class CPlugin;

class CSpider : public WObject {
public:
// Construction
	CSpider(CEngine& Engine);

// Operations
	bool	Crawl(bool OptimizeRouting = TRUE);

protected:
// Types
	class CMixerRoute {
	public:
		int		m_PlugIdx;				// mixer's plugin index
		int		m_InpIdx;				// mixer's input index
		int		m_RouteLen;				// distance to mixer, in frames
	};
	typedef CArrayEx<CMixerRoute, CMixerRoute&> CMixerRouteArray;
	class CNode : public WObject {
	public:
	// Operations
		int		FindMixerRoute(int PlugIdx, int InpIdx) const;

	// Member data
		bool	m_OnBranch;				// true if we're on branch being crawled
		bool	m_FBSource;				// true if we're a source of feedback
		int		m_Depth;				// our recursion depth
		int		m_MaxOutputRouteLen;	// longest route to output, in frames
		CMixerRouteArray	m_MixerRoute;	// array of routes to sink mixers
	};
	typedef CArrayEx<CNode, CNode&> CNodeArray;

// Member data
	CEngine&	m_Engine;			// reference to engine
	int			m_Depth;			// current recursion depth
	CNodeArray	m_Node;				// array of crawl states, one for each plugin
	int			m_MixerPlugIdx;		// plugin index of nearest sink mixer
									// on branch we're crawling, or -1 if none
	int			m_MixerInpIdx;		// which one of nearest sink mixer's
									// inputs we're crawling, or -1 if none

// Helpers
	CPlugin&	GetPlugin(int PlugIdx) const;
	LPCTSTR	GetName(int PlugIdx) const;
	bool	CrawlPlugin(int PlugIdx);
	void	DumpNodes() const;
	void	DelayMixerRoutes() const;
};

