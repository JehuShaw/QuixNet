/*
 * File:   ScopedLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __SCOPEDLOCK_H_
#define __SCOPEDLOCK_H_

#include "ILock.h"

namespace thd {

class CScopedLock
{
  public:

  /** Construct a new CScopedock for the given Lock. The Lock is locked
   * immediately.
   */
  explicit CScopedLock(ILock& lock) throw()
    : m_lock(lock)
  {
    m_lock.Lock();
  }

  explicit CScopedLock(const ILock& lock) throw()
	  : m_lock(const_cast<ILock&>(lock))
  {
	  m_lock.Lock();
  }

  /** Destructor. Unlocks the Lock. */
  ~CScopedLock() throw()
  {
    m_lock.Unlock();
  }

private:
  ILock& m_lock;

  CScopedLock(const CScopedLock& orig);
  CScopedLock& operator=(const CScopedLock &orig);
};

}; // namespace thd


#endif // __SCOPEDLOCK_H_

/* end of header file */
