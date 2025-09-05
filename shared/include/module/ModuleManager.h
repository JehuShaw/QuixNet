/*
 * File:   ModuleManager.h
 * Author: Jehu Shaw
 *
 * Created on 2011年3月14日, 下午3:27
 */

#ifndef MODULEMANAGER_H
#define	MODULEMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <stdint.h>
#include <stdexcept>
#include "WeakPointer.h"
#include "INotificationBody.h"
#include "SpinRWLock.h"
#include "Singleton.h"

namespace mdl {

	 class SHARED_DLL_DECL IResponse {
	public:
		virtual ~IResponse(){}

		virtual void SetResult(int result) = 0;

		virtual int GetResult() const = 0;

		virtual void SetBody(const util::CWeakPointer<IBody> body) = 0;

		virtual util::CWeakPointer<IBody> GetBody() const = 0;

		virtual void ResetBody() = 0;
	};

	class SHARED_DLL_DECL INotification {
	public:
		virtual ~INotification(){}

		virtual int GetName() const = 0;

		virtual void SetBody(const util::CWeakPointer<IBody>& body) = 0;

		virtual const util::CWeakPointer<IBody>& GetBody() const = 0;

		virtual void SetType(int notificationType) = 0;

		virtual int GetType() const = 0;

		virtual void ResetBody() = 0;
	};

	/**
	 *  An INotificationHandler base class.
	 */
	class SHARED_DLL_DECL INotificationHandler
	{
	public:
		virtual ~INotificationHandler(){}

		virtual void HandleNotification(const util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response) = 0;
	};

	class SHARED_DLL_DECL INotifier {
	public:
		virtual ~INotifier(){}

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int notificationType, bool reverse) = 0;

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply) = 0;

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request) = 0;

		virtual int SendNotification(int notificationName, int notificationType) = 0;

		virtual int SendNotification(int notificationName) = 0;

	};

	class SHARED_DLL_DECL IObserverRestricted
	{
	public:
		virtual ~IObserverRestricted(){}

		virtual void NotifyObserver(const util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response) = 0;

		virtual bool CompareNotifyContext(intptr_t memoryAddress) const = 0;
	};

	class SHARED_DLL_DECL IInterests {
	public:
		virtual ~IInterests() {}

		virtual int GetNotificationName() const = 0;

		virtual const util::CAutoPointer<IObserverRestricted>& GetObserver() const = 0;
	};

	class SHARED_DLL_DECL IModule : public virtual INotifier, public INotificationHandler {
	public:

		typedef std::vector<util::CAutoPointer<IInterests> > InterestList;

		virtual ~IModule(){}

		virtual const std::string& GetModuleName() const = 0;

		virtual std::vector<int> ListNotificationInterests() = 0;

		virtual void OnRegister() = 0;

		virtual void OnRemove() = 0;

		virtual InterestList ListProtocolInterests() = 0;

		// IF (reply.IsInvalid() || reply->getResult() == FALSE) take all protocol.
		virtual util::CAutoPointer<IObserverRestricted> FullProtocolInterests() = 0;

		virtual int GetType() const = 0;
	};

	class SHARED_DLL_DECL ISubject
	{
	public:
		virtual ~ISubject(){}

		virtual void RegisterObserver(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer, int type) = 0;

		virtual void RemoveObserver(int notificationName, intptr_t contextAddress, int type) = 0;

		virtual void NotifyObservers(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse) = 0;

		virtual void RegisterModule(util::CAutoPointer<IModule>& module) = 0;

		virtual util::CAutoPointer<IModule> RetrieveModule(const std::string& moduleName) const = 0;

		virtual util::CAutoPointer<IModule> RemoveModule(const std::string& moduleName) = 0;

		virtual bool HasModule(const std::string& moduleName) const = 0;

		virtual void RegisterProtocol(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer, int type) = 0;

		virtual void RegisterProtocol(const util::CAutoPointer<IObserverRestricted>& observer, int type) = 0;

		virtual void NotifyProtocol(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse) = 0;

		virtual void RemoveProtocol(int notificationName, intptr_t contextAddress, int type) = 0;

		virtual void RemoveProtocol(intptr_t contextAddress, int type) = 0;

		virtual void IterateModule(std::vector<util::CAutoPointer<IModule> >& outModules) const = 0;
	};

	template<class T>
	class IObserverTemplated : public IObserverRestricted
	{
	public:

		typedef void(T::*NotifyMethod)(const util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response);

		virtual ~IObserverTemplated(){}

		virtual void SetNotifyMethod(NotifyMethod method) = 0;

		virtual void SetNotifyContext(T* notifyContext) = 0;
	};

	class SHARED_DLL_DECL IFacade : public virtual INotifier
	{
	public:
		virtual ~IFacade(){}

		virtual void RegisterModule(util::CAutoPointer<IModule> module) = 0;

		virtual util::CAutoPointer<IModule> RetrieveModule(std::string moduleName) const = 0;

		virtual util::CAutoPointer<IModule> RemoveModule(std::string moduleName) = 0;

		virtual bool HasModule(std::string moduleName) const = 0;

		virtual void NotifyObservers(util::CWeakPointer<INotification> request, util::CWeakPointer<IResponse> response, bool reverse) = 0;

		virtual void NotifyProtocol(util::CWeakPointer<INotification> request, util::CWeakPointer<IResponse> response, bool reverse) = 0;

	};

	//--------------------------------------
	//  CResponse
	//--------------------------------------

	class SHARED_DLL_DECL CResponse : public IResponse {
	public:
		CResponse() : m_result(FALSE) {}
		CResponse(const util::CWeakPointer<IBody> body) : m_result(FALSE), m_body(body) {}
		CResponse(int result) : m_result(result) {}
		CResponse(const util::CWeakPointer<IBody> body, int result) : m_result(result), m_body(body) {}

		virtual ~CResponse(){}

		virtual void SetResult(int result)
		{
			m_result = result;
		}

		virtual int GetResult() const
		{
			return m_result;
		}

		virtual void SetBody(const util::CWeakPointer<IBody> body)
		{
			m_body = body;
		}

		virtual util::CWeakPointer<IBody> GetBody() const
		{
			return m_body;
		}

		virtual void ResetBody()
		{
			if (m_body.IsInvalid()) {
				return;
			}
			m_body->ResetBody();
		}

	private:
		int m_result;
		util::CWeakPointer<IBody> m_body;
	};

	//--------------------------------------
	//  CNotification
	//--------------------------------------
	class SHARED_DLL_DECL CNotification : public INotification
	{
	public:

		CNotification(int notificationName, const util::CWeakPointer<IBody>& body, int notificationType)
			: m_name(notificationName), m_body(body), m_type(notificationType) {}

		CNotification(int notificationName, const util::CWeakPointer<IBody>& body)
			: m_name(notificationName), m_body(body) {}

		CNotification(int notificationName, int notificationType)
			: m_name(notificationName), m_type(notificationType) {}

		CNotification(int notificationName) : m_name(notificationName) {}

		int GetName() const
		{
			return m_name;
		}

		void SetBody(const util::CWeakPointer<IBody>& body)
		{
			m_body = body;
		}

		const util::CWeakPointer<IBody>& GetBody() const
		{
			return m_body;
		}

		void SetType(int notificationType)
		{
			m_type = notificationType;
		}

		int GetType() const
		{
			return m_type;
		}

		void ResetBody()
		{
			if (m_body.IsInvalid()) {
				return;
			}
			m_body->ResetBody();
		}

	private:
		int m_name;
		int m_type;
		util::CWeakPointer<IBody> m_body;
	};

	//--------------------------------------
	//  CNotifier
	//--------------------------------------
	class SHARED_DLL_DECL CNotifier : public virtual INotifier
	{
	public:

		virtual int SendNotification (int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int notificationType, bool reverse)
		{
			return GetFacade()->SendNotification(notificationName, request, reply,
				notificationType, reverse);
		}

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply)
		{
			return GetFacade()->SendNotification(notificationName, request, reply);
		}

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request)
		{
			return GetFacade()->SendNotification(notificationName, request);
		}

		virtual int SendNotification(int notificationName, int notificationType)
		{
			return GetFacade()->SendNotification(notificationName, notificationType);
		}

		virtual int SendNotification(int notificationName)
		{
			return GetFacade()->SendNotification(notificationName);
		}
		
	protected:
		util::CAutoPointer<IFacade> GetFacade();
	};

	/**
	 * A base <code>IObserver</code> implementation.
	 */
	template<class ST, class BT = ST>
	class CObserver : public IObserverTemplated<ST>
	{
	private:
		typedef ST* NotifyContext;

	public:
		CObserver(typename IObserverTemplated<ST>::NotifyMethod method, NotifyContext context)
			: m_notifyMethod(method), m_notifyContext(context) {}

		void SetNotifyMethod(typename IObserverTemplated<ST>::NotifyMethod method)
		{
			m_notifyMethod = method;
		}

		void SetNotifyContext(NotifyContext context)
		{
			m_notifyContext = context;
		}

		typename IObserverTemplated<ST>::NotifyMethod GetNotifyMethod() const
		{
			return m_notifyMethod;
		}

		NotifyContext GetNotifyContext() const
		{
			return m_notifyContext;
		}

		void NotifyObserver(const util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response)
		{
			(m_notifyContext->*m_notifyMethod)(request, response);
		}

		bool CompareNotifyContext(intptr_t compareContextAddress) const
		{
			return compareContextAddress == (intptr_t)(BT*)this->GetNotifyContext();
		}

	private:
		typename IObserverTemplated<ST>::NotifyMethod m_notifyMethod;
		NotifyContext m_notifyContext;
	};

	class SHARED_DLL_DECL CSubject : public ISubject
	{
	public:

		void RegisterObserver(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer, int type);

		void NotifyObservers(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse);

		void RemoveObserver(int notificationName, intptr_t contextAddress, int type);

		void RegisterModule(util::CAutoPointer<IModule>& module);

		util::CAutoPointer<IModule> RetrieveModule(const std::string& moduleName) const;

		util::CAutoPointer<IModule> RemoveModule(const std::string& moduleName);

		bool HasModule(const std::string& moduleName) const;

		void RegisterProtocol(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer, int type);

		void RegisterProtocol(const util::CAutoPointer<IObserverRestricted>& observer, int type);

		void NotifyProtocol(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse);

		void RemoveProtocol(int notificationName, intptr_t contextAddress, int type);

		void RemoveProtocol(intptr_t contextAddress, int type);

		void IterateModule(std::vector<util::CAutoPointer<IModule> >& outModules) const;

	protected:
		typedef std::map<std::string, util::CAutoPointer<IModule> > MODULE_MAP_T;
		typedef std::vector<util::CAutoPointer<IObserverRestricted> > OBSERVER_VECTOR_T;
		typedef std::map<int, OBSERVER_VECTOR_T > OBSERVER_MAP_T;


		MODULE_MAP_T m_moduleMap;
		thd::CSpinRWLock m_moduleMapLock;

		OBSERVER_MAP_T m_observerMap;
		thd::CSpinRWLock m_observerMapLock;

		OBSERVER_MAP_T m_protoMap;
		thd::CSpinRWLock m_protoMapLock;

		OBSERVER_VECTOR_T m_protoObserver;
		thd::CSpinRWLock m_protoLock;

	protected:
		inline bool AddModule(const std::string& moduleName, const util::CAutoPointer<IModule>& module) {
			thd::CScopedWriteLock wtlock(m_moduleMapLock);
			std::pair<MODULE_MAP_T::iterator, bool> pairIB(m_moduleMap.insert(
				MODULE_MAP_T::value_type(moduleName, module)));
			return pairIB.second;
		}

        inline util::CAutoPointer<IModule> FindModule(const std::string& moduleName) const {
            thd::CScopedReadLock rdlock(m_moduleMapLock);

            MODULE_MAP_T::const_iterator it(m_moduleMap.find(moduleName));
            if(m_moduleMap.end() == it) {
                return util::CAutoPointer<IModule>();
            }
            return it->second;
        }

        inline void EraseModule(const std::string& moduleName) {
            thd::CScopedWriteLock wtlock(m_moduleMapLock);
            m_moduleMap.erase(moduleName);
        }
	};

	class SHARED_DLL_DECL CSubjectType : public ISubject
	{
	public:

		void RegisterObserver(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer, int type);

		void NotifyObservers(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse);

		void RemoveObserver(int notificationName, intptr_t contextAddress, int type);

		void RegisterModule(util::CAutoPointer<IModule>& module);

		util::CAutoPointer<IModule> RetrieveModule(const std::string& moduleName) const;

		util::CAutoPointer<IModule> RemoveModule(const std::string& moduleName);

		bool HasModule(const std::string& moduleName) const;

		void RegisterProtocol(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer, int type);

		void RegisterProtocol(const util::CAutoPointer<IObserverRestricted>& observer, int type);

		void NotifyProtocol(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse);

		void RemoveProtocol(int notificationName, intptr_t contextAddress, int type);

		void RemoveProtocol(intptr_t contextAddress, int type);

		void IterateModule(std::vector<util::CAutoPointer<IModule> >& outModules) const;

	protected:
		typedef std::map<std::string, util::CAutoPointer<IModule> > MODULE_MAP_T;
		typedef std::vector<util::CAutoPointer<IObserverRestricted> > OBSERVER_VECTOR_T;
		typedef std::map<int, OBSERVER_VECTOR_T > OBSERVER_MAP_T;
		typedef std::map<int, OBSERVER_MAP_T> TYPE_OBSERVER_MAP_T;


		MODULE_MAP_T m_moduleMap;
		thd::CSpinRWLock m_moduleMapLock;

		TYPE_OBSERVER_MAP_T m_observerMap;
		thd::CSpinRWLock m_observerMapLock;

		TYPE_OBSERVER_MAP_T m_protoMap;
		thd::CSpinRWLock m_protoMapLock;

		OBSERVER_MAP_T m_protoObserver;
		thd::CSpinRWLock m_protoLock;

	protected:
		inline bool AddModule(const std::string& moduleName, const util::CAutoPointer<IModule>& module) {
			thd::CScopedWriteLock wtlock(m_moduleMapLock);
			std::pair<MODULE_MAP_T::iterator, bool> pairIB(m_moduleMap.insert(
				MODULE_MAP_T::value_type(moduleName, module)));
			return pairIB.second;
		}

		inline util::CAutoPointer<IModule> FindModule(const std::string& moduleName) const {
			thd::CScopedReadLock rdlock(m_moduleMapLock);

			MODULE_MAP_T::const_iterator it(m_moduleMap.find(moduleName));
			if(m_moduleMap.end() == it) {
				return util::CAutoPointer<IModule>();
			}
			return it->second;
		}

		inline void EraseModule(const std::string& moduleName) {
			thd::CScopedWriteLock wtlock(m_moduleMapLock);
			m_moduleMap.erase(moduleName);
		}
	};

	class SHARED_DLL_DECL CInterests : public IInterests {
	public:

		explicit CInterests(int nNotificationName, util::CAutoPointer<IObserverRestricted>& observer)
			: m_nNotificationName(nNotificationName), m_observer(observer) {}

		virtual int GetNotificationName() const
		{
			return m_nNotificationName;
		}

		virtual const util::CAutoPointer<IObserverRestricted>& GetObserver() const
		{
			return m_observer;
		}

	private:
		int m_nNotificationName;
		util::CAutoPointer<IObserverRestricted> m_observer;
	};

	//--------------------------------------
	//  CModule
	//--------------------------------------
	class SHARED_DLL_DECL CModule : public CNotifier, public virtual IModule
	{
	public:
		/**
		 * Constructor.
		 */
		CModule(const char* moduleName) : m_moduleName(moduleName) {}

		CModule(const std::string& moduleName) : m_moduleName(moduleName) {}

		virtual const std::string& GetModuleName() const
        {
            return m_moduleName;
        }

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int notificationType, bool reverse) {

			return this->CNotifier::SendNotification(notificationName, request, reply,
				notificationType, reverse);
		}

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply) {

			return this->CNotifier::SendNotification(notificationName, request, reply);
		}

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request) {

			return this->CNotifier::SendNotification(notificationName, request);
		}

		virtual int SendNotification(int notificationName, int notificationType) {

			return this->CNotifier::SendNotification(notificationName, notificationType);
		}

		virtual int SendNotification(int notificationName) {

			return this->CNotifier::SendNotification(notificationName);
		}

		virtual util::CAutoPointer<IObserverRestricted> FullProtocolInterests() {
			return util::CAutoPointer<IObserverRestricted>();
		}

		virtual int GetType() const {
			return 0;
		}

		template<class T>
		util::CAutoPointer<IInterests> BindMethod(int notificationName
			, typename IObserverTemplated<T>::NotifyMethod method) {
				T* pSubClass = dynamic_cast<T*>(this);
				if(NULL == pSubClass) {
					throw std::runtime_error("The type must be subclass of CModule !");
				}
				util::CAutoPointer<IObserverRestricted> observer(new CObserver<T, IModule>(method, pSubClass));
				return util::CAutoPointer<IInterests>(new CInterests(notificationName, observer));
		}

		template<class T>
		util::CAutoPointer<IObserverRestricted> BindMethod(typename IObserverTemplated<T>::NotifyMethod method) {
			T* pSubClass = dynamic_cast<T*>(this);
			if(NULL == pSubClass) {
				throw std::runtime_error("The type must be subclass of CModule !");
			}
			return util::CAutoPointer<IObserverRestricted>(new CObserver<T, IModule>(method, pSubClass));
		}

	protected:
		// the module name
		std::string m_moduleName;

	};

	class SHARED_DLL_DECL CFacade
		: public IFacade
		, public util::Singleton<CFacade>
	{
	public:
		CFacade() : m_subject(new CSubject()) {}

		~CFacade(){ m_subject.SetRawPointer(NULL); }

		void RegisterModule(util::CAutoPointer<IModule> module)
		{
			if (m_subject.IsInvalid()) {
				return;
			}
			m_subject->RegisterModule(module);
		}

		util::CAutoPointer<IModule> RetrieveModule(std::string moduleName) const
		{
			if (m_subject.IsInvalid()) {
				return util::CAutoPointer<IModule>();
			}
			return m_subject->RetrieveModule(moduleName);
		}

		util::CAutoPointer<IModule> RemoveModule(std::string moduleName)
		{
			if(m_subject.IsInvalid()) {
				return util::CAutoPointer<IModule>();
			}
			return m_subject->RemoveModule(moduleName);
		}

		bool HasModule(std::string moduleName) const
		{
			if(m_subject.IsInvalid()) {
				return false;
			}
			return m_subject->HasModule(moduleName);
		}

		int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int notificationType, bool reverse)
		{
			CNotification notification(notificationName, request, notificationType);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response(reply);
			util::CAutoPointer<CResponse> pResponse(&response, false);
			this->NotifyObservers(pRequest, pResponse, reverse);
			return response.GetResult();
		}

		int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply)
		{
			CNotification notification(notificationName, request);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response(reply);
			util::CAutoPointer<CResponse> pResponse(&response, false);
			this->NotifyObservers(pRequest, pResponse, false);
			return response.GetResult();
		}

		int SendNotification(int notificationName, util::CWeakPointer<IBody> request)
		{
			CNotification notification(notificationName, request);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response;
			util::CAutoPointer<CResponse> pResponse(&response, false);
			this->NotifyObservers(pRequest, pResponse, false);
			return response.GetResult();
		}

		int SendNotification(int notificationName, int notificationType)
		{
			CNotification notification(notificationName, notificationType);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response;
			util::CAutoPointer<CResponse> pResponse(&response, false);
			this->NotifyObservers(pRequest, pResponse, false);
			return response.GetResult();
		}

		int SendNotification(int notificationName)
		{
			CNotification notification(notificationName);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response;
			util::CAutoPointer<CResponse> pResponse(&response, false);
			this->NotifyObservers(pRequest, pResponse, false);
			return response.GetResult();
		}

		void NotifyObservers(util::CWeakPointer<INotification> request,
			util::CWeakPointer<IResponse> response, bool reverse)
		{
			if (m_subject.IsInvalid()) {
				return;
			}
			return m_subject->NotifyObservers(request, response, reverse);
		}

		//////////////////////////////////////////////////////////////////
		int SendProtocol(int cmd, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int type, bool reverse, bool result)
		{
			CNotification notification(cmd, request, type);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			int nResult = result ? TRUE : FALSE;
			CResponse response(reply, nResult);
			util::CAutoPointer<CResponse> pResponse(&response, false);
			NotifyProtocol(pRequest, pResponse, reverse);
			return response.GetResult();
		}

		int SendProtocol(int cmd, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply)
		{
			CNotification notification(cmd, request);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response(reply);
			util::CAutoPointer<CResponse> pResponse(&response, false);
			NotifyProtocol(pRequest, pResponse, false);
			return response.GetResult();
		}

		int SendProtocol(int cmd, util::CWeakPointer<IBody> request)
		{
			CNotification notification(cmd, request);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response;
			util::CAutoPointer<CResponse> pResponse(&response, false);
			NotifyProtocol(pRequest, pResponse, false);
			return response.GetResult();
		}

		int SendProtocol(int cmd, int type)
		{
			CNotification notification(cmd, type);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response;
			util::CAutoPointer<CResponse> pResponse(&response, false);
			NotifyProtocol(pRequest, pResponse, false);
			return response.GetResult();
		}

		int SendProtocol(int cmd)
		{
			CNotification notification(cmd);
			util::CAutoPointer<CNotification> pRequest(&notification, false);

			CResponse response;
			util::CAutoPointer<CResponse> pResponse(&response, false);
			NotifyProtocol(pRequest, pResponse, false);
			return response.GetResult();
		}

		void NotifyProtocol(util::CWeakPointer<INotification> request,
			util::CWeakPointer<IResponse> response, bool reverse)
		{
			if (m_subject.IsInvalid()) {
				return;
			}
			m_subject->NotifyProtocol(request, response, reverse);
		}
		//////////////////////////////////////////////////////////////////
		void IterateModule(std::vector<util::CAutoPointer<IModule> >& outModules) const
		{
			if (m_subject.IsInvalid()){
				return;
			}
			return m_subject->IterateModule(outModules);
		}

	protected:
		// References CSubject
		util::CAutoPointer<ISubject> m_subject;
	};

}

#endif	/* MODULEMANAGER_H */

