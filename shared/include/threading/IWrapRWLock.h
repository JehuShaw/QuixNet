/*
 * File:   IWrapRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __IWRAPRWLOCK_H_
#define __IWRAPRWLOCK_H_

#include "IRWLock.h"
#include "IWrapBase.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
class IWrapRWLock : virtual protected IRWLock, virtual public IWrapBase
{
public:
	virtual ~IWrapRWLock() {}

	template<class DataType> friend class CScopedWrapRWLock;
};

}; // namespace thd

#endif // __IWRAPRWLOCK_H_

/* end of header file */
