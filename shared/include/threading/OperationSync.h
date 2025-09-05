/* 
 * File:   OperationSync.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef OPERATION_SYNC_H
#define	OPERATION_SYNC_H

#include "AtomicLock.h"
#include "OperationBase.h"
#include "MessageData.h"
#include "UniquePointer.h"
#include "SysTickCount.h"
#include "WrapControl.h"
#include "SpinLock.h"
#include "ScopedLock.h"

namespace thd {

	template<typename T>
	class COperationSync : public COperationBase<T>
	{
	public:
		COperationSync(util::CUniquePointer<CMessageBase<T> >& data)
			: m_data(data.Release()),
			m_type(eOperationType::OPERATION_ONCE),
			m_flag(true),
			m_processed(false) {}

		~COperationSync() {
			atomic_xchg8(&m_flag, false);
		}

		virtual eOperationType OperationType() override {
			return m_type;
		}

		virtual bool Preprocess() override {
			if (m_data.IsInvalid()) {
				return false;
			}
			m_type = m_data->OperationType();
			return true;
		}

		virtual bool ReadOnly() const override {
			if (m_data.IsInvalid()) {
				return true;
			}
			return m_data->ReadOnly();
		}

		virtual void Process(util::CWeakPointer<T> object) override {

			if (m_data->ReadOnly()) {
				m_data->Process(util::CWeakPointer<const T>(object));
			} else {
				m_data->Process(object);
			}

			if (eOperationType::OPERATION_ONCE == m_type) {
				m_type = m_data->OperationType();
				if (m_type == eOperationType::OPERATION_REPEAT) {
					return;
				}
			}
			m_type = eOperationType::OPERATION_ONCE;
			atomic_xchg8(&m_flag, false);
		}

		virtual bool IsProcessed() const override {
			return m_processed;
		}

		inline void SetProcessed(bool v) {
			atomic_xchg8(&m_processed, v);
		}

		volatile const bool& GetFlag() const {
			return m_flag;
		}

		eWaitingResult Waiting() {
			// wait
			for(int i = 0; m_flag; ++i) {
				cpu_relax(i);
			}
			return WAITING_SUCCESS;
		}

		eWaitingResult Waiting(int msTime) {
			// wait
			uint64_t startTime = GetSysTickCount();
			for (int i = 0; m_flag; ++i) {
				cpu_relax(i);
				if(static_cast<int>(GetSysTickCount() - 
					startTime) >= msTime)
				{
					return WAITING_TIMEOUT;
				}
			}
			return WAITING_SUCCESS;
		}

		util::CUniquePointer<CMessageBase<T> >& GetData() {
			return m_data;
		}

	private:
		util::CUniquePointer<CMessageBase<T> > m_data;
		eOperationType m_type;
		volatile bool m_flag;
		volatile bool m_processed;
	};
} // end namespace 

#endif /* OPERATION_SYNC_H */