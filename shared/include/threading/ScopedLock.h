/*
 * File:   ScopedLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef SCOPEDLOCK_H
#define SCOPEDLOCK_H

#include <stddef.h>
#include "ILock.h"

namespace thd {

class CScopedLock
{
  public:

  /** Construct a new CScopedock for the given Lock. The Lock is locked
   * immediately.
   */
  explicit CScopedLock(ILock& lock) throw()
    : m_lock(&lock)
  {
    m_lock->Lock();
  }

  explicit CScopedLock(const ILock& lock) throw()
	  : m_lock(const_cast<ILock*>(&lock))
  {
	  m_lock->Lock();
  }

  /** Destructor. Unlocks the Lock. */
  ~CScopedLock() noexcept
  {
	  if (m_lock == NULL) {
		  return;
	  }
	  m_lock->Unlock();
  }

  void Unlock() throw() {
	  if (m_lock == NULL) {
		  return;
	  }
	  m_lock->Unlock();
	  m_lock = NULL;
  }

private:
  ILock* m_lock;

  CScopedLock(const CScopedLock& orig);
  CScopedLock& operator=(const CScopedLock &orig);
};

}; // namespace thd


#endif // SCOPEDLOCK_H

/* end of header file */
