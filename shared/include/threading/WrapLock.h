/*
 * File:   WrapLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef WRAPLOCK_H
#define WRAPLOCK_H

#include "SpinLock.h"
#include <stdexcept>

namespace thd {

template<class WrapedType, class LockType,  class UseType>
class CWrapLock;

template<class WrapedType, class LockType, class UseType = WrapedType>
class CScopedWrapLock
{
public:
	CScopedWrapLock(CWrapLock<WrapedType, LockType, UseType>& wrapLock)
		: m_pData(&wrapLock.m_data), m_pLock(&wrapLock.m_lock)
	{
		m_pLock->Lock();
	}

	CScopedWrapLock(CWrapLock<WrapedType, LockType, UseType>& wrapLock, uint32_t msec)
		: m_pData(&wrapLock.m_data), m_pLock(&wrapLock.m_lock)
	{
		if (!m_pLock->TimedLock(msec)) {
			throw std::runtime_error("lock timeout!");
		}
	}

	~CScopedWrapLock()
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

	const UseType* operator->() const
	{
		return m_pData;
	}

	UseType& operator*()
	{
		return *m_pData;
	}

	const UseType& operator*() const
	{
		return *m_pData;
	}

	inline void Lock(CWrapLock<WrapedType, LockType, UseType>& wrapLock)
	{
		if (m_pLock == &wrapLock.m_lock) {
			return;
		}
		if (NULL != m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = &wrapLock.m_data;
		m_pLock = &wrapLock.m_lock;
		m_pLock->Lock();
	}

	inline bool Lock(CWrapLock<WrapedType, LockType, UseType>& wrapLock, uint32_t msec)
	{
		if (m_pLock == &wrapLock.m_lock) {
			return true;
		}
		if (NULL != m_pLock) {
			m_pLock->Unlock();
		}
		m_pData = &wrapLock.m_data;
		m_pLock = &wrapLock.m_lock;
		return m_pLock->TimedLock(msec);
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

private:
	CScopedWrapLock(const CScopedWrapLock& orig) {}

	CScopedWrapLock& operator = (const CScopedWrapLock& right) { return *this; }

private:
	UseType* m_pData;
	ILock* m_pLock;
};

/** An abstract base class for synchronization primitives.
 */
template<class WrapedType, class LockType = CSpinLock, class UseType = WrapedType>
class CWrapLock
{
	template<class WrapedT, class LockT, class UseT>
	friend class CScopedWrapLock;
public:
	typedef CScopedWrapLock<WrapedType, LockType, UseType> Write;

	CWrapLock() : m_data() {}

	template<typename ...A>
	CWrapLock(A... args) : m_data(args ...) {}

	CWrapLock(const CWrapLock& orig)
		: m_data(orig.m_data) {}

	~CWrapLock() { assert(!m_lock.Using()); }

	CWrapLock& operator = (const CWrapLock& right) {
		m_data = right.m_data;
		return *this;
	}

private:
	WrapedType m_data;
	LockType m_lock;
};

}; // namespace thd

#endif // WRAPLOCK_H

/* end of header file */
