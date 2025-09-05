/* 
 * File:   WrapObject.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef WRAP_OBJECT_H
#define	WRAP_OBJECT_H

#include "MessageData.h"
#include "UniquePointer.h"
#include "AutoPointer.h"
#include "WrapControl.h"
#include "CircleQueue.h"
#include "PoolBase.h"
#include "OperationAsync.h"
#include "OperationSync.h"
#include "OperationIterate.h"

namespace thd {

#define SEND_TIMEOUT_INFINITY -1

	template<typename T> class COperationBase; 

	template<typename T, int AnswerQueueKeepSize = 8, int RoutineQueueKeepSize = 8>
	class CWrapObject : public CWrapControl, public ICorrespondent<T>
	{
	public:
		typedef typename util::CAutoPointer<COperationBase<T> > QueueItemType;

		CWrapObject() : m_answer(), m_routine() {}

		CWrapObject(util::CAutoPointer<T> &object) : m_object(object), m_answer(), m_routine() {}

		~CWrapObject() {
			CWrapControl::Dispose();
			m_answer.Clear();
			m_routine.Clear();
		}

		/**
		* Send messages to other objects and wait for results.
		* Template function matching different subclass message types.
		* 
		* @param subMsg      The message
		* @param emergency   If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		* @param msTime Message waiting timeout, unit: milliseconds.
		*/
		template<typename subMsgT>
		eSendResult Send(util::CUniquePointer<subMsgT>& subMsg,
			bool emergency = true, int msTime = SEND_TIMEOUT_INFINITY) const
		{
			if (subMsg.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			eSendResult ret = InnerSend(msg, emergency, msTime);
			subMsg = msg;
			return ret;
		}
		/**
		* Send messages to other objects and wait for results
		*
		* @param subMsg      The message
		* @param emergency   If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		* @param msTime Message waiting timeout, unit: milliseconds.
		*/
		eSendResult Send(util::CUniquePointer<CMessageBase<T> >& msg,
			bool emergency = true, int msTime = SEND_TIMEOUT_INFINITY) const
		{
			if (msg.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			return InnerSend(msg, emergency, msTime);
		}

		/**
		* Post messages to other objects without waiting for any results.
		*
		* @param subMsg      The message
		* @param emergency   If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		*/
		bool Post(util::CUniquePointer<CMessageBase<T> >& msg, bool emergency = false) const
		{
			if (GetStatus() != CONTROL_RUNNING) {
				return false;
			}

			if (msg.IsInvalid()) {
				return false;
			}
			util::CAutoPointer<COperationBase<T> > asyncOperation(new util::WrapPoolIs<COperationAsync<T> >(msg));
			if (emergency) {
				QueueItemType* p = m_answer.WriteLock();
				*p = asyncOperation;
				m_answer.WriteUnlock();
				InnerTrigger();
			} else {
				QueueItemType* p = m_routine.WriteLock();
				*p = asyncOperation;
				m_routine.WriteUnlock();
				InnerTrigger();
			}
			return true;
		}

		/**
		* Post messages to other objects without waiting for any results.
		* Template function matching different subclass message types.
		*
		* @param subMsg       The message
		* @param emergency    If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		*/
		template<typename mT>
		bool Post(util::CUniquePointer<mT>& subMsg, bool emergency = false) const {
			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			return Post(msg, emergency);
		}

		/**
		* When the WrapObject class object is in a list and you need to iterate over each object
		*    in the list to execute the same message, call the Iterate function.
		* The message does not wait for the result, similar to the Post operation on each object in the list.
		*    If you need to notify the result after all objects have completed the message,
		*	 you can overload the OnFinish method in the subclass of the iterator object 
		*	 to get the status of whether all objects have completed the message.
		* 
		* @param iterator     The iterator
		* @param msg          The message to be post
		* @param emergency    If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		* 
		*/
		bool Iterate(util::CAutoPointer<CIteratorBase<T> >& iterator,
			util::CUniquePointer<CMessageBase<T> >& msg, bool emergency = false) const
		{
			if (GetStatus() != CONTROL_RUNNING) {
				return false;
			}

			if (iterator.IsInvalid()) {
				return false;
			}

			if (msg.IsInvalid()) {
				return false;
			}

			util::CAutoPointer<COperationIterate<T> > iterateOperation(new util::WrapPoolIs<COperationIterate<T> >(iterator, msg));
			if (emergency) {
				QueueItemType* p = m_answer.WriteLock();
				*p = iterateOperation;
				m_answer.WriteUnlock();
				InnerTrigger();
			} else {
				QueueItemType* p = m_routine.WriteLock();
				*p = iterateOperation;
				m_routine.WriteUnlock();
				InnerTrigger();
			}
			return true;
		}

		/**
		* When the WrapObject class object is in a list and you need to iterate over each object
		*    in the list to execute the same message, call the Iterate function.
		* The message does not wait for the result, similar to the Post operation on each object in the list.
		*    If you need to notify the result after all objects have completed the message,
		*	 you can overload the OnFinish method in the subclass of the iterator object
		*	 to get the status of whether all objects have completed the message.
		* 
		* Template function matching iterator subclasses.
		*
		* @param iterator     The iterator
		* @param msg          The message to be post
		* @param emergency    If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		*
		*/
		template<typename iT>
		bool Iterate(util::CAutoPointer<iT>& subIterator,
			util::CUniquePointer<CMessageBase<T> >& msg, bool emergency = false) const
		{
			util::CAutoPointer<CIteratorBase<T> > iterator(subIterator);
			return Iterate(iterator, msg, emergency);
		}

		/**
		* When the WrapObject class object is in a list and you need to iterate over each object
		*    in the list to execute the same message, call the Iterate function.
		* The message does not wait for the result, similar to the Post operation on each object in the list.
		*    If you need to notify the result after all objects have completed the message,
		*	 you can overload the OnFinish method in the subclass of the iterator object
		*	 to get the status of whether all objects have completed the message.
		*
		* Template function matching message subclasses.
		*
		* @param iterator     The iterator
		* @param msg          The message to be post
		* @param emergency    If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		*
		*/
		template<typename mT>
		bool Iterate(util::CAutoPointer<CIteratorBase<T> >& iterator,
			util::CUniquePointer<mT>& subMsg, bool emergency = false) const
		{
			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			return Iterate(iterator, msg, emergency);
		}

		/**
		* When the WrapObject class object is in a list and you need to iterate over each object
		*    in the list to execute the same message, call the Iterate function.
		* The message does not wait for the result, similar to the Post operation on each object in the list.
		*    If you need to notify the result after all objects have completed the message,
		*	 you can overload the OnFinish method in the subclass of the iterator object
		*	 to get the status of whether all objects have completed the message.
		*
		* Template functions matching iterator subclasses and message subclasses.
		*
		* @param iterator     The iterator
		* @param msg          The message to be post
		* @param emergency    If there is a blocked Send function call in the message,
		*    please set emergency to false, otherwise set it to true.
		*
		*/
		template<typename iT, typename mT>
		bool Iterate(util::CAutoPointer<iT>& subIterator,
			util::CUniquePointer<mT>& subMsg, bool emergency = false) const
		{
			util::CAutoPointer<CIteratorBase<T> > iterator(subIterator);
			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			return Iterate(iterator, msg, emergency);
		}

		/**
		* If the WrapObject class objects are in the list and you need to traverse each object
		*    and perform a pause operation, you can call the Pause function.
		* If you need to know whether all objects in the list have completed the pause state,
		*    please overload the OnFinish method in the iterator subclass extension to obtain it.
		* 
		* Template function matching message subclasses.
		* 
		* @param iterator   The iterator
		* @param subMsg     The message
		*/
		template<typename mT>
		void Pause(util::CAutoPointer<CIteratorBase<T> >& iterator,
			       util::CUniquePointer<mT>& subMsg) const
		{
			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			Pause(iterator, msg);
		}

		/**
		* If the WrapObject class objects are in the list and you need to traverse each object
		*    and perform a pause operation, you can call the Pause function.
		* If you need to know whether all objects in the list have completed the pause state,
		*    please overload the OnFinish method in the iterator subclass extension to obtain it.
		*
		* Template function matching iterator subclasses.
		* 
		* @param iterator   The iterator
		* @param subMsg     The message
		*/
		template<typename iT>
		bool Pause(util::CAutoPointer<iT>& subIterator,
			util::CUniquePointer<CMessageBase<T> >& msg) const
		{
			util::CAutoPointer<CIteratorBase<T> > iterator(subIterator);
			return Pause(iterator, msg);
		}

		/**
		* If the WrapObject class objects are in the list and you need to traverse each object
		*    and perform a pause operation, you can call the Pause function.
		* If you need to know whether all objects in the list have completed the pause state,
		*    please overload the OnFinish method in the iterator subclass extension to obtain it.
		*
        * Template functions matching iterator subclasses and message subclasses.
		* 
		* @param iterator   The iterator
		* @param subMsg     The message
		*/
		template<typename iT, typename mT>
		bool Pause(util::CAutoPointer<iT>& subIterator,
			       util::CUniquePointer<mT>& subMsg) const
		{
			util::CAutoPointer<CIteratorBase<T> > iterator(subIterator);
			util::CUniquePointer<CMessageBase<T> > msg(subMsg);
			return Pause(iterator, msg);
		}

		/**
		* If the WrapObject class objects are in the list and you need to traverse each object
		*    and perform a pause operation, you can call the Pause function.
		* If you need to know whether all objects in the list have completed the pause state,
		*    please overload the OnFinish method in the iterator subclass extension to obtain it.
		*
		* @param iterator   The iterator
		* @param subMsg     The message
		*/
		void Pause(util::CAutoPointer<CIteratorBase<T> >& iterator,
		           util::CUniquePointer<CMessageBase<T> >& msg) const
		{
			m_pause.SetRawPointer(new util::WrapPoolIs<COperationIterate<T> >(iterator, msg));
		}

	private:
		eSendResult InnerSend(util::CUniquePointer<CMessageBase<T> >& msg,
			bool emergency = true,
			int msTime = SEND_TIMEOUT_INFINITY) const
		{
			if (s_tlsProcessing) {
				throw std::runtime_error("Do not use this send function,"
					" please use the send of CMessageSender instead.");
				return eSendResult::SEND_FAIL;
			}

			if (GetStatus() != CONTROL_RUNNING) {
				return eSendResult::SEND_EXIT;
			}
			
			if (msg.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			bool readOnly = msg->ReadOnly();
			if (TryDirectAccess(readOnly)) {
				util::CAutoPointer<COperationSync<T> > syncOperation(new util::WrapPoolIs<COperationSync<T> >(msg));
				QueueItemType baseOperation(syncOperation);
				ProcessMessage(baseOperation);
				DirectAccessDone();
				if (!readOnly) {
					CheckSizeAndTrigger();
				}
				msg = syncOperation->GetData();
				return eSendResult::SEND_SUCCESS;
			}

			bool notSender = IsNotSender(msg);
			util::CAutoPointer<COperationSync<T> > syncOperation(new util::WrapPoolIs<COperationSync<T> >(msg));
			if (emergency && notSender) {
				QueueItemType* p = m_answer.WriteLock();
				*p = syncOperation;
				m_answer.WriteUnlock();
				InnerTrigger();
			} else {
				QueueItemType* p = m_routine.WriteLock();
				*p = syncOperation;
				m_routine.WriteUnlock();
				InnerTrigger();
			}

			if (msTime == SEND_TIMEOUT_INFINITY) {
				eWaitingResult ret = syncOperation->Waiting();
				if (WAITING_EXIT == ret) {
					msg = syncOperation->GetData();
					return eSendResult::SEND_EXIT;
				}
			} else {
				eWaitingResult ret = syncOperation->Waiting(msTime);
				if (WAITING_EXIT == ret) {
					msg = syncOperation->GetData();
					return eSendResult::SEND_EXIT;
				} else if (WAITING_TIMEOUT == ret) {
					msg = syncOperation->GetData();
					return eSendResult::SEND_TIMEOUT;
				}
			}
			msg = syncOperation->GetData();
			return eSendResult::SEND_SUCCESS;
		}

		eSendResult InnerSend(util::CAutoPointer<CWrapObject>& sender,
			util::CUniquePointer<CMessageBase<T> >& msg,
			bool emergency = true) const
		{
			if (GetStatus() != CONTROL_RUNNING) {
				return eSendResult::SEND_EXIT;
			}
			
			if (sender.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			if (msg.IsInvalid()) {
				return eSendResult::SEND_FAIL;
			}

			if (sender == this) {
				return eSendResult::SEND_FAIL;
			}

			bool readOnly = msg->ReadOnly();
			if (TryDirectAccess(readOnly)) {
				util::CAutoPointer<COperationSync<T> > syncOperation(new util::WrapPoolIs<COperationSync<T> >(msg));
				QueueItemType baseOperation(syncOperation);
				ProcessMessage(baseOperation);
				DirectAccessDone();
				if (!readOnly) {
					CheckSizeAndTrigger();
				}
				msg = syncOperation->GetData();
				return eSendResult::SEND_SUCCESS;
			}
			
			bool notSender = IsNotSender(msg);
			const CWrapControl* receiver = NULL;

			util::CAutoPointer<COperationSync<T> > syncOperation(new util::WrapPoolIs<COperationSync<T> >(msg));
			if (emergency && notSender) {
				receiver = dynamic_cast<const CWrapControl*>(this);
				if (!readOnly) {
					QueueItemType* p = m_answer.WriteLock();
					*p = syncOperation;
					m_answer.WriteUnlock();
					InnerTrigger();
				}
			} else {
				if (emergency) {
					receiver = dynamic_cast<const CWrapControl*>(this);
					if (!readOnly) {
						QueueItemType* p = m_routine.WriteLock();
						*p = syncOperation;
						m_routine.WriteUnlock();
						InnerTrigger();
					}
				} else {
					QueueItemType* p = m_routine.WriteLock();
					*p = syncOperation;
					m_routine.WriteUnlock();
					InnerTrigger();
				}
			}

			eWaitingResult ret = sender->Waitting(syncOperation->GetFlag(), receiver, readOnly);
			if (WAITING_DIRECT_ACCESS == ret) {
				syncOperation->SetProcessed(true);
				QueueItemType baseOperation(syncOperation);
				ProcessMessage(baseOperation);
				DirectAccessDone();
				if (!readOnly) {
					CheckSizeAndTrigger();
				}
			} else if (WAITING_EXIT == ret) {
				msg = syncOperation->GetData();
				return eSendResult::SEND_EXIT;
			} else if (WAITING_SUCCESS != ret) {
				msg = syncOperation->GetData();
				return eSendResult::SEND_FAIL;
			}
			msg = syncOperation->GetData();
			return eSendResult::SEND_SUCCESS;
		}

		virtual eSendResult InnerSend(util::CAutoPointer<ICorrespondent<T> >& sender,
			util::CUniquePointer<CMessageBase<T> >& msg,
			bool emergency = true) const override
		{
			util::CAutoPointer<CWrapObject> wrapObject(sender);
			assert(wrapObject != static_cast<CWrapObject*>(NULL));
			return InnerSend(wrapObject, msg, emergency);
		}

		virtual void TriggerOperation(util::CAutoPointer<COperationBase<T> >& operation) override
		{
			QueueItemType* p = m_routine.WriteLock();
			*p = operation;
			m_routine.WriteUnlock();
			Trigger();
		}

		void ReadWriteProcess(QueueItemType& item, CCircleQueue<QueueItemType>& queue) {
			if (item->ReadOnly()) {
				s_tlsProcessing = true;
				if (item->Preprocess()) {
					item->Process(m_object);
				}
				s_tlsProcessing = false;
				if (item->OperationType() == eOperationType::OPERATION_REPEAT) {
					QueueItemType* p = queue.WriteLock();
					*p = item;
					queue.WriteUnlock();
				}
			} else {
				WaitUpgradeRead();
				s_tlsProcessing = true;
				if (item->Preprocess()) {
					item->Process(m_object);
				}
				s_tlsProcessing = false;
				if (item->OperationType() == eOperationType::OPERATION_REPEAT) {
					WaitDegradeWrite();
					QueueItemType* p = queue.WriteLock();
					*p = item;
					queue.WriteUnlock();
				} else {
					WaitDegradeWrite();
				}		
			}
		}

		void OnlyWaitReadProcess(QueueItemType& item, CCircleQueue<QueueItemType>& queue) {
			if (item->ReadOnly()) {
				s_tlsProcessing = true;
				if (item->Preprocess()) {
					item->Process(m_object);
				}
				s_tlsProcessing = false;
				if (item->OperationType() == eOperationType::OPERATION_REPEAT) {
					QueueItemType* p = queue.WriteLock();
					*p = item;
					queue.WriteUnlock();
				}
			} else {
				bool bWrite = OnlyReadWaitUpgrade();
				s_tlsProcessing = true;
				if (item->Preprocess()) {
					item->Process(m_object);
				}
				s_tlsProcessing = false;
				if (item->OperationType() == eOperationType::OPERATION_REPEAT) {
					if (bWrite) {
						WaitDegradeWrite();
					}
					QueueItemType* p = queue.WriteLock();
					*p = item;
					queue.WriteUnlock();
				} else {
					if (bWrite) {
						WaitDegradeWrite();
					}
				}		
			}
		}

		void ReadWriteProcess(QueueItemType& item) {
			if (item->ReadOnly()) {
				s_tlsProcessing = true;
				do {
					if (item->Preprocess()) {
						item->Process(m_object);
					}
				} while(item->OperationType() == eOperationType::OPERATION_REPEAT);
				s_tlsProcessing = false;
			} else {
				do {
					WaitUpgradeRead();
					s_tlsProcessing = true;
					if (item->Preprocess()) {
						item->Process(m_object);
					}
					s_tlsProcessing = false;
					if (item->OperationType() == eOperationType::OPERATION_REPEAT) {
						WaitDegradeWrite();
						continue;
					} else {
						WaitDegradeWrite();
					}
				} while (false);
			}
		}

		void ProcessMessage(QueueItemType& item) const {
			s_tlsProcessing = true;
			do {
				if (item->Preprocess()) {
					item->Process(m_object);
				}
			} while(item->OperationType() == eOperationType::OPERATION_REPEAT);
			s_tlsProcessing = false;
		}

		virtual void OnEmergency() override {
			QueueItemType* pNode = NULL;
			do {
				pNode = m_answer.ReadLock();
				if (pNode == NULL) {
					break;
				}
				QueueItemType item(*pNode);
				m_answer.ReadUnlock();
				if (item->IsProcessed()) {
					continue;
				}
				OnlyWaitReadProcess(item, m_answer);
			} while (true);
		}

		virtual void OnRouite() override {
			QueueItemType* pNode = NULL;
			do {
				pNode = m_answer.ReadLock();
				if (pNode == NULL) {
					break;
				}
				QueueItemType item(*pNode);
				m_answer.ReadUnlock();
				if (item->IsProcessed()) {
					continue;
				}
				ReadWriteProcess(item, m_answer);
			} while (true);

			do {
				pNode = m_routine.ReadLock();
				if (pNode == NULL) {
					break;
				}
				QueueItemType item(*pNode);
				m_routine.ReadUnlock();
				if (item->IsProcessed()) {
					continue;
				}
				ReadWriteProcess(item, m_routine);
			} while (true);
		}
		
		virtual void OnPause() override
		{
			if (m_pause.IsInvalid()) {
				return;
			}
			ReadWriteProcess(m_pause);
		}

		virtual int RemainingSize() const override
		{
			return m_routine.Size() + m_answer.Size();
		}

		inline static bool IsNotSender(const util::CUniquePointer<CMessageBase<T> >& msg)
		{
			return dynamic_cast<CMessageSender<T> *>(msg.Get()) == NULL;
		}

    protected:
		util::CAutoPointer<T> m_object;

	private:
		mutable CCircleQueue<QueueItemType, AnswerQueueKeepSize> m_answer;
		mutable CCircleQueue<QueueItemType, RoutineQueueKeepSize> m_routine;
		QueueItemType m_pause;

#if defined( _MSC_VER ) || defined( __BORLANDC__ )
		__declspec(thread) static bool s_tlsProcessing;
#else
		static __thread bool s_tlsProcessing;
#endif
	};

#if defined( _MSC_VER ) || defined( __BORLANDC__ )
	template<typename T, int AnswerQueueKeepSize, int RoutineQueueKeepSize>
	bool CWrapObject<T, AnswerQueueKeepSize, RoutineQueueKeepSize>::s_tlsProcessing = false;
#else
	template<typename T, int AnswerQueueKeepSize, int RoutineQueueKeepSize>
	__thread bool CWrapObject<T, AnswerQueueKeepSize, RoutineQueueKeepSize>::s_tlsProcessing = false;
#endif
}

#endif /* WRAP_OBJECT_H */
