/*
 * File:   ReferObject.h
 * Author: Jehu Shaw
 *
 */

#ifndef _REFEROBJECT_H_
#define _REFEROBJECT_H_

#include <assert.h>
#include "WeakPointer.h"

namespace util {
#define REFER_MAX_WAIT_COUNT 60000
// This is base class for which want to get weak pointer.
template<class SubclassType>
class CReferObject {
public:
	// deconstruction
	~CReferObject() {
		WaitUseClear();
	}
	void operator()(const SubclassType* pObject) {
		if(NULL == pObject) {
			throw std::logic_error("The instance can't be NULL!");
		}
		m_pObject.SetRawPointer(pObject, false);
	}
	// get weak pointer
	CWeakPointer<SubclassType> operator()() const {
		assert(!m_pObject.IsInvalid());
		return m_pObject;
	}
private:
	void WaitUseClear() {
		// Don't get pointer.
		m_pObject.SetRawPointer(NULL, false);
		// if anywhere use wait it.
		for(unsigned int i = 0 ;; ++i) {
			if(i >= REFER_MAX_WAIT_COUNT) {
				throw std::logic_error("Wait too long !");
			}
			if(m_pObject.PeekUse() < 1) {
				break;
			}
			cpu_relax(i);
		}
	}
private:
	CAutoPointer<SubclassType> m_pObject;
};

} // end namespace util

#endif /* _REFEROBJECT_H_ */
