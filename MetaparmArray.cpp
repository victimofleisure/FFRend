// Copyleft 2007 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      25dec07	initial version
        01      29jan08	in DumpGroups, remove unused local var

        metaparameter array
 
*/

#include "stdafx.h"
#include "MetaparmArray.h"

int CMetaparmArray::GetMaster(int ParmIdx) const
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	const CMetaparm&	mp = GetAt(ParmIdx);
	if (mp.IsSlave())		// if slave
		return(mp.m_Master);	// return master
	if (mp.IsMaster())		// if master
		return(ParmIdx);		// return self
	return(-1);	// not grouped
}

void CMetaparmArray::GetGroup(int ParmIdx, CMetaparmGroup& Group) const
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	int	MasterIdx = GetMaster(ParmIdx);	// get group master
	if (MasterIdx >= 0) {	// if grouped
		Group.m_Master = MasterIdx;	// copy master
		Group.m_Slave.Copy(GetAt(MasterIdx).m_Slave);	// copy slaves
	} else	// not grouped
		Group.Empty();	// return empty group
}

void CMetaparmArray::SetGroup(const CMetaparmGroup& Group)
{
	int	MasterIdx = Group.m_Master;	// get index of group's master
	if (MasterIdx < 0)
		return;	// nothing to do
	ASSERT(!GetAt(MasterIdx).IsMaster());	// verify master has no slaves
	GetAt(MasterIdx).m_Slave.Copy(Group.m_Slave);	// set master's slave list
	int	slaves = Group.GetSlaveCount();
	for (int j = 0; j < slaves; j++) {	// for each of group's slaves
		int	SlaveIdx = Group.m_Slave[j];
		ASSERT(!GetAt(SlaveIdx).IsSlave());	// verify slave is unowned
		GetAt(SlaveIdx).m_Master = MasterIdx;	// link slave to master
	}
}

void CMetaparmArray::GetGroups(CMetaparmGroupList& GroupList) const
{
	GroupList.RemoveAll();
	int	parms = GetSize();
	for (int i = 0; i < parms; i++) {	// for each metaparameter
		const CMetaparm&	mp = GetAt(i);
		if (mp.IsMaster()) {	// if metaparameter has slaves
			CMetaparmGroup	mpg;
			mpg.m_Master = i;	// copy master
			mpg.m_Slave.Copy(mp.m_Slave);	// copy slaves
			GroupList.Add(mpg);	// add new group
		}
	}
}

void CMetaparmArray::SetGroups(const CMetaparmGroupList& GroupList)
{
	int	groups = GroupList.GetSize();
	for (int i = 0; i < groups; i++)
		SetGroup(GroupList[i]);
}

LPCTSTR	CMetaparmArray::GetGroupName(int ParmIdx) const
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	int	MasterIdx = GetMaster(ParmIdx);	// get group master
	if (MasterIdx >= 0)	// if grouped
		return(GetAt(MasterIdx).m_Name);	// return group name
	return(NULL);	// not grouped
}

void CMetaparmArray::RemoveAllGroups()
{
	int	parms = GetSize();
	for (int i = 0; i < parms; i++) {
		CMetaparm&	mp = GetAt(i);
		mp.m_Slave.RemoveAll();
		mp.m_Master = -1;
	}
}

void CMetaparmArray::DumpGroups() const
{
	int	parms = GetSize();
	printf("%d metaparameters\n", parms);
	for (int i = 0; i < parms; i++) {
		const CMetaparm&	mp = GetAt(i);
		if (mp.IsGrouped()) {
			printf("%d: %d [", i, mp.m_Master);
			int	Slaves = mp.GetSlaveCount();
			for (int j = 0; j < Slaves; j++)
				printf(" %d", mp.m_Slave[j]);
			printf("]\n");
		}
	}
}

void CMetaparmArray::Reset(int ParmIdx)
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	Unlink(ParmIdx);
	GetAt(ParmIdx).Reset();
}

void CMetaparmArray::InsertAt(int ParmIdx, CMetaparm& Metaparm)
{
	ASSERT(ParmIdx >= 0 && ParmIdx <= GetSize());
	m_Metaparm.InsertAt(ParmIdx, Metaparm);
	OffsetLinks(ParmIdx, 1);
}

void CMetaparmArray::RemoveAt(int ParmIdx)
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	Unlink(ParmIdx);
	m_Metaparm.RemoveAt(ParmIdx);
	OffsetLinks(ParmIdx, -1);
}

void CMetaparmArray::Move(int Src, int Dst)
{
	ASSERT(Src >= 0 && Src < GetSize());
	ASSERT(Dst >= 0 && Dst < GetSize());
	CMetaparm	tmp = GetAt(Src);
	m_Metaparm.RemoveAt(Src);
	m_Metaparm.InsertAt(Dst, tmp);
	//	for all inputs in the range Src..Dst
	//		if (input == Src)
	//			shift input by (Dst - Src)
	//		else
	//			if (Src < Dst)
	//				shift input down one
	//			else
	//				shift input up one
	int	Delta = Dst - Src;
	int	Shift, Start, End;
	if (Src < Dst) {
		Start = Src;
		End = Dst;
		Shift = -1;
	} else {	// invert range
		Start = Dst;
		End = Src;
		Shift = 1;
	}
	CGroupLinkIter	Iter(*this);
	int	*Link;
	while ((Link = Iter.Next()) != NULL) {
		if (*Link >= Start && *Link <= End) {
			if (*Link == Src)
				*Link += Delta;
			else
				*Link += Shift;
		}
	}
}

void CMetaparmArray::Group(int MasterIdx, int SlaveIdx)
{
	ASSERT(MasterIdx >= 0 && MasterIdx < GetSize());
	ASSERT(SlaveIdx >= 0 && SlaveIdx < GetSize());
	ASSERT(!GetAt(SlaveIdx).IsSlave());	// verify slave is unowned
	CMetaparm&	Master = GetAt(MasterIdx);
	Master.m_Slave.Add(SlaveIdx);	// add slave to master's list
	GetAt(SlaveIdx).m_Master = MasterIdx;	// link slave to master
}

void CMetaparmArray::Ungroup(int ParmIdx)
{
	int	MasterIdx = GetMaster(ParmIdx);	// get group master
	if (MasterIdx >= 0)	// if grouped
		Unlink(MasterIdx);	// leave group
}

void CMetaparmArray::Unlink(int ParmIdx)
{
	ASSERT(ParmIdx >= 0 && ParmIdx < GetSize());
	CMetaparm&	mp = GetAt(ParmIdx);
	if (mp.IsMaster()) {	// if we're a master
		int	slaves = mp.GetSlaveCount();
		for (int i = 0; i < slaves; i++) {	// for each of our slaves
			int	SlaveIdx = mp.m_Slave[i];
			ASSERT(GetAt(SlaveIdx).m_Master == ParmIdx);	// verify we're owner
			GetAt(SlaveIdx).m_Master = -1;	// set slave free
		}
		mp.m_Slave.RemoveAll();	// destroy slave list
	}
	if (mp.IsSlave()) {	// if we're a slave
		CMetaparm&	master = GetAt(mp.m_Master);	// access our master
		mp.m_Master = -1;	// mark ourself as free
		int	slaves = master.GetSlaveCount();
		int	i;
		for (i = 0; i < slaves; i++) {	// for each of master's slaves
			if (int(master.m_Slave[i]) == ParmIdx) {	// if slave is us
				master.m_Slave.RemoveAt(i);	// detach from master
				break;	// early out assumes no duplicate slaves
			}
		}
		ASSERT(i < slaves);	// verify master was linked to us
	}
}

void CMetaparmArray::OffsetLinks(int ParmIdx, int Offset)
{
	CGroupLinkIter	Iter(*this);
	int	*Link;
	while ((Link = Iter.Next()) != NULL) {
		if (*Link >= ParmIdx)
			(*Link) += Offset;
	}

}

CMetaparmArray::CGroupLinkIter::CGroupLinkIter(CMetaparmArray& Metaparm) :
	m_Metaparm(Metaparm)
{
	m_ParmIdx = 0;
	m_SlaveIdx = -1;
}

int *CMetaparmArray::CGroupLinkIter::Next()
{
	while (m_ParmIdx < m_Metaparm.GetSize()) {
		CMetaparm&	mp = m_Metaparm[m_ParmIdx];
		if (m_SlaveIdx < 0) {
			m_SlaveIdx = 0;
			return(&mp.m_Master);	// return master link
		}
		if (m_SlaveIdx < mp.GetSlaveCount()) {
			int	*Link = &mp.m_Slave[m_SlaveIdx];
			m_SlaveIdx++;
			return(Link);	// return slave link
		}
		m_ParmIdx++;
		m_SlaveIdx = -1;
	}
	return(NULL);
}

int CMetaparmArray::Find(const CMetaparm& Metaparm) const
{
	int	parms = GetSize();
	for (int i = 0; i < parms; i++) {
		if (&GetAt(i) == &Metaparm)
			return(i);
	}
	return(-1);
}
