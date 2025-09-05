/* 
 * File:   WrapIteratorChild.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef WRAP_ITERATOR_CHILD_H
#define	WRAP_ITERATOR_CHILD_H

#include "WrapIterator.h"
#include "WrapIteratorParent.h"

namespace thd {

	template<typename T>
	class CWrapIteratorChild : public CWrapIterator<T>
	{
	public
		CWrapIteratorChild(CWeakPointer<CWrapIteratorParent<T> > parent, int synchroMaxNum = ITERATOR_SYNCHRO_INFINITY) 
		    : CWrapIterator<T>(eOperationType::OPERATION_ONCE, synchroMaxNum), m_parent(parent)
		{
		}

		~CWrapIteratorChild() {
		}

		virtual bool OnFinish(util::CAutoPointer<CMessageBase<T> > &msg,
			util::CWeakPointer<T>& object) override
		{
			if (!CWrapIterator<T>::OnFinish(msg, object)) {
				return false;
			}
			OnParentFinish();
			return true;
		}

		bool OnParentFinish() {
			util::CAutoPointer<CWrapIteratorParent> parent(m_parent.GetStrong());
			if (parent.IsInvalid()) {
				return false;
			}
			parent->OnChildFinish();
			return true;
		}

	private:
		CWrapIteratorChild(const CWrapIteratorChild &orig) {}

		CWrapIteratorChild &operator=(const CWrapIteratorChild& right) { return *this; }

	private:
		CWeakPointer<CWrapIteratorParent<T> > m_parent;
	};

}

#endif /* WRAP_ITERATOR_CHILD_H */
