/*
 * File:   IRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __IRWLOCK_H_
#define __IRWLOCK_H_

#include "Common.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
class SHARED_DLL_DECL IRWLock
{
  public:

  /** Destructor. */
  virtual ~IRWLock() {}

  /** Acquire a write lock, blocking until the unlockWrite is acquired. */
  virtual void LockWrite() throw() = 0;

  virtual bool TimedLockWrite(uint32_t msec) throw() = 0;

  /** Release the write lock. */
  virtual void UnlockWrite() throw() = 0;

  /** Acquire a read lock, blocking until the unlockRead is acquired. */
  virtual void LockRead() throw() = 0;

  virtual bool TimedLockRead(uint32_t msec) throw() = 0;

  /** Release the read lock. */
  virtual void UnlockRead() throw() = 0;

  /** Upgrade the read lock. */
  virtual void UpgradeLockRead() throw() = 0;

  virtual bool TimedUpgradeLockRead(uint32_t msec) throw() = 0;

  /** Degrade the write lock. */
  virtual bool DegradeLockWrite() throw() = 0;
  
  /** Whether be Using **/
  virtual bool Using() throw() = 0;
};

}; // namespace thd

#endif // __IRWLOCK_H_

/* end of header file */
