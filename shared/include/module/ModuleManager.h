/*
 * File:   ModuleManager.h
 * Author: Jehu Shaw
 *
 * Created on 2011年3月14日, 下午3:27
 */

#ifndef _MODULEMANAGER_H
#define	_MODULEMANAGER_H

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
	};

	class SHARED_DLL_DECL ISubject
	{
	public:
		virtual ~ISubject(){}

		virtual void RegisterObserver(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer) = 0;

		virtual void RemoveObserver(int notificationName, intptr_t contextAddress) = 0;

		virtual void NotifyObservers(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse) = 0;

		virtual void RegisterModule(util::CAutoPointer<IModule>& module) = 0;

		virtual util::CAutoPointer<IModule> RetrieveModule(const std::string& moduleName) const = 0;

		virtual util::CAutoPointer<IModule> RemoveModule(const std::string& moduleName) = 0;

		virtual bool HasModule(const std::string& moduleName) const = 0;

		virtual void RegisterProtocol(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer) = 0;

		virtual void RegisterProtocol(const util::CAutoPointer<IObserverRestricted>& observer) = 0;

		virtual void NotifyProtocol(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse) = 0;

		virtual void RemoveProtocol(int notificationName, intptr_t contextAddress) = 0;

		virtual void RemoveProtocol(intptr_t contextAddress) = 0;

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

		virtual void SetResult(int result);

		virtual int GetResult() const;

		virtual void SetBody(const util::CWeakPointer<IBody> body);

		virtual util::CWeakPointer<IBody> GetBody() const;

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

		CNotification(int notificationName, const util::CWeakPointer<IBody>& body, int notificationType);
		CNotification(int notificationName, const util::CWeakPointer<IBody>& body);
		CNotification(int notificationName, int notificationType);
		CNotification(int notificationName);

		int GetName() const;

		void SetBody(const util::CWeakPointer<IBody>& body);

		const util::CWeakPointer<IBody>& GetBody() const;

		void SetType(int notificationType);

		int GetType() const;

		void ResetBody();

	private:
		int name;
		int type;
		util::CWeakPointer<IBody> body;
	};

	//--------------------------------------
	//  CNotifier
	//--------------------------------------
	class SHARED_DLL_DECL CNotifier : public virtual INotifier
	{
	public:

		virtual int SendNotification (int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int notificationType, bool reverse);

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply);

		virtual int SendNotification(int notificationName, util::CWeakPointer<IBody> request);

		virtual int SendNotification(int notificationName, int notificationType);

		virtual int SendNotification(int notificationName);

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
		{
			this->SetNotifyMethod(method);
			this->SetNotifyContext(context);
		}

		void SetNotifyMethod(typename IObserverTemplated<ST>::NotifyMethod method)
		{
			this->notifyMethod = method;
		}

		void SetNotifyContext(NotifyContext context)
		{
			this->notifyContext = context;
		}

		typename IObserverTemplated<ST>::NotifyMethod GetNotifyMethod() const
		{
			return this->notifyMethod;
		}

		NotifyContext GetNotifyContext() const
		{
			return this->notifyContext;
		}

		void NotifyObserver(const util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response)
		{
			(this->notifyContext->*notifyMethod)(request, response);
		}

		bool CompareNotifyContext(intptr_t compareContextAddress) const
		{
			return compareContextAddress == (intptr_t)(BT*)this->GetNotifyContext();
		}

	private:
		typename IObserverTemplated<ST>::NotifyMethod notifyMethod;
		NotifyContext notifyContext;
	};

	class SHARED_DLL_DECL CSubject : public ISubject
	{
	public:
		/**
		 * Constructor.
		 */
		CSubject();

		void RegisterObserver(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer);

		void NotifyObservers(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse);

		void RemoveObserver(int notificationName, intptr_t contextAddress);

		void RegisterModule(util::CAutoPointer<IModule>& module);

		util::CAutoPointer<IModule> RetrieveModule(const std::string& moduleName) const;

		util::CAutoPointer<IModule> RemoveModule(const std::string& moduleName);

		bool HasModule(const std::string& moduleName) const;

		void RegisterProtocol(int notificationName, const util::CAutoPointer<IObserverRestricted>& observer);

		void RegisterProtocol(const util::CAutoPointer<IObserverRestricted>& observer);

		void NotifyProtocol(util::CWeakPointer<INotification>& request, util::CWeakPointer<IResponse>& response, bool reverse);

		void RemoveProtocol(int notificationName, intptr_t contextAddress);

		void RemoveProtocol(intptr_t contextAddress);

		void IterateModule(std::vector<util::CAutoPointer<IModule> >& outModules) const;

	protected:
		typedef std::map<std::string, util::CAutoPointer<IModule> > MODULE_MAP_T;
		MODULE_MAP_T moduleMap;
		thd::CSpinRWLock m_moduleMapLock;

		typedef std::vector<util::CAutoPointer<IObserverRestricted> > OBSERVER_VECTOR_T;

		typedef std::map<int, OBSERVER_VECTOR_T > OBSERVER_MAP_T;
		OBSERVER_MAP_T observerMap;
		thd::CSpinRWLock m_observerMapLock;

		typedef std::map<int, OBSERVER_VECTOR_T > PROTOCOL_MAP_T;
		PROTOCOL_MAP_T protocolMap;
		thd::CSpinRWLock m_protocolMapLock;

		OBSERVER_VECTOR_T protocolObserver;
		thd::CSpinRWLock m_protocolsLock;

	protected:
		inline bool AddModule(const std::string& moduleName, const util::CAutoPointer<IModule>& module) {
			thd::CScopedWriteLock wtlock(m_moduleMapLock);
			std::pair<MODULE_MAP_T::iterator, bool> pairIB(this->moduleMap.insert(
				MODULE_MAP_T::value_type(moduleName, module)));
			return pairIB.second;
		}

        inline util::CAutoPointer<IModule> FindModule(const std::string& moduleName) const {
            thd::CScopedReadLock rdlock(m_moduleMapLock);

            MODULE_MAP_T::const_iterator it(this->moduleMap.find(moduleName));
            if(this->moduleMap.end() == it) {
                return util::CAutoPointer<IModule>();
            }
            return it->second;
        }

        inline void EraseModule(const std::string& moduleName) {
            thd::CScopedWriteLock wtlock(m_moduleMapLock);
            this->moduleMap.erase(moduleName);
        }

	};

	class SHARED_DLL_DECL CInterests : public IInterests {
	public:

		explicit CInterests(int nNotificationName, util::CAutoPointer<IObserverRestricted>& observer)
			: m_nNotificationName(nNotificationName)
			, m_observer(observer)
		{

		}

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
		CModule();
		CModule(const char* moduleName);
		CModule(const std::string& moduleName);

		virtual const std::string& GetModuleName() const
        {
            return this->moduleName;
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
		std::string moduleName;

	};

	class SHARED_DLL_DECL CFacade
		: public IFacade
		, public util::Singleton<CFacade>
	{
	public:
		CFacade();

		~CFacade();

		void RegisterModule(util::CAutoPointer<IModule> module);

		util::CAutoPointer<IModule> RetrieveModule(std::string moduleName) const;

		util::CAutoPointer<IModule> RemoveModule(std::string moduleName);

		bool HasModule(std::string moduleName) const;

		int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int notificationType, bool reverse);

		int SendNotification(int notificationName, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply);

		int SendNotification(int notificationName, util::CWeakPointer<IBody> request);

		int SendNotification(int notificationName, int notificationType);

		int SendNotification(int notificationName);

		void NotifyObservers(util::CWeakPointer<INotification> request,
			util::CWeakPointer<IResponse> response, bool reverse);

		//////////////////////////////////////////////////////////////////
		int SendProtocol(int cmd, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply, int type, bool reverse, bool result);

		int SendProtocol(int cmd, util::CWeakPointer<IBody> request,
			util::CWeakPointer<IBody> reply);

		int SendProtocol(int cmd, util::CWeakPointer<IBody> request);

		int SendProtocol(int cmd, int type);

		int SendProtocol(int cmd);

		void NotifyProtocol(util::CWeakPointer<INotification> request,
			util::CWeakPointer<IResponse> response, bool reverse);
		//////////////////////////////////////////////////////////////////
		void IterateModule(std::vector<util::CAutoPointer<IModule> >& outModules) const;

	protected:
		// References CSubject
		util::CAutoPointer<ISubject> subject;
	};

}

#endif	/* _MODULEMANAGER_H */

