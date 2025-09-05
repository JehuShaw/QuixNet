/*
 * File:   IEnvLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef IENVLOCK_H
#define IENVLOCK_H

#include "ILock.h"
#include "IEnvLockBase.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
class IEnvLock : virtual protected ILock, virtual public IEnvLockBase
{
public:
	virtual ~IEnvLock() {}

	template<class DataType> friend class CScopedEnvLock;
};

}; // namespace thd

#endif // IENVLOCK_H

/* end of header file */
