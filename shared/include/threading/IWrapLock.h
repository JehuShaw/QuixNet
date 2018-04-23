/*
 * File:   IWrapLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __IWRAPLOCK_H_
#define __IWRAPLOCK_H_

#include "ILock.h"
#include "IWrapBase.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
class IWrapLock : virtual protected ILock, virtual public IWrapBase
{
public:
	virtual ~IWrapLock() {}

	template<class DataType> friend class CScopedWrapLock;
};

}; // namespace thd

#endif // __IWRAPLOCK_H_

/* end of header file */
