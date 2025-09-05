/*
 * File:   IEnvRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef IENVRWLOCK_H
#define IENVRWLOCK_H

#include "IRWLock.h"
#include "IEnvLockBase.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
class IEnvRWLock : virtual protected IRWLock, virtual public IEnvLockBase
{
public:
	virtual ~IEnvRWLock() {}

	template<class DataType> friend class CScopedEnvRWLock;
};

}; // namespace thd

#endif // IENVRWLOCK_H

/* end of header file */
