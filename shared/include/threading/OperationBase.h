/* 
 * File:   OperationBase.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef OPERATION_BASE_H
#define	OPERATION_BASE_H

#include "MessageData.h"

namespace thd {

	template<typename T>
	class COperationBase
	{
	public:
		virtual ~COperationBase() {}

		virtual eOperationType OperationType() {
			return eOperationType::OPERATION_ONCE;
		}

		virtual bool Preprocess() {
			return true;
		}

		virtual bool ReadOnly() const = 0;

		virtual void Process(util::CWeakPointer<T> object) = 0;

		virtual bool IsProcessed() const {
			return false;
		}
	};
} // end namespace 

#endif /* OPERATION_BASE_H */
