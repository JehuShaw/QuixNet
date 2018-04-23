/*
 * File:   ILock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __ILOCK_H_
#define __ILOCK_H_

#include "Common.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
class SHARED_DLL_DECL ILock
{
  public:

  /** Destructor. */
  virtual ~ILock() { }

  /** %Lock operation. */
  virtual void Lock() throw() = 0;

  virtual bool TimedLock(uint32_t msec) throw() = 0;

  /** Unlock operation. */
  virtual void Unlock() throw() = 0;

  /** Whether be Using **/
  virtual bool Using() throw() = 0;
};

}; // namespace thd

#endif // __ILOCK_H_

/* end of header file */
