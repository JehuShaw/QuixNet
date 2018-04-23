
#ifndef __READWRITELOCK_H_
#define __READWRITELOCK_H_

#include "Common.h"
#include "IRWLock.h"
#include "CondVar.h"
#include "Mutex.h"


namespace thd {

/** A Read/Write lock -- a synchronization primitive that allows multiple
 * threads to coordinate access to a mutable resource. Any number of threads
 * can simultaneously hold the read lock as long as no thread holds the
 * write lock. Conversely, a thread can acquire the write lock as long as
 * no threads are holding the read lock.
 *
 * See also ScopedReadLock and ScopedWriteLock.
 *
 */

class SHARED_DLL_DECL CReadWriteLock : public IRWLock
{
  public:

  /** Construct a new ReadWriteLock. */
  CReadWriteLock() throw();

  /** Destructor. */
  ~CReadWriteLock() throw();

   /** Acquire a write lock, blocking until the lock is acquired. */
  void LockWrite() throw();

  /** Try to acquire a write lock, returning if the lock could not
   * be acquired within the given amount of time. On platforms that
   * do not support timed read/write locks, the method returns
   * immediately if the lock could not be acquired.
   *
   * @param msec The timeout, in milliseconds.
   * @return <b>true</b> if the lock was acquired, <b>false</b> otherwise.
   */
  bool TimedLockWrite(uint32_t msec) throw();

  /** Release the write lock. */
  void UnlockWrite() throw();

  /** Acquire a read lock, blocking until the lock is acquired. */
  void LockRead() throw();

  /** Try to acquire a read lock, returning if the lock could not be
   * acquired within the given amount of time. On platforms that do
   * not support timed read/write locks, the method returns
   * immediately if the lock could not be acquired.
   *
   * @param msec The timeout, in milliseconds.
   * @return <b>true</b> if the lock was acquired, <b>false</b> otherwise.
   */
  bool TimedLockRead(uint32_t msec) throw();

  /** Release the read lock. */
  void UnlockRead() throw();

  /** Upgrade the read lock. */
  void UpgradeLockRead() throw();

  bool TimedUpgradeLockRead(uint32_t msec) throw();

  /** Degrade the write lock. */
  bool DegradeLockWrite() throw();

  bool Using() throw();

  private:
	int m_readersReading;
	int m_writerWriting;
	CMutex m_mutex;
	CCondVar m_lockFree;

private:
	CReadWriteLock(const CReadWriteLock& orig) {}
	CReadWriteLock& operator=(const CReadWriteLock& right) { return *this; }
};

}; // namespace thd

#endif // __READWRITELOCK_H_

/* end of source file */
