/* 
 * File:   MessageData.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef MESSAGE_DATA_H
#define	MESSAGE_DATA_H

#include "UniquePointer.h"
#include "AutoPointer.h"
#include "WeakPointer.h"


namespace thd {

	enum class eSendResult {
		SEND_SUCCESS,
		SEND_FAIL,
		SEND_TIMEOUT,
		SEND_EXIT,
	};

	enum class eOperationType
	{
		OPERATION_ONCE,
		OPERATION_REPEAT,
	};

	template<typename T>
	class COperationBase;

	template<typename T>
	class CIteratorBase;

	template<typename T>
	class CMessageBase;

	template<typename T>
	class ICorrespondent
	{
	protected:
		template<typename ObjectT> friend class CMessageSender;
		template<typename ObjectT> friend class CIteratorContext;

		virtual eSendResult InnerSend(util::CAutoPointer<ICorrespondent<T> >& sender,
			util::CUniquePointer<CMessageBase<T> >& msg,
			bool emergency = true) const = 0;

		virtual void TriggerOperation(util::CAutoPointer<COperationBase<T> >& operation) = 0;
	};

	template<typename T>
	class CMessageBase
	{
	public:
		virtual ~CMessageBase() {}

		virtual eOperationType OperationType() = 0;

		virtual bool ReadOnly() const = 0;
		
		virtual void Process(util::CWeakPointer<T> object) = 0;

		virtual void Process(util::CWeakPointer<const T> object) = 0;
	};

	template<typename T>
	class CWriteData : public CMessageBase<T>
	{
	public:
		~CWriteData() override {}

		virtual eOperationType OperationType() override {
			return eOperationType::OPERATION_ONCE;
		}

		virtual bool ReadOnly() const override {
			return false;
		}

	private:
		virtual void Process(util::CWeakPointer<const T> object) final {
			throw std::runtime_error("Can not call the Process(util::CWeakPointer<const T> object) function.");
		}
	};

	template<typename T>
	class CReadData : public CMessageBase<T>
	{
	public:
		~CReadData() override {}

		virtual eOperationType OperationType() override {
			return eOperationType::OPERATION_ONCE;
		}

		virtual bool ReadOnly() const override {
			return true;
		}

	private:
		virtual void Process(util::CWeakPointer<T> object) final {
			throw std::runtime_error("Can not call the Process(util::CWeakPointer<T> object) function.");
		}
	};

	template<typename T>
	class CMessageSender : public CMessageBase<T>
	{
	protected:
		CMessageSender(const util::CAutoPointer<ICorrespondent<T> >& sender)
			: m_sender(sender) {}

		~CMessageSender() override {}

		virtual eOperationType OperationType() override {
			return eOperationType::OPERATION_ONCE;
		}

		template<typename subMsgT>
		eSendResult Send(const util::CAutoPointer<ICorrespondent<T> >& receiver,
			util::CUniquePointer<subMsgT>& subMsg,
			bool emergency = true) const
		{
			if (receiver.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			if (subMsg.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			if (receiver == m_sender) {
				throw std::runtime_error("Do not send self.");
				return eSendResult::SEND_FAIL;
			}

			if (dynamic_cast<CMessageBase<T> *>(this) == NULL) {
				throw std::runtime_error("Do not use this Send function out the message subclass,"
					" please use the CWrapObject Send instead.");
				return eSendResult::SEND_FAIL;
			}

			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			eSendResult ret = receiver->InnerSend(m_sender, msg, emergency);
			subMsg = msg;
			return ret;
		}


		eSendResult Send(const util::CAutoPointer<ICorrespondent<T> >& receiver,
			util::CUniquePointer<CMessageBase<T> >& msg,
			bool emergency = true) const
		{
			if (receiver.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			if (msg.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			if (receiver == m_sender) {
				throw std::runtime_error("Do not send self.");
				return eSendResult::SEND_FAIL;
			}

			if (dynamic_cast<CMessageBase<T> *>(this) == NULL) {
				throw std::runtime_error("Do not use this Send function out the message subclass,"
					" please use the CWrapObject Send instead.");
				return eSendResult::SEND_FAIL;
			}

			return receiver->InnerSend(m_sender, msg, emergency);
		}

	protected:
		util::CWeakPointer<ICorrespondent<T> > m_sender;
	};

	template<typename T>
	class CSenderWriteData : public CMessageSender<T> {
	protected:
		CSenderWriteData(const util::CAutoPointer<ICorrespondent<T> >& sender)
			: CMessageSender<T>(sender) {}

		~CSenderWriteData() override {}

		virtual bool ReadOnly() const override {
			return false;
		}

	private:
		virtual void Process(util::CWeakPointer<const T> object) final {
			throw std::runtime_error("Can not call the Process(util::CWeakPointer<const T> object) function.");
		}
	};

	template<typename T>
	class CSenderReadData : public CMessageSender<T> {
	protected:
		CSenderReadData(const util::CAutoPointer<ICorrespondent<T> >& sender)
			: CMessageSender<T>(sender) {}

		~CSenderReadData() override {}

		virtual bool ReadOnly() const override {
			return true;
		}

	private:
		virtual void Process(util::CWeakPointer<T> object) final {
			throw std::runtime_error("Can not call the Process(util::CWeakPointer<T> object) function.");
		}
	};


	template<typename T>
	class CIteratorContext {
	public:
		CIteratorContext(const util::CWeakPointer<ICorrespondent<T> >& correspondent,
			const util::CWeakPointer<COperationBase<T> >& operation)
			: m_correspondent(correspondent.GetStrong()), m_operation(operation.GetStrong()) {
		}

		bool Restart() {
			if (m_correspondent.IsInvalid()) {
				return false;
			}
			if (m_operation.IsInvalid()) {
				return false;
			}
			if (m_operation->OperationType() != eOperationType::OPERATION_REPEAT) {
				return false;
			}
			m_correspondent->TriggerOperation(m_operation);
			return true;
		}

	private:
		util::CAutoPointer<ICorrespondent<T> > m_correspondent;
		util::CAutoPointer<COperationBase<T> > m_operation;
	};

	template<typename T>
	class CMessageIterator : public CMessageSender<T>
	{
		void SetIterator(const util::CAutoPointer<CIteratorBase<T> > &iterator) {
			m_iterator = iterator;
		}
	protected:
		CMessageIterator(const util::CAutoPointer<ICorrespondent<T> >& correspondent)
			: CMessageSender<T>(correspondent) {

		}

		~CMessageIterator() override {}
		
		CIteratorContext<T> GetIteratorContext() const {
			return CIteratorContext<T>(CMessageSender<T>::m_wrapObject, m_operation);
		}
	private:
		util::CAutoPointer<CIteratorBase<T> > m_iterator;
		util::CWeakPointer<COperationBase<T> > m_operation;
	};

	template<typename T>
	class CIteratorWriteData : public CMessageIterator<T> {
	protected:
		CIteratorWriteData(const util::CAutoPointer<ICorrespondent<T> >& correspondent)
			: CMessageIterator<T>(correspondent) {}

		~CIteratorWriteData() override {}

		virtual bool ReadOnly() const override {
			return false;
		}

	private:
		virtual void Process(util::CWeakPointer<const T> object) final {
			throw std::runtime_error("Can not call the Process(util::CWeakPointer<const T> object) function.");
		}
	};

	template<typename T>
	class CIteratorReadData : public CMessageIterator<T> {
	protected:
		CIteratorReadData(const util::CAutoPointer<ICorrespondent<T> >& correspondent)
			: CMessageIterator<T>(correspondent) {}

		~CIteratorReadData() override {}

		virtual bool ReadOnly() const override {
			return true;
		}

	private:
		virtual void Process(util::CWeakPointer<T> object) final {
			throw std::runtime_error("Can not call the Process(util::CWeakPointer<T> object) function.");
		}
	};
}

#endif /* MESSAGE_DATA_H */
