/* 
 * File:   WrapIteratorParent.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef WRAP_ITERATOR_PARENT_H
#define	WRAP_ITERATOR_PARENT_H

#include "WrapIterator.h"
#include "ReferObject.h"

namespace thd {

	template<typename T>
	class CWrapIteratorParent : public CWrapIterator<T>
	{
	public:
		CWrapIteratorParent(int synchroMaxNum = ITERATOR_SYNCHRO_INFINITY) 
		    : CWrapIterator<T>(eOperationType::OPERATION_ONCE, synchroMaxNum), m_childFinishCount(0)
		{
			m_pThis(this);
		}

		~CWrapIteratorParent() {
		}

	protected:
		template<typename objectT> friend class CWrapIteratorChild;

		virtual int ChildSize() const = 0;

		virtual bool OnChildFinish() {
			if (CheckParentComplated()) {
				return true;
			}
			return false;
		}

	private:
		CWrapIteratorParent(const CWrapIteratorParent &orig) : m_childFinishCount(0) {}

		CWrapIteratorParent &operator=(const CWrapIteratorParent& right) { return *this; }

		bool CheckParentComplated()
		{
			int size = ChildSize();
			int count = atomic_inc(&m_childFinishCount);
			do {
				if (count < size || size <= 0) {
					return false;
				}
				if (atomic_cmpxchg(&m_childFinishCount, count, 0) == count) {
					break;
				}
				count = m_childFinishCount;
			} while (true);
			return true;
		}

	private:
		volatile int m_childFinishCount;

	protected:
		util::CReferObject<CWrapIteratorParent> m_this;
	};

}

#endif /* WRAP_ITERATOR_PARENT_H */
