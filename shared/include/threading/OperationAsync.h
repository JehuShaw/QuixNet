/* 
 * File:   OperationAsync.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef OPERATION_ASYNC_H
#define	OPERATION_ASYNC_H

#include "UniquePointer.h"
#include "MessageData.h"
#include "OperationBase.h"

namespace thd {
	template<typename T>
	class COperationAsync : public COperationBase<T>
	{
	public:
		COperationAsync(util::CUniquePointer<CMessageBase<T> >& data)
			: m_data(data.Release()) {
		}

		virtual eOperationType OperationType() override {
			if (m_data.IsInvalid()) {
				return eOperationType::OPERATION_ONCE;
			}
			return m_data->OperationType();
		}

		virtual bool ReadOnly() const override {
			if (m_data.IsInvalid()) {
				return true;
			}
			return m_data->ReadOnly();
		}

		virtual void Process(util::CWeakPointer<T> object) override {
			if (m_data.IsInvalid()) {
				return;
			}
			if (m_data->ReadOnly()) {
				m_data->Process(util::CWeakPointer<const T>(object));
			} else {
				m_data->Process(object);
			}
		}

	private:
		util::CUniquePointer<CMessageBase<T> > m_data;
	};
}  // end namespace 

#endif /* OPERATION_ASYNC_H */
