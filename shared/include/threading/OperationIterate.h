/* 
 * File:   OperationIterate.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef OPERATION_ITERATE_H
#define	OPERATION_ITERATE_H

#include "AtomicLock.h"
#include "OperationBase.h"
#include "MessageData.h"
#include "AutoPointer.h"
#include "IteratorBase.h"

namespace thd {

	template<typename T>
	class COperationIterate : public COperationBase<T>
	{
	public:
		COperationIterate(typename util::CAutoPointer<CIteratorBase<T> >& iterator,
						  typename util::CUniquePointer<CMessageBase<T> >& data)
			: m_iterator(iterator),
			m_data(data.Release()),
			m_type(eOperationType::OPERATION_REPEAT),
			m_roundNum(0)
		{
			m_iterator->IncreaseSize();
			util::CAutoPointer<CMessageIterator<T> > subData(m_data);
			subData->SetIterator(m_iterator);

		}

		~COperationIterate() {
			if (m_iterator->OperationType() == eOperationType::OPERATION_REPEAT) {
				m_iterator->DecreaseSize();
			}
		}

		virtual eOperationType OperationType() override {
			return m_type;
		}

		virtual bool Preprocess() override {
			if (m_data.IsInvalid()) {
				return false;
			}
			m_type = m_data->OperationType();
			m_iterator->OnStart(m_roundNum);
			return true;
		}

		virtual bool ReadOnly() const override {
			if (m_data.IsInvalid()) {
				return true;
			}
			return m_data->ReadOnly();
		}

		virtual void Process(util::CWeakPointer<T> object) override {

			if (!m_iterator->IncreaseRunningCount()) {
				m_type = eOperationType::OPERATION_REPEAT;
				return;
			}

			if (m_data->ReadOnly()) {
				m_data->Process(util::CWeakPointer<const T>(object));
			} else {
				m_data->Process(object);
			}

			if (eOperationType::OPERATION_ONCE == m_type) {
				m_type = m_data->OperationType();
				if (m_type == eOperationType::OPERATION_REPEAT) {
					m_iterator->DecreaseRunningCount();
					return;
				}
			} else {
				m_type = eOperationType::OPERATION_ONCE;
			}
			m_iterator->DecreaseRunningCount();
			if (m_iterator->OnFinish(m_data, object)) {
				m_iterator->SetupCompleted();
				if (m_iterator->OperationType() == eOperationType::OPERATION_REPEAT) {
					m_type = eOperationType::OPERATION_REPEAT;
				}
			}
		}

	private:
		COperationIterate(const COperationIterate &orig)
			: m_iterator(orig.m_iterator), m_data(orig.m_data),
			m_type(orig.m_type), m_roundNum(0) {}

		COperationIterate & operator= (const COperationIterate &right) { return *this; }

	private:
		util::CAutoPointer<CIteratorBase<T> > m_iterator;
		util::CAutoPointer<CMessageBase<T> > m_data;
		eOperationType m_type;
		uint32_t m_roundNum;
	};
} // end namespace 

#endif /* OPERATION_ITERATE_H */