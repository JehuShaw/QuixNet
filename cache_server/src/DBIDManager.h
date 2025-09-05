/* 
 * File:   DBIDManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2018_1_5, 21:10
 */

#ifndef DBIDMANAGER_H
#define	DBIDMANAGER_H

#include <map>
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

typedef char DIRSERVID_TAG;
typedef short BALSERVID_TAG;
typedef int BALUSERID_TAG;


template <class KeyType, class Tag>
class CDBIDManager
	: public util::Singleton<CDBIDManager<KeyType, Tag> >
{
	typedef std::map<KeyType, uint16_t> ROUTEID_TO_DBID_T;
public:

	void Add(KeyType routeId, uint16_t u16DBId)
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		m_serverIds.insert(typename ROUTEID_TO_DBID_T
			::value_type(routeId, u16DBId));
	}

	bool Find(KeyType routeId, uint16_t& outDBId)
	{
		thd::CScopedReadLock rLock(m_rwLock);
		typename ROUTEID_TO_DBID_T::const_iterator it(
			m_serverIds.find(routeId));
		if(m_serverIds.end() == it) {
			return false;
		}
		outDBId = it->second;
		return true;
	}

	void Remove(KeyType routeId)
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		m_serverIds.erase(routeId);
	}

	void Clear() {
		thd::CScopedWriteLock wLock(m_rwLock);
		if(m_serverIds.empty()) {
			return;
		}
		m_serverIds.clear();
	}

 
private:
    ROUTEID_TO_DBID_T m_serverIds;
	thd::CSpinRWLock m_rwLock;
};

#endif /* DBIDMANAGER_H */



