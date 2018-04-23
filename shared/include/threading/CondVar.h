
#ifndef __CONDVAR_H_
#define __CONDVAR_H_

#include "Common.h"

#if COMPILER == COMPILER_MICROSOFT
#include "SpinLock.h"
#else
#include <pthread.h>
#endif

namespace thd {

class CMutex;

/** A condition variable -- a synchronization mechanism used to
 * coordinate the actions of multiple threads. A condition variable
 * is <i>signaled</i> to indicate that the state of some shared
 * data has changed; one or more threads can <i>wait</i> on the
 * condition variable for this change to occur.
 *
 */

class SHARED_DLL_DECL CCondVar
{
  public:

  /** Construct a new <b>CCondVar</b>. */
  CCondVar() throw();

  /** Destructor. */
  ~CCondVar() throw();

  /** Wait for the condition variable to be signaled. The associated mutex
   * should be held by the thread at the time that it calls this method.
   *
   * @param mutex The mutex that is protecting the shared data; the
   * mutex will be automatically released during the wait and reacquired
   * when the wait completes.
   * @param msec The amount of time to wait, or <b>MSEC_FOREVER</b> to wait
   * indefinitely.
   * @return <b>true</b> if the condition variable was signaled,
   * <b>false</b> otherwise.
   */
  bool Wait(CMutex& mutex, uint32_t msec) throw();
  
  bool Wait(CMutex& mutex) throw();

  /** Signal one of the threads that are waiting on this condition
   * variable. If predictable scheduling behavior is required, the
   * associated mutex should be held by the thread at the time that it
   * calls this method.
   */
  void Notify() throw();

  /** Signal all threads that are waiting on this condition
   * variable.  If predictable scheduling behavior is required, the
   * associated mutex should be held by the thread at the time that
   * it calls this method.
   */
  void NotifyAll() throw();

  private:

#if COMPILER == COMPILER_MICROSOFT
  HANDLE m_sem;
  int m_waitersCount;
  CSpinLock m_waitersLock;
  HANDLE m_waitersDone;
  bool m_broadcast;
#else
  pthread_cond_t m_cond;
#endif

 private:
	CCondVar(const CCondVar& orig) {}
	CCondVar& operator=(const CCondVar& right) { return *this; }
};

}; // namespace thd

#endif // __CONDVAR_H_

/* end of header file */
