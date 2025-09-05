/*
 * File:   WrapRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef WRAPRWLOCK_H
#define WRAPRWLOCK_H

#include "SpinRWLock.h"
#include <stdexcept>

namespace thd {

template<class WrapedType, class LockType, class UseType>
class CWrapRWLock;

template<class WrapedType, class LockType, class UseType>
class CScopedWrapWrite;

template<class WrapedType, class LockType, class UseType = WrapedType>
class CScopedWrapRead
{
	template<class WrapedT, class LockT, class UseT>
	friend class CScopedWrapWrite;
public:
	CScopedWrapRead(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock)
		: m_pData(&wrapLock.m_data), m_pLock(&wrapLock.m_rwLock)
	{
		m_pLock->LockRead();
	}

	CScopedWrapRead(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock, uint32_t msec)
		: m_pData(&wrapLock.m_data), m_pLock(&wrapLock.m_rwLock)
	{
		if (!m_pLock->TimedLockRead(msec)) {
			throw std::runtime_error("Read lock timeout!");
		}
	}

	CScopedWrapRead(CScopedWrapWrite<WrapedType, LockType, UseType>& scopedWrite)
		: m_pData(scopedWrite.m_pData), m_pLock(scopedWrite.m_pLock)
	{
		if (NULL == m_pLock) {
			throw std::runtime_error("Must lock write !");
		}
		if (!m_pLock->DegradeLockWrite()) {
			throw std::runtime_error("Degrade lock fail !");
		}
		scopedWrite.m_pData = NULL;
		scopedWrite.m_pLock = NULL;
	}

	~CScopedWrapRead()
	{
		if (NULL != m_pLock) {
			m_pData = NULL;
			m_pLock->Unlock();
			m_pLock = NULL;
		}
	}

	const UseType* operator->() const
	{
		return m_pData;
	}

	const UseType& operator*() const
	{
		return *m_pData;
	}

	inline void Lock(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock)
	{
		if (m_pLock == &wrapLock.m_rwLock) {
			return;
		}
		if (NULL != m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = &wrapLock.m_data;
		m_pLock = &wrapLock.m_rwLock;
		m_pLock->LockRead();
	}

	inline bool Lock(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock, uint32_t msec)
	{
		if (m_pLock == &wrapLock.m_rwLock) {
			return true;
		}
		if (NULL != m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = &wrapLock.m_data;
		m_pLock = &wrapLock.m_rwLock;
		return m_pLock->TimedLockRead(msec);
	}

	inline void Unlock()
	{
		if (NULL == m_pLock) {
			return;
		}
		m_pData = NULL;
		m_pLock->Unlock();
		m_pLock = NULL;
	}

	/** Degrade the write lock. */
	bool Degrade(CScopedWrapWrite<WrapedType, LockType, UseType>& scopedWrite)
	{
		if (NULL == scopedWrite.m_pLock) {
			return false;
		}

		if (!scopedWrite.m_pLock->DegradeLockWrite()) {
			return false;
		}

		if (NULL != m_pLock && m_pLock != scopedWrite.m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = scopedWrite.m_pData;
		m_pLock = scopedWrite.m_pLock;
		scopedWrite.m_pData = NULL;
		scopedWrite.m_pLock = NULL;
		return true;
	}

private:
	CScopedWrapRead(const CScopedWrapRead& orig) {}

	CScopedWrapRead& operator = (const CScopedWrapRead& right) { return *this; }

private:
	UseType* m_pData;
	IRWLock* m_pLock;
};

template<class WrapedType, class LockType, class UseType = WrapedType>
class CScopedWrapWrite
{
	template<class WrapedT, class LockT, class UseT>
	friend class CScopedWrapRead;
public:
	CScopedWrapWrite(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock)
		: m_pData(&wrapLock.m_data), m_pLock(&wrapLock.m_rwLock)
	{
		m_pLock->LockWrite();
	}

	CScopedWrapWrite(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock, uint32_t msec)
		: m_pData(&wrapLock.m_data), m_pLock(&wrapLock.m_rwLock)
	{
		if (!m_pLock->TimedLockWrite(msec)) {
			throw std::runtime_error("write lock timeout!");
		}
	}

	CScopedWrapWrite(CScopedWrapRead<WrapedType, LockType, UseType>& scopedRead)
		: m_pData(scopedRead.m_pData), m_pLock(scopedRead.m_pLock)
	{
		if (NULL == m_pLock) {
			throw std::runtime_error("Must lock read !");
		}
		m_pLock->UpgradeLockRead();
		scopedRead.m_pData = NULL;
		scopedRead.m_pLock = NULL;
	}

	CScopedWrapWrite(CScopedWrapRead<WrapedType, LockType, UseType>& scopedRead, uint32_t msec)
		: m_pData(scopedRead.m_pData), m_pLock(scopedRead.m_pLock)
	{
		if (NULL == m_pLock) {
			throw std::runtime_error("Must lock read !");
		}
		if (!m_pLock->UpgradeLockRead(msec)) {
			throw std::runtime_error("Upgrade lock timeout !");
		}
		scopedRead.m_pData = NULL;
		scopedRead.m_pLock = NULL;
	}

	~CScopedWrapWrite()
	{
		if (NULL != m_pLock) {
			m_pData = NULL;
			m_pLock->Unlock();
			m_pLock = NULL;
		}
	}

	UseType* operator->()
	{
		return m_pData;
	}

	UseType& operator*()
	{
		return *m_pData;
	}

	inline void Lock(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock)
	{
		if (m_pLock == &wrapLock.m_rwLock) {
			return;
		}
		if (NULL != m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = &wrapLock.m_data;
		m_pLock = &wrapLock.m_rwLock;
		m_pLock->LockWrite();
	}

	inline bool Lock(CWrapRWLock<WrapedType, LockType, UseType>& wrapLock, uint32_t msec)
	{
		if (m_pLock == &wrapLock.m_rwLock) {
			return true;
		}
		if (NULL != m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = &wrapLock.m_data;
		m_pLock = &wrapLock.m_rwLock;
		return m_pLock->TimedLockWrite(msec);
	}

	inline void Unlock()
	{
		if (NULL == m_pLock) {
			return;
		}
		m_pData = NULL;
		m_pLock->Unlock();
		m_pLock = NULL;
	}

	/** Upgrade the read lock. */
	void Upgrade(CScopedWrapRead<WrapedType, LockType, UseType>& scopedRead)
	{
		if (NULL == scopedRead.m_pLock) {
			return;
		}
		if (NULL != m_pLock && m_pLock != scopedRead.m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = scopedRead.m_pData;
		m_pLock = scopedRead.m_pLock;
		scopedRead.m_pData = NULL;
		scopedRead.m_pLock = NULL;
		m_pLock->UpgradeLockRead();
	}

	bool Upgrade(CScopedWrapRead<WrapedType, LockType, UseType>& scopedRead, uint32_t msec)
	{
		if (NULL == scopedRead.m_pLock) {
			return false;
		}

		if (!scopedRead.m_pLock->TimedUpgradeLockRead(msec)) {
			return false;
		}

		if (NULL != m_pLock && m_pLock != scopedRead.m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = scopedRead.m_pData;
		m_pLock = scopedRead.m_pLock;
		scopedRead.m_pData = NULL;
		scopedRead.m_pLock = NULL;
		return true;
	}

private:
	CScopedWrapWrite(const CScopedWrapWrite& orig) {}

	CScopedWrapWrite& operator = (const CScopedWrapWrite& right) { return *this; }

private:
	UseType* m_pData;
	IRWLock* m_pLock;
};

/** An abstract base class for synchronization primitives.
 */
template<class WrapedType, class LockType = CSpinRWLock, class UseType = WrapedType>
class CWrapRWLock
{
	template<class WrapedT, class LockT, class UseT>
	friend class CScopedWrapRead;

	template<class WrapedT, class LockT, class UseT>
	friend class CScopedWrapWrite;
public:
	typedef CScopedWrapRead<WrapedType, LockType, UseType> Read;
	typedef CScopedWrapWrite<WrapedType, LockType, UseType> Write;

	CWrapRWLock() : m_data() {}

	template<typename ...A>
	CWrapRWLock(A... args) : m_data(args ...) {}

	CWrapRWLock(const CWrapRWLock& orig)
		: m_data(orig.m_data) {
	}

	~CWrapRWLock() { assert(!m_rwLock.Using()); }

	CWrapRWLock& operator = (const CWrapRWLock& right) {
		m_data = right.m_data;
		return *this;
	}

private:
	WrapedType m_data;
	LockType m_rwLock;
};

} // namespace thd

#endif // WRAPRWLOCK_H

/* end of header file */
