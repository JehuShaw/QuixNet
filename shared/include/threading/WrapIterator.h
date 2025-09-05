/* 
 * File:   WrapIterator.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef WRAP_ITERATOR_H
#define	WRAP_ITERATOR_H

#include <stdint.h>
#include "IteratorBase.h"
#include "SpinLock.h"
#include "ScopedLock.h"
#include "SysTickCount.h"

namespace thd {

#define ITERATOR_KEEP_THREAD_COUNT 1

	template<typename T>
	class CWrapIterator : public CIteratorBase<T>
	{
		typedef util::WrapPoolHas<std::vector<CIteratorContext<T> > > iteratorContextsType;
	public:
		CWrapIterator(eOperationType type, int synchroMaxNum = ITERATOR_SYNCHRO_INFINITY) 
		    : m_size(0), m_count(0), m_roundCount(1), m_type(type),
			m_synchroMaxNum(synchroMaxNum), m_runningCount(0), m_start(false), m_ready(false)
			m_restartContexts(new iteratorContextsType), m_restartContexts(NULL)
		{
			if (m_type != eOperationType::OPERATION_REPEAT) {
				atomic_xchg8(&m_flag, true);
			}
		}

		~CWrapIterator() {
			if (m_type != eOperationType::OPERATION_REPEAT) {
				atomic_xchg8(&m_flag, false);
			}
		}

		virtual void Start() override {
			atomic_xchg8(&m_ready, true); 
		}

		void Wait() const {
			atomic_xchg8(&m_ready, true); 
			// wait
			for(int i = 0; m_flag; ++i) {
				cpu_relax(i);
			}
		}

		bool Wait(uint32_t msTime) const {
			atomic_xchg8(&m_ready, true); 
			// wait
			uint64_t startTime = GetSysTickCount();
			for (int i = 0; m_flag; ++i) {
				cpu_relax(i);
				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msTime)
				{
					return false;
				}
			}
			return true;
		}

	protected:

		virtual eOperationType OperationType() override {
			return m_type;
		}

		virtual eStartResult OnStart(uint32_t& roundNum) override {
			if (m_roundCount != roundNum) {
				roundNum = m_roundCount;
				if (static_cast<bool>(atomic_xchg8(&m_start, true)) == false) {
					return START_SUCCESS;
				}
				return START_PROCESSED;
			}
			return START_FAIL;
		}

		virtual bool OnFinish(util::CAutoPointer<CMessageBase<T> > &msg,
			util::CWeakPointer<T>& object) override {
			if (m_type != eOperationType::OPERATION_REPEAT) {
				if (CheckComplated()) {
					return true;
				}
				return false;
			}

			if (CheckComplated()) {
				CScopedLock lockFinish(m_finishContextsLock);
				CScopedLock lockRestart(m_restartContextsLock);
				m_restartContexts.Reset(m_finishContexts.Release());
				m_finishContexts.Reset(new iteratorContextsType);
				return true;
			}
			util::CAutoPointer<CMessageIterator<T> > msgIterator(msg);
			if (msg.IsInvalid()) {
				throw std::runtime_error("Must be the instance of CMessageIterator!");
			} else {
				CScopedLock lockFinish(m_finishContextsLock);
				m_finishContexts->push_back(msgIterator->GetIteratorContext());
			}
			return false;
		}

	private:
		template<typename objectT> friend class COperationIterate;

		bool CheckComplated() {
			int count = atomic_inc(&m_count);
			for (int i = 0; !m_ready && m_size > 0; ++i) {
				if (count != m_count) {
					return false;
				}
				cpu_relax(i);
			}
			int size = m_size;
			do {
				if (count < size || size <= 0) {
					return false;
				}
				if (atomic_cmpxchg(&m_count, count, 0) == count) {
					break;
				}
				count = m_count;
				size = m_size;
			} while (true);
			return true;
		}

		virtual void IncreaseSize() override {
			atomic_inc(&m_size);
		}

		virtual void DecreaseSize() override {
			atomic_dec(&m_size);
		}

		virtual void SetupCompleted() override {
			atomic_xchg8(&m_start, false);
			atomic_inc(&m_roundCount);
			if (m_type != eOperationType::OPERATION_REPEAT) {
				atomic_xchg8(&m_flag, false);
				atomic_xchg(&m_size, 0);
				atomic_xchg8(&m_ready, false);
				return;
			}
			atomic_xchg8(&m_ready, false);
			CScopedLock lockRestart(m_restartContextsLock);
			if (!m_restartContexts->empty()) {
				iteratorContextsType::iterator it(m_restartContexts->begin());
				for (; m_restartContexts->end() != it; ++it) {
					it->Restart();
				}
				atomic_xchg8(&m_ready, true);
				m_restartContexts->clear();
			}
		}

		virtual bool IncreaseRunningCount() override {
			int synchroMaxNum = m_synchroMaxNum;
			int count;
			do {
				count = m_runningCount;
				if (count > synchroMaxNum && synchroMaxNum != ITERATOR_SYNCHRO_INFINITY) {
					return false;
				}
			} while (atomic_cmpxchg(&m_runningCount, count, count + 1) != count);
			return true;
		}

		virtual void DecreaseRunningCount() override {
			int count;
			do {
				count = m_runningCount;
				if (count <= 0) {
					return;
				}
			} while (atomic_cmpxchg(&m_runningCount, count, count - 1) != count);
		}

	private:
		CWrapIterator(const CWrapIterator &orig)
			: m_size(0), m_count(0), m_roundCount(0),
			m_type(eOperationType::OPERATION_ONCE), m_synchroMaxNum(orig.m_synchroMaxNum),
			m_runningCount(0), m_start(false), m_ready(false) {}

		CWrapIterator &operator=(const CWrapIterator& right) { return *this; }

	private:
		volatile int m_size;
		volatile int m_count;
		volatile uint32_t m_roundCount;
		const eOperationType m_type;
		const int m_synchroMaxNum;
		volatile int m_runningCount;
		volatile bool m_start;
		volatile bool m_ready;
		volatile bool m_flag;
		thd:CSpinLock m_finishContextsLock;
		util:CUniquePointer<iteratorContextsType> m_finishContexts;
		thd::CSpinLock m_restartContextsLock;
		util:CUniquePointer<iteratorContextsType> m_restartContexts;
	};

}

#endif /* WRAP_ITERATOR_H */
