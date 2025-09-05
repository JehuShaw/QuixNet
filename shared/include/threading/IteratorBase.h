/* 
 * File:   IteratorBase.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef ITERATOR_BASE_H
#define	ITERATOR_BASE_H

#include "MessageData.h"

namespace thd {

#define ITERATOR_SYNCHRO_NUM_MIN 1
#define ITERATOR_SYNCHRO_INFINITY -1

	enum eStartResult {
		START_FAIL,
		START_SUCCESS,
		START_PROCESSED,
	};

	template <typename T>
	class CIteratorBase
	{
	public:
		virtual ~CIteratorBase() {}

		virtual void Start() = 0;

	protected:
		template<typename ObjectT> friend class COperationIterate;

		virtual eOperationType OperationType() = 0;

		virtual eStartResult OnStart(uint32_t& roundNum) = 0;

		virtual bool OnFinish(util::CAutoPointer<CMessageBase<T> > &msg,
			util::CWeakPointer<T>& object) = 0;

		virtual void SetupCompleted() = 0;
		
		virtual void IncreaseSize() = 0;

		virtual void DecreaseSize() = 0;

		virtual bool IncreaseRunningCount() = 0;

		virtual void DecreaseRunningCount() = 0;

	};

}

#endif /* ITERATOR_BASE_H */
