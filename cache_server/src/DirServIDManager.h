/* 
 * File:   DirServIDManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2018_1_5, 21:10
 */

#ifndef _DIRSERVIDMANAGER_H
#define	_DIRSERVIDMANAGER_H

#include <map>
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

typedef std::map<uint16_t, uint16_t> DIRSERVID_TO_DBID_T;

class CDirServIDManager
	: public util::Singleton<CDirServIDManager>
{
public:

	void Add(uint16_t u16SerId, uint16_t u16DBId)
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		m_serverIds.insert(DIRSERVID_TO_DBID_T
			::value_type(u16SerId, u16DBId));
	}

	bool Find(uint16_t u16SerId, uint16_t& outDBId)
	{
		thd::CScopedReadLock rLock(m_rwLock);
		DIRSERVID_TO_DBID_T::const_iterator it(
			m_serverIds.find(u16SerId));
		if(m_serverIds.end() == it) {
			return false;
		}
		outDBId = it->second;
		return true;
	}

	void Remove(uint16_t u16SerId)
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		m_serverIds.erase(u16SerId);
	}

	void Clear() {
		thd::CScopedWriteLock wLock(m_rwLock);
		if(m_serverIds.empty()) {
			return;
		}
		m_serverIds.clear();
	}

 
private:
    DIRSERVID_TO_DBID_T m_serverIds;
	thd::CSpinRWLock m_rwLock;
};

#endif /* _DIRSERVIDMANAGER_H */



