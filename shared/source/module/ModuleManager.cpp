/*
 * File:   ModuleManager.cpp
 * Author: Jehu Shaw
 *
 * Created on 2011年3月14日, 下午3:27
 */

#include "ModuleEnv.h"

using namespace util;
using namespace thd;

namespace mdl {

//--------------------------------------
//  CNotifier
//--------------------------------------
util::CAutoPointer<IFacade> CNotifier::GetFacade()
{
	return CFacade::Pointer();
}

//--------------------------------------
//  CSubject
//--------------------------------------

void CSubject::RegisterObserver(int notificationName, const CAutoPointer<IObserverRestricted>& observer, int type)
{
	CScopedWriteLock wtlock(m_observerMapLock);
	OBSERVER_MAP_T::iterator itOB(m_observerMap.lower_bound(notificationName));
    if(m_observerMap.end() == itOB || itOB->first != notificationName) {
		m_observerMap.insert(itOB, OBSERVER_MAP_T::value_type(notificationName, OBSERVER_VECTOR_T({observer})));
    } else {
        itOB->second.push_back(observer);
    }
}

void CSubject::NotifyObservers(CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& response, bool reverse)
{
	CScopedReadLock rdlock(m_observerMapLock);
	OBSERVER_MAP_T::iterator itPR(m_observerMap.find(request->GetName()));
	if(m_observerMap.end() == itPR) {
		return;
	}
	OBSERVER_VECTOR_T& observers = itPR->second;
	if(reverse){
		OBSERVER_VECTOR_T::reverse_iterator it(observers.rbegin());
		for(; it != observers.rend(); ++it) {
			(*it)->NotifyObserver(request, response);
			// Reset position to start position
			if (!request.IsInvalid()) {
				request->ResetBody();
			}
			if (!response.IsInvalid()) {
				response->ResetBody();
			}
		}
	} else {
		OBSERVER_VECTOR_T::iterator it(observers.begin());
		for(; it != observers.end(); ++it) {
			(*it)->NotifyObserver(request, response);
			// Reset position to start position
			if (!request.IsInvalid()) {
				request->ResetBody();
			}
			if (!response.IsInvalid()) {
				response->ResetBody();
			}
		}
	}
}

void CSubject::RemoveObserver(int notificationName, intptr_t contextAddress, int type)
{
	CScopedWriteLock wtlock(m_observerMapLock);
	OBSERVER_MAP_T::iterator itOB(m_observerMap.find(notificationName));
	if(m_observerMap.end() == itOB) {
		return;
	}
    OBSERVER_VECTOR_T& observers = itOB->second;
    OBSERVER_VECTOR_T::iterator it(observers.begin());
    while(observers.end() != it) {
        if((*it)->CompareNotifyContext(contextAddress) == true) {
            it = observers.erase(it);
        } else {
			++it;
		}
    }

    if(observers.empty()) {
        m_observerMap.erase(itOB);
    }
}

void CSubject::RegisterModule(CAutoPointer<IModule>& module)
{
	if(!AddModule(module->GetModuleName(), module)) {
		return;
	}
    //////////////////////////////////////////////////////////////////////////
    IModule::InterestList protocols(module->ListProtocolInterests());
    if(protocols.empty()) {
		CAutoPointer<IObserverRestricted> pObserver(module->FullProtocolInterests());
		if(!pObserver.IsInvalid()) {
			this->RegisterProtocol(pObserver, 0);
		}
	} else {
        IModule::InterestList::const_iterator itP(protocols.begin());
        for(; itP != protocols.end(); ++itP) {
            const CAutoPointer<IObserverRestricted>& observer = (*itP)->GetObserver();
            this->RegisterProtocol((*itP)->GetNotificationName(), observer, 0);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    std::vector<int> interests(module->ListNotificationInterests());
    if(!interests.empty()) {
        CAutoPointer<IObserverRestricted> observer(new CObserver<IModule>(
			&IModule::HandleNotification, &*module));
        std::vector<int>::const_iterator itI(interests.begin());
        for(; itI != interests.end(); ++itI) {
            this->RegisterObserver(*itI, observer, 0);
        }
    }

    module->OnRegister();
}

CAutoPointer<IModule> CSubject::RetrieveModule(const std::string& moduleName) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	MODULE_MAP_T::const_iterator it(m_moduleMap.find(moduleName));
	if(m_moduleMap.end() != it) {
		return it->second;
	}
	return CAutoPointer<IModule>();
}

CAutoPointer<IModule> CSubject::RemoveModule(const std::string& moduleName)
{
     CAutoPointer<IModule> module(FindModule(moduleName));
     if(module.IsInvalid()) {
         return module;
     }
    //////////////////////////////////////////////////////////////////////////
    IModule::InterestList protocols(module->ListProtocolInterests());
	if(protocols.empty()) {
		CAutoPointer<IObserverRestricted> pObserver(module->FullProtocolInterests());
		if(!pObserver.IsInvalid()) {
			this->RemoveProtocol((intptr_t) &*module, 0);
		}
	} else {
		IModule::InterestList::const_iterator itP(protocols.begin());
		for(; itP != protocols.end(); ++itP) {
			this->RemoveProtocol((*itP)->GetNotificationName(), (intptr_t) &*module, 0);
		}
	}

    //////////////////////////////////////////////////////////////////////////
    std::vector<int> interests(module->ListNotificationInterests());
	std::vector<int>::const_iterator itI(interests.begin());
	for (; itI != interests.end(); ++itI) {
		this->RemoveObserver(*itI, (intptr_t) &*module, 0);
	}
    //////////////////////////////////////////////////////////////////////////
    EraseModule(moduleName);

    module->OnRemove();

    return module;
}

bool CSubject::HasModule(const std::string& moduleName) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
    return m_moduleMap.find(moduleName) != m_moduleMap.end();
}

void CSubject::RegisterProtocol(int notificationName, const CAutoPointer<IObserverRestricted>& observer, int type)
{
	CScopedWriteLock wtlock(m_protoMapLock);
	OBSERVER_MAP_T::iterator itPR(m_protoMap.lower_bound(notificationName));
    if(m_protoMap.end() == itPR || itPR->first != notificationName) {
		m_protoMap.insert(itPR, OBSERVER_MAP_T::value_type(notificationName, OBSERVER_VECTOR_T({observer})));
    } else {
        itPR->second.push_back(observer);
    }
}

void CSubject::RegisterProtocol(const CAutoPointer<IObserverRestricted>& observer, int type)
{
	CScopedWriteLock wtlock(m_protoLock);
	m_protoObserver.push_back(observer);
}

void CSubject::NotifyProtocol(CWeakPointer<INotification>& request, CWeakPointer<IResponse>& response, bool reverse)
{
    do {
        CScopedReadLock rdlock(m_protoMapLock);
        OBSERVER_MAP_T::iterator itPR(m_protoMap.find(request->GetName()));
        if(m_protoMap.end() == itPR) {
			break;
		}
        OBSERVER_VECTOR_T& observers = itPR->second;
        if(reverse) {
            OBSERVER_VECTOR_T::reverse_iterator it(observers.rbegin());
            for(; it != observers.rend(); ++it) {
                (*it)->NotifyObserver(request, response);
				// Reset position to start position
				if (!request.IsInvalid()) {
					request->ResetBody();
				}
				if (!response.IsInvalid()) {
					response->ResetBody();
				}
            }
        } else {
            OBSERVER_VECTOR_T::iterator it(observers.begin());
            for(; it != observers.end(); ++it) {
                (*it)->NotifyObserver(request, response);
				// Reset position to start position
				if (!request.IsInvalid()) {
					request->ResetBody();
				}
				if (!response.IsInvalid()) {
					response->ResetBody();
				}
            }
        }
    } while(false);
	// attach all
    if(response.IsInvalid() || response->GetResult() == FALSE) {
        CScopedReadLock rdlock(m_protoLock);
        OBSERVER_VECTOR_T::iterator itOB(m_protoObserver.begin());
        for(; m_protoObserver.end() != itOB; ++itOB) {
            (*itOB)->NotifyObserver(request, response);
			// Reset position to start position
			if (!request.IsInvalid()) {
				request->ResetBody();
			}
			if (!response.IsInvalid()) {
				response->ResetBody();
			}
        }
    }
}

void CSubject::RemoveProtocol(int notificationName, intptr_t contextAddress, int type)
{
	CScopedWriteLock wtlock(m_protoMapLock);
	OBSERVER_MAP_T::iterator itPR(m_protoMap.find(notificationName));
    if(m_protoMap.end() == itPR) {
		return;
	}
    OBSERVER_VECTOR_T& observers = itPR->second;
    OBSERVER_VECTOR_T::iterator it(observers.begin());
    while(observers.end() != it) {
        if((*it)->CompareNotifyContext(contextAddress) == true) {
            it = observers.erase(it);
        } else {
			++it;
		}
    }

    if(observers.empty()){
        m_protoMap.erase(itPR);
    }
}

void CSubject::RemoveProtocol(intptr_t contextAddress, int type)
{
	CScopedWriteLock wtlock(m_protoLock);
	OBSERVER_VECTOR_T::iterator it(m_protoObserver.begin());
	while(m_protoObserver.end() != it) {
		if((*it)->CompareNotifyContext(contextAddress) == true) {
			it = m_protoObserver.erase(it);
		} else {
			++it;
		}
	}
}

void CSubject::IterateModule(std::vector<CAutoPointer<IModule> >& outModules) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	MODULE_MAP_T::const_iterator it(m_moduleMap.begin());
	for(; m_moduleMap.end() != it; ++it) {
		outModules.push_back(it->second);
	}
}


//--------------------------------------
//  CSubjectType
//--------------------------------------

void CSubjectType::RegisterObserver(int notificationName, const CAutoPointer<IObserverRestricted>& observer, int type)
{
	CScopedWriteLock wtlock(m_observerMapLock);
	TYPE_OBSERVER_MAP_T::iterator itTO(m_observerMap.lower_bound(type));
	if(m_observerMap.end() == itTO || itTO->first != type) {
		itTO = m_observerMap.insert(itTO, TYPE_OBSERVER_MAP_T::value_type(type, OBSERVER_MAP_T()));
	}
	OBSERVER_MAP_T& observerMap = itTO->second;
	OBSERVER_MAP_T::iterator itO(observerMap.lower_bound(type));
	if(observerMap.end() == itO || itO->first != notificationName) {
		observerMap.insert(itO, OBSERVER_MAP_T::value_type(notificationName, OBSERVER_VECTOR_T({observer})));
	} else {
		itO->second.push_back(observer);
	}
}

void CSubjectType::NotifyObservers(CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& response, bool reverse)
{
	CScopedReadLock rdlock(m_observerMapLock);
	TYPE_OBSERVER_MAP_T::iterator itTO(m_observerMap.find(request->GetType()));
	if(m_observerMap.end() == itTO) {
		return;
	}
	OBSERVER_MAP_T& observerMap = itTO->second;
	OBSERVER_MAP_T::iterator itPR(observerMap.find(request->GetName()));
	if(observerMap.end() == itPR) {
		return;
	}
	OBSERVER_VECTOR_T& observers = itPR->second;
	if(reverse){
		OBSERVER_VECTOR_T::reverse_iterator it(observers.rbegin());
		for(; it != observers.rend(); ++it) {
			(*it)->NotifyObserver(request, response);
			// Reset position to start position
			if (!request.IsInvalid()) {
				request->ResetBody();
			}
			if (!response.IsInvalid()) {
				response->ResetBody();
			}
		}
	} else {
		OBSERVER_VECTOR_T::iterator it(observers.begin());
		for(; it != observers.end(); ++it) {
			(*it)->NotifyObserver(request, response);
			// Reset position to start position
			if (!request.IsInvalid()) {
				request->ResetBody();
			}
			if (!response.IsInvalid()) {
				response->ResetBody();
			}
		}
	}
}

void CSubjectType::RemoveObserver(int notificationName, intptr_t contextAddress, int type)
{
	CScopedWriteLock wtlock(m_observerMapLock);
	TYPE_OBSERVER_MAP_T::iterator itTO(m_observerMap.find(type));
	if(m_observerMap.end() == itTO) {
		return;
	}
	OBSERVER_MAP_T& observerMap = itTO->second;
	OBSERVER_MAP_T::iterator itO(observerMap.find(notificationName));
	if (observerMap.end() == itO) {
		return;
	}
	OBSERVER_VECTOR_T& observers = itO->second;
	OBSERVER_VECTOR_T::iterator it(observers.begin());
	while(observers.end() != it) {
		if((*it)->CompareNotifyContext(contextAddress) == true) {
			it = observers.erase(it);
		} else {
			++it;
		}
	}
	if (observers.empty()) {
		observerMap.erase(itO);
		if (observerMap.empty()) {
			m_observerMap.erase(itTO);
		}
	}
}

void CSubjectType::RegisterModule(CAutoPointer<IModule>& module)
{
	if(!AddModule(module->GetModuleName(), module)) {
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	IModule::InterestList protocols(module->ListProtocolInterests());
	if(protocols.empty()) {
		CAutoPointer<IObserverRestricted> pObserver(module->FullProtocolInterests());
		if(!pObserver.IsInvalid()) {
			this->RegisterProtocol(pObserver, module->GetType());
		}
	} else {
		IModule::InterestList::const_iterator itP(protocols.begin());
		for(; itP != protocols.end(); ++itP) {
			const CAutoPointer<IObserverRestricted>& observer = (*itP)->GetObserver();
			this->RegisterProtocol((*itP)->GetNotificationName(), observer, module->GetType());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	std::vector<int> interests(module->ListNotificationInterests());
	if(!interests.empty()) {
		CAutoPointer<IObserverRestricted> observer(new CObserver<IModule>(
			&IModule::HandleNotification, &*module));
		std::vector<int>::const_iterator itI(interests.begin());
		for(; itI != interests.end(); ++itI) {
			this->RegisterObserver(*itI, observer, module->GetType());
		}
	}

	const_cast<CAutoPointer<IModule>&>(module)->OnRegister();
}

CAutoPointer<IModule> CSubjectType::RetrieveModule(const std::string& moduleName) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	MODULE_MAP_T::const_iterator it(m_moduleMap.find(moduleName));
	if(m_moduleMap.end() != it) {
		return it->second;
	}
	return CAutoPointer<IModule>();
}

CAutoPointer<IModule> CSubjectType::RemoveModule(const std::string& moduleName)
{
	CAutoPointer<IModule> module(FindModule(moduleName));
	if(module.IsInvalid()) {
		return module;
	}
	//////////////////////////////////////////////////////////////////////////
	IModule::InterestList protocols(module->ListProtocolInterests());
	if(protocols.empty()) {
		CAutoPointer<IObserverRestricted> pObserver(module->FullProtocolInterests());
		if(!pObserver.IsInvalid()) {
			this->RemoveProtocol((intptr_t) &*module, module->GetType());
		}
	} else {
		IModule::InterestList::const_iterator itP(protocols.begin());
		for(; itP != protocols.end(); ++itP) {
			this->RemoveProtocol((*itP)->GetNotificationName(), (intptr_t) &*module, module->GetType());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	std::vector<int> interests(module->ListNotificationInterests());
	std::vector<int>::const_iterator itI(interests.begin());
	for (; itI != interests.end(); ++itI) {
		this->RemoveObserver(*itI, (intptr_t) &*module, module->GetType());
	}
	//////////////////////////////////////////////////////////////////////////
	EraseModule(moduleName);

	const_cast<CAutoPointer<IModule>&>(module)->OnRemove();

	return module;
}

bool CSubjectType::HasModule(const std::string& moduleName) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	return m_moduleMap.find(moduleName) != m_moduleMap.end();
}

void CSubjectType::RegisterProtocol(int notificationName, const CAutoPointer<IObserverRestricted>& observer, int type)
{
	CScopedWriteLock wtlock(m_protoMapLock);
	TYPE_OBSERVER_MAP_T::iterator itTO(m_protoMap.lower_bound(type));
	if(m_protoMap.end() == itTO || itTO->first != type) {
		itTO = m_protoMap.insert(itTO, TYPE_OBSERVER_MAP_T::value_type(type, OBSERVER_MAP_T()));
	}
	OBSERVER_MAP_T& protoMap = itTO->second;
	OBSERVER_MAP_T::iterator itPR(protoMap.lower_bound(notificationName));
	if(protoMap.end() == itPR || itPR->first != notificationName) {
		protoMap.insert(itPR, OBSERVER_MAP_T::value_type(notificationName, OBSERVER_VECTOR_T({observer})));
	} else {
		itPR->second.push_back(observer);
	}
}

void CSubjectType::RegisterProtocol(const CAutoPointer<IObserverRestricted>& observer, int type)
{
	CScopedWriteLock wtlock(m_protoLock);
	OBSERVER_MAP_T::iterator itPR(m_protoObserver.lower_bound(type));
	if(m_protoObserver.end() == itPR || itPR->first != type) {
		m_protoObserver.insert(itPR, OBSERVER_MAP_T::value_type(type, OBSERVER_VECTOR_T({observer})));
	} else {
		itPR->second.push_back(observer);
	}
}

void CSubjectType::NotifyProtocol(CWeakPointer<INotification>& request, CWeakPointer<IResponse>& response, bool reverse)
{
	do {
		CScopedReadLock rdlock(m_protoMapLock);
		TYPE_OBSERVER_MAP_T::iterator itTO(m_protoMap.find(request->GetType()));
		if(m_protoMap.end() == itTO) {
			break;
		}
		OBSERVER_MAP_T& protoMap = itTO->second;
		OBSERVER_MAP_T::iterator itPR(protoMap.find(request->GetName()));
		if(protoMap.end() == itPR) {
			break;
		}
		OBSERVER_VECTOR_T& observers = itPR->second;
		if(reverse) {
			OBSERVER_VECTOR_T::reverse_iterator it(observers.rbegin());
			for(; it != observers.rend(); ++it) {
				(*it)->NotifyObserver(request, response);
				// Reset position to start position
				if (!request.IsInvalid()) {
					request->ResetBody();
				}
				if (!response.IsInvalid()) {
					response->ResetBody();
				}
			}
		} else {
			OBSERVER_VECTOR_T::iterator it(observers.begin());
			for(; it != observers.end(); ++it) {
				(*it)->NotifyObserver(request, response);
				// Reset position to start position
				if (!request.IsInvalid()) {
					request->ResetBody();
				}
				if (!response.IsInvalid()) {
					response->ResetBody();
				}
			}
		}
	} while(false);
	// attach all
	if(response.IsInvalid() || response->GetResult() == FALSE) {
		CScopedReadLock rdlock(m_protoLock);
		OBSERVER_MAP_T::iterator itO(m_protoObserver.find(request->GetType()));
		if (m_protoObserver.end() != itO) {
			OBSERVER_VECTOR_T& observers = itO->second;
			OBSERVER_VECTOR_T::iterator it(observers.begin());
			for(; observers.end() != it; ++it) {
				(*it)->NotifyObserver(request, response);
				// Reset position to start position
				if (!request.IsInvalid()) {
					request->ResetBody();
				}
				if (!response.IsInvalid()) {
					response->ResetBody();
				}
			}
		}
	}
}

void CSubjectType::RemoveProtocol(int notificationName, intptr_t contextAddress, int type)
{
	CScopedWriteLock wtlock(m_protoMapLock);
	TYPE_OBSERVER_MAP_T::iterator itTO(m_protoMap.find(type));
	if (m_protoMap.end() == itTO) {
		return;
	}
	OBSERVER_MAP_T& protoMap = itTO->second;
	OBSERVER_MAP_T::iterator itPR(protoMap.find(notificationName));
	if (protoMap.end() == itPR) {
		return;
	}
	OBSERVER_VECTOR_T& observers = itPR->second;
	OBSERVER_VECTOR_T::iterator it(observers.begin());
	while(observers.end() != it) {
		if((*it)->CompareNotifyContext(contextAddress) == true) {
			it = observers.erase(it);
		} else {
			++it;
		}
	}

	if (observers.empty()){
		protoMap.erase(itPR);
		if (protoMap.empty()) {
			m_protoMap.erase(itTO);
		}
	}
}

void CSubjectType::RemoveProtocol(intptr_t contextAddress, int type)
{
	CScopedWriteLock wtlock(m_protoLock);
	OBSERVER_MAP_T::iterator itPR(m_protoObserver.find(type));
	if(m_protoObserver.end() == itPR) {
		return;
	}
	OBSERVER_VECTOR_T& observers = itPR->second;
	OBSERVER_VECTOR_T::iterator it(observers.begin());
	while(observers.end() != it) {
		if((*it)->CompareNotifyContext(contextAddress) == true) {
			it = observers.erase(it);
		} else {
			++it;
		}
	}
	if(observers.empty()){
		m_protoObserver.erase(itPR);
	}
}

void CSubjectType::IterateModule(std::vector<CAutoPointer<IModule> >& outModules) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	MODULE_MAP_T::const_iterator it(m_moduleMap.begin());
	for(; m_moduleMap.end() != it; ++it) {
		outModules.push_back(it->second);
	}
}


} // end namespace mdl


