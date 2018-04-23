#include "RecursiveRWLock.h"

namespace thd {

SHARED_DLL_DECL CRecursiveRWLock::thread_table_t CRecursiveRWLock::s_thdContext;
SHARED_DLL_DECL CRecursiveRWLock::instc_table_t CRecursiveRWLock::s_instcIdxs;
/*
 */

CRecursiveRWLock::CRecursiveRWLock() throw()
	: m_readersReading(0)
	, m_writerWriting(0)
	, m_threadId(0)
	, m_recursionCount(0)
{
	m_thisIdx = s_instcIdxs.Add();
	assert(XQXTABLEINDEXS_INDEX_NIL != m_thisIdx);
}

/*
 */

CRecursiveRWLock::~CRecursiveRWLock() throw()
{
	m_mutex.Lock();
	m_writerWriting = 0;
	m_readersReading = 0;
	m_lockFree.NotifyAll();
	m_mutex.Unlock();
	// remove thread context
	thread_table_t::iterator it(s_thdContext.Begin());
	for(; s_thdContext.End() != it; ++it) {
		thread_table_t::value_t itValue(it.GetValue());
		if(XQXTABLE2S_INDEX_NIL != itValue.nIndex && itValue.pObject) {
			store_table_t::value_t thdReaders(itValue.pObject->Find(m_thisIdx));
			if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex) {
				itValue.pObject->Remove(m_thisIdx);
			}
		}
	}
	s_instcIdxs.Remove(m_thisIdx);
	// Wait all done.
	m_mutex.Lock();
	m_thisIdx = XQXTABLEINDEXS_INDEX_NIL;
	m_recursionCount = 0;
	m_threadId = 0;
	m_mutex.Unlock();
}

/*
 */

void CRecursiveRWLock::LockWrite() throw()
{
	m_mutex.Lock();

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			++m_recursionCount;
			m_mutex.Unlock();
			return;
		}
	}
	
	while(m_writerWriting || m_readersReading) {
		m_lockFree.Wait(m_mutex);
	}

	m_writerWriting++;

	m_threadId = threadId;
	m_recursionCount = 1;

	m_mutex.Unlock();
}

/*
 */

bool CRecursiveRWLock::TimedLockWrite(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
	}

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			++m_recursionCount;
			m_mutex.Unlock();
			return true;
		}
	}
	
	uint64_t expires = GetSysTickCount() + msec;

	while(m_writerWriting || m_readersReading) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	++m_writerWriting;

	m_threadId = threadId;
	m_recursionCount = 1;

	m_mutex.Unlock();

	return true;
}

void CRecursiveRWLock::UnlockWrite() throw()
{
	m_mutex.Lock();
	// unlock write lock
	if(m_writerWriting > 0)
	{
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			if(m_recursionCount > 1) {
				--m_recursionCount;
				m_mutex.Unlock();
				return;
			} else {
				m_recursionCount = 0;
				m_threadId = 0;
			}
		}

		m_writerWriting = 0;
		m_lockFree.NotifyAll();
	}
	m_mutex.Unlock();
}

/*
 */

void CRecursiveRWLock::LockRead() throw()
{
	m_mutex.Lock();

	if(m_writerWriting > 0) {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return;
		}
	}

	if(IncThreadReader()) {
		m_mutex.Unlock();
		return;
	}
	
	while(m_writerWriting) {
		m_lockFree.Wait(m_mutex);
	}
	
	++m_readersReading;
	m_mutex.Unlock();
}

/*
 */

bool CRecursiveRWLock::TimedLockRead(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
	}

	if(m_writerWriting > 0) {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return true;
		}
	}

	if(IncThreadReader()) {
		m_mutex.Unlock();
		return true;
	}

	uint64_t expires = GetSysTickCount() + msec;

	while(m_writerWriting) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	m_readersReading++;
	m_mutex.Unlock();

	return true;
}

/*
 */

void CRecursiveRWLock::UnlockRead() throw()
{
	m_mutex.Lock();

	if(m_writerWriting > 0) {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return;
		}
	}

	if(DecThreadReader()) {
		m_mutex.Unlock();
		return;
	}

	// unlock read lock
	if(m_readersReading > 0)
	{
		if(--m_readersReading == 0) {
			m_lockFree.Notify();
		}
	}
	m_mutex.Unlock();
}

void CRecursiveRWLock::UpgradeLockRead() throw()
{
	m_mutex.Lock();

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return;
		}
	}

	if(ClearThreadReader()) {
		// unlock read lock
		if(m_readersReading > 0)
		{
			--m_readersReading;
		}
	}

	while(m_writerWriting) {
		m_lockFree.Wait(m_mutex);
	}

	m_writerWriting = 1;

	// wait all read unlock
	while(m_readersReading) {
		m_lockFree.Wait(m_mutex);
	}

	m_threadId = threadId;
	m_recursionCount = 1;

	m_mutex.Unlock();
}

bool CRecursiveRWLock::TimedUpgradeLockRead(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
	}

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return true;
		}
	}

	if(ClearThreadReader()) {
		// unlock read lock
		if(m_readersReading > 0)
		{
			--m_readersReading;
		}
	}

	uint64_t expires = GetSysTickCount() + msec;

	while(m_writerWriting) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	m_writerWriting = 1;
	// wait all read unlock
	while(m_readersReading) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	m_mutex.Unlock();
	return true;
}

bool CRecursiveRWLock::DegradeLockWrite() throw()
{
	m_mutex.Lock();
	// unlock write lock
	if(m_writerWriting > 0)
	{
		assert(m_readersReading == 0);
		++m_readersReading;
		m_writerWriting = 0;
		m_lockFree.NotifyAll();

		m_mutex.Unlock();
		return true;
	}
	m_mutex.Unlock();
	return false;
}

size_t& CRecursiveRWLock::GetThreadIndex()
{
#if COMPILER == COMPILER_MICROSOFT
	__declspec(thread) static size_t s_thdIndex = XQXTABLE2S_INDEX_NIL;
#elif COMPILER == COMPILER_GNU
	__thread static size_t s_thdIndex = XQXTABLE2S_INDEX_NIL;
#elif COMPILER == COMPILER_BORLAND
	static size_t __thread s_thdIndex = XQXTABLE2S_INDEX_NIL;
#else
#error "not support";
#endif
	return s_thdIndex;
}

bool CRecursiveRWLock::IncThreadReader()
{
	bool bSkipLock = false;
	size_t& thdIndex = GetThreadIndex();
	if(XQXTABLE2S_INDEX_NIL == thdIndex) {
		store_table_t tmp;
		int32_t thdReaders = 1;
		tmp.Add(m_thisIdx, thdReaders);
		thdIndex = s_thdContext.Add(tmp);
	} else {
		store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
		if(pStore) {
			store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
			if(XQXTABLE1STORE_INDEX_NIL == thdReaders.nIndex) {
				thdReaders.object = 1;
				assert(pStore->Add(m_thisIdx, thdReaders.object));
			} else {
				pStore->Change(m_thisIdx, ++thdReaders.object);
				if(thdReaders.object > 1) {
					bSkipLock = true;
				}
			}
		} else {
			assert(pStore);
		}
	}
	return bSkipLock;
}

bool CRecursiveRWLock::DecThreadReader()
{
	bool bSkipUnlock = false;
	size_t& thdIndex = GetThreadIndex();
	if(XQXTABLE2S_INDEX_NIL != thdIndex) {
		store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
		if(pStore) {
			store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
			if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex && thdReaders.object > 0) {
				pStore->Change(m_thisIdx, --thdReaders.object);
				if(thdReaders.object > 0) {
					bSkipUnlock = true;
				}
			}
		} else {
			assert(pStore);
		}
	}
	return bSkipUnlock;
}

bool CRecursiveRWLock::ClearThreadReader()
{
	bool bClear = false;
	size_t& thdIndex = GetThreadIndex();
	if(XQXTABLE2S_INDEX_NIL != thdIndex) {
		store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
		if(pStore) {
			store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
			if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex && thdReaders.object > 0) {
				pStore->Change(m_thisIdx, 0);
				bClear = true;
			}
		} else {
			assert(pStore);
		}
	}
	return bClear;
}

bool CRecursiveRWLock::HasThreadReader()
{
	bool hasLock = false;
	size_t& thdIndex = GetThreadIndex();
	if(XQXTABLE2S_INDEX_NIL != thdIndex) {
		store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
		if(pStore) {
			store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
			if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex && thdReaders.object > 0) {
				hasLock = true;
			}
		} else {
			assert(pStore);
		}
	}
	return hasLock;
}

bool CRecursiveRWLock::Using() throw()
{
	bool bUsing = false;
	m_mutex.Lock();
	bUsing = (0 != m_writerWriting || 0 != m_readersReading);
	m_mutex.Unlock();
	return bUsing;
}

} // namespace thd

/* end of source file */
