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
//  CResponse
//--------------------------------------

void CResponse::SetResult(int result)
{
	this->m_result = result;
}

int CResponse::GetResult() const
{
	return this->m_result;
}

void CResponse::SetBody(const CWeakPointer<IBody> body)
{
	this->m_body = body;
}

CWeakPointer<IBody> CResponse::GetBody() const
{
	return this->m_body;
}

//--------------------------------------
//  CNotification
//--------------------------------------
CNotification::CNotification(int name, const CWeakPointer<IBody>& body, int notificationType)
{
    this->name = name;
    this->SetBody(body);
    this->SetType(notificationType);
}

CNotification::CNotification(int name, const CWeakPointer<IBody>& body)
{
    this->name = name;
    this->SetBody(body);
}

CNotification::CNotification(int name, int notificationType)
{
    this->name = name;
    this->SetType(notificationType);
}

CNotification::CNotification(int name)
{
    this->name = name;
}

int CNotification::GetName() const
{
    return this->name;
}

void CNotification::SetBody(const CWeakPointer<IBody>& body)
{
    this->body = body;
}

const CWeakPointer<IBody>& CNotification::GetBody() const
{
    return this->body;
}

void CNotification::SetType(int notificationType)
{
    this->type = notificationType;
}

int CNotification::GetType() const
{
    return this->type;
}

void CNotification::ResetBody()
{
    if(this->body.IsInvalid())
        return;

    this->body->ResetBody();
}

//--------------------------------------
//  CNotifier
//--------------------------------------
int CNotifier::SendNotification(int notificationName, CWeakPointer<IBody> request,
	CWeakPointer<IBody> reply, int notificationType, bool reverse)
{
    return this->GetFacade()->SendNotification(notificationName, request, reply,
		notificationType, reverse);
}

int CNotifier::SendNotification(int notificationName, CWeakPointer<IBody> request,
	CWeakPointer<IBody> reply)
{
	return this->GetFacade()->SendNotification(notificationName, request, reply);
}

int CNotifier::SendNotification(int notificationName, CWeakPointer<IBody> request)
{
	return this->GetFacade()->SendNotification(notificationName, request);
}

int CNotifier::SendNotification(int notificationName, int notificationType)
{
    return this->GetFacade()->SendNotification(notificationName, notificationType);
}

int CNotifier::SendNotification(int notificationName)
{
    return this->GetFacade()->SendNotification(notificationName);
}

util::CAutoPointer<IFacade> CNotifier::GetFacade()
{
	return CFacade::Pointer();
}

//--------------------------------------
//  CModule
//--------------------------------------
CModule::CModule()
{
}

CModule::CModule(const char* moduleName)
{
	this->moduleName = moduleName;
}

CModule::CModule(const std::string& moduleName)
{
	this->moduleName = moduleName;
}

//--------------------------------------
//  CSubject
//--------------------------------------

CSubject::CSubject( )
{
}

void CSubject::RegisterObserver(int notificationName, const CAutoPointer<IObserverRestricted>& observer)
{
	CScopedWriteLock wtlock(m_observerMapLock);
	OBSERVER_MAP_T::iterator itOB = this->observerMap.find(notificationName);
    if(this->observerMap.end() == itOB)
    {
        OBSERVER_VECTOR_T observerVector;
        observerVector.push_back(observer);
        this->observerMap[notificationName] = observerVector;
    }
    else
    {
        itOB->second.push_back(observer);
    }
}

void CSubject::NotifyObservers(CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& response, bool reverse)
{
	if(true) {
		CScopedReadLock rdlock(m_observerMapLock);
		OBSERVER_MAP_T::const_iterator itPR(this->observerMap.find(request->GetName()));
		if(this->observerMap.end() != itPR) {
			const OBSERVER_VECTOR_T& observers = itPR->second;
			if(reverse){
				OBSERVER_VECTOR_T::const_reverse_iterator it(observers.rbegin());
				for(; it != observers.rend(); ++it) {
					const_cast<util::CAutoPointer<IObserverRestricted>&>(*it)->
					NotifyObserver(request, response);
					request->ResetBody();
				}
			}else{
				OBSERVER_VECTOR_T::const_iterator it(observers.begin());
				for(; it != observers.end(); ++it) {
					const_cast<util::CAutoPointer<IObserverRestricted>&>(*it)->
					NotifyObserver(request, response);
					request->ResetBody();
				}
			}
		}
	}

}

void CSubject::RemoveObserver(int notificationName, intptr_t contextAddress)
{
	CScopedWriteLock wtlock(m_observerMapLock);
	OBSERVER_MAP_T::iterator itOB = this->observerMap.find(notificationName);
	if(this->observerMap.end() != itOB)
    {
        OBSERVER_VECTOR_T& observers = itOB->second;
        OBSERVER_VECTOR_T::iterator it(observers.begin());
        while(observers.end() != it)
        {
            if((*it)->CompareNotifyContext(contextAddress) == true)
            {
                it = observers.erase(it);
            } else {
				++it;
			}
        }

        if(observers.empty()){
           this->observerMap.erase(itOB);
        }
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
			this->RegisterProtocol(pObserver);
		}
	} else {
        IModule::InterestList::const_iterator itP(protocols.begin());
        for(; itP != protocols.end(); ++itP) {
            const CAutoPointer<IObserverRestricted>& observer = (*itP)->GetObserver();
            this->RegisterProtocol((*itP)->GetNotificationName(), observer);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    std::vector<int> interests(module->ListNotificationInterests());
    if(!interests.empty()) {
        CAutoPointer<IObserverRestricted> observer(new CObserver<IModule>(
			&IModule::HandleNotification, &*module));
        std::vector<int>::const_iterator itI(interests.begin());
        for(; itI != interests.end(); ++itI) {
            this->RegisterObserver(*itI, observer);
        }
    }

    const_cast<CAutoPointer<IModule>&>(module)->OnRegister();
}

CAutoPointer<IModule> CSubject::RetrieveModule(const std::string& moduleName) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	MODULE_MAP_T::const_iterator it(this->moduleMap.find(moduleName));
	if(this->moduleMap.end() != it) {
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
			this->RemoveProtocol((intptr_t) &*module);
		}
	} else {
		IModule::InterestList::const_iterator itP(protocols.begin());
		for(; itP != protocols.end(); ++itP)
		{
			this->RemoveProtocol((*itP)->GetNotificationName(), (intptr_t) &*module);
		}
	}

    //////////////////////////////////////////////////////////////////////////
    std::vector<int> interests(module->ListNotificationInterests());
	std::vector<int>::const_iterator itI(interests.begin());
	for (; itI != interests.end(); ++itI)
	{
		this->RemoveObserver(*itI, (intptr_t) &*module);
	}
    //////////////////////////////////////////////////////////////////////////
    EraseModule(moduleName);

    const_cast<CAutoPointer<IModule>&>(module)->OnRemove();

    return module;
}

bool CSubject::HasModule(const std::string& moduleName) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
    return this->moduleMap.find(moduleName) != this->moduleMap.end();
}

void CSubject::RegisterProtocol(int notificationName, const CAutoPointer<IObserverRestricted>& observer)
{
	CScopedWriteLock wtlock(m_protocolMapLock);
	PROTOCOL_MAP_T::iterator itPR = this->protocolMap.find(notificationName);
    if(this->protocolMap.end() == itPR)
    {
        OBSERVER_VECTOR_T observerVector;
        observerVector.push_back(observer);
        this->protocolMap[notificationName] = observerVector;
    }
    else
    {
        itPR->second.push_back(observer);
    }
}

void CSubject::RegisterProtocol(const CAutoPointer<IObserverRestricted>& observer)
{
	CScopedWriteLock wtlock(m_protocolsLock);
	protocolObserver.push_back(observer);
}

void CSubject::NotifyProtocol(CWeakPointer<INotification>& request, CWeakPointer<IResponse>& response, bool reverse)
{
    if(true) {
        CScopedReadLock rdlock(m_protocolMapLock);
        PROTOCOL_MAP_T::const_iterator itPR(this->protocolMap.find(request->GetName()));
        if(this->protocolMap.end() != itPR) {
            const OBSERVER_VECTOR_T& observers = itPR->second;
            if(reverse) {
                OBSERVER_VECTOR_T::const_reverse_iterator it(observers.rbegin());
                for(; it != observers.rend(); ++it) {
                    const_cast<util::CAutoPointer<IObserverRestricted>&>(*it)->
                    NotifyObserver(request, response);
                    request->ResetBody();
                }
            } else {
                OBSERVER_VECTOR_T::const_iterator it(observers.begin());
                for(; it != observers.end(); ++it) {
                    const_cast<util::CAutoPointer<IObserverRestricted>&>(*it)->
                    NotifyObserver(request, response);
                    request->ResetBody();
                }
            }
        }
    }
	// attach all
    if(response.IsInvalid() || response->GetResult() == FALSE) {
        CScopedReadLock rdlock(m_protocolsLock);
        OBSERVER_VECTOR_T::const_iterator itOB(protocolObserver.begin());
        for(; protocolObserver.end() != itOB; ++itOB){
            const_cast<util::CAutoPointer<IObserverRestricted>&>(*itOB)->
            NotifyObserver(request, response);
            request->ResetBody();
        }
    }
}

void CSubject::RemoveProtocol(int notificationName, intptr_t contextAddress)
{
	CScopedWriteLock wtlock(m_protocolMapLock);
	PROTOCOL_MAP_T::iterator itPR = this->protocolMap.find(notificationName);
    if(this->protocolMap.end() != itPR)
    {
        OBSERVER_VECTOR_T& observers = itPR->second;
        OBSERVER_VECTOR_T::iterator it(observers.begin());
        while(observers.end() != it)
        {
            if((*it)->CompareNotifyContext(contextAddress) == true)
            {
                it = observers.erase(it);
            } else {
				++it;
			}
        }

        if(observers.empty()){
            this->protocolMap.erase(itPR);
        }
    }
}

void CSubject::RemoveProtocol(intptr_t contextAddress)
{
	CScopedWriteLock wtlock(m_protocolsLock);
	OBSERVER_VECTOR_T::iterator it(protocolObserver.begin());
	while(protocolObserver.end() != it)
	{
		if((*it)->CompareNotifyContext(contextAddress) == true)
		{
			it = protocolObserver.erase(it);
		} else {
			++it;
		}
	}
}

void CSubject::IterateModule(std::vector<CAutoPointer<IModule> >& outModules) const
{
	CScopedReadLock rdlock(m_moduleMapLock);
	MODULE_MAP_T::const_iterator it(this->moduleMap.begin());
	for(; this->moduleMap.end() != it; ++it) {
		outModules.push_back(it->second);
	}
}

//--------------------------------------
//  CFacade
//--------------------------------------

CFacade::CFacade():subject(new CSubject())
{
}

CFacade::~CFacade()
{
	subject.SetRawPointer(NULL);
}

void CFacade::RegisterModule(CAutoPointer<IModule> module)
{
    if(this->subject.IsInvalid())
        return;
    this->subject->RegisterModule(module);
}

CAutoPointer<IModule> CFacade::RetrieveModule(std::string moduleName) const
{
    if(this->subject.IsInvalid())
        return CAutoPointer<IModule>();
    return this->subject->RetrieveModule(moduleName);
}

CAutoPointer<IModule> CFacade::RemoveModule(std::string moduleName)
{
    if(this->subject.IsInvalid())
        return CAutoPointer<IModule>();
    return this->subject->RemoveModule(moduleName);
}

bool CFacade::HasModule(std::string moduleName) const
{
    if(this->subject.IsInvalid())
        return false;
    return this->subject->HasModule(moduleName);
}

int CFacade::SendNotification(int notificationName, CWeakPointer<IBody> request,
	CWeakPointer<IBody> reply, int notificationType, bool reverse)
{
	CNotification notification(notificationName, request, notificationType);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response(reply);
	CAutoPointer<CResponse> pResponse(&response, false);
    this->NotifyObservers(pRequest, pResponse, reverse);
	return response.GetResult();
}

int CFacade::SendNotification(int notificationName, CWeakPointer<IBody> request,
	CWeakPointer<IBody> reply)
{
	CNotification notification(notificationName, request);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response(reply);
	CAutoPointer<CResponse> pResponse(&response, false);
	this->NotifyObservers(pRequest, pResponse, false);
	return response.GetResult();
}

int CFacade::SendNotification(int notificationName, CWeakPointer<IBody> request)
{
	CNotification notification(notificationName, request);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response;
	CAutoPointer<CResponse> pResponse(&response, false);
    this->NotifyObservers(pRequest, pResponse, false);
	return response.GetResult();
}

int CFacade::SendNotification(int notificationName, int notificationType)
{
	CNotification notification(notificationName, notificationType);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response;
	CAutoPointer<CResponse> pResponse(&response, false);
	this->NotifyObservers(pRequest, pResponse, false);
	return response.GetResult();
}

int CFacade::SendNotification(int notificationName)
{
	CNotification notification(notificationName);
    CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response;
	CAutoPointer<CResponse> pResponse(&response, false);
    this->NotifyObservers(pRequest, pResponse, false);
	return response.GetResult();
}

void CFacade::NotifyObservers(CWeakPointer<INotification> request, CWeakPointer<IResponse> response, bool reverse)
{
    if(this->subject.IsInvalid())
        return;
    return this->subject->NotifyObservers(request, response, reverse);
}

int CFacade::SendProtocol(int cmd, CWeakPointer<IBody> request,
	CWeakPointer<IBody> reply, int type, bool reverse, bool result) {

	CNotification notification(cmd, request, type);
	CAutoPointer<CNotification> pRequest(&notification, false);

	int nResult = result ? TRUE : FALSE;
	CResponse response(reply, nResult);
	CAutoPointer<CResponse> pResponse(&response, false);
	NotifyProtocol(pRequest, pResponse, reverse);
	return response.GetResult();
}

int CFacade::SendProtocol(int cmd, CWeakPointer<IBody> request,
	CWeakPointer<IBody> reply) {

	CNotification notification(cmd, request);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response(reply);
	CAutoPointer<CResponse> pResponse(&response, false);
	NotifyProtocol(pRequest, pResponse, false);
	return response.GetResult();
}

int CFacade::SendProtocol(int cmd, CWeakPointer<IBody> request) {

	CNotification notification(cmd, request);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response;
	CAutoPointer<CResponse> pResponse(&response, false);
	NotifyProtocol(pRequest, pResponse, false);
	return response.GetResult();
}

int CFacade::SendProtocol(int cmd, int type) {

	CNotification notification(cmd, type);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response;
	CAutoPointer<CResponse> pResponse(&response, false);
	NotifyProtocol(pRequest, pResponse, false);
	return response.GetResult();
}

int CFacade::SendProtocol(int cmd) {

	CNotification notification(cmd);
	CAutoPointer<CNotification> pRequest(&notification, false);

	CResponse response;
	CAutoPointer<CResponse> pResponse(&response, false);
	NotifyProtocol(pRequest, pResponse, false);
	return response.GetResult();
}

void CFacade::NotifyProtocol(CWeakPointer<INotification> request, CWeakPointer<IResponse> response, bool reverse)
{
    if(this->subject.IsInvalid())
        return;
    this->subject->NotifyProtocol(request, response, reverse);
}

void CFacade::IterateModule(std::vector<CAutoPointer<IModule> >& outModules) const
{
	if(this->subject.IsInvalid())
		return;
	return this->subject->IterateModule(outModules);
}

} // end namespace mdl


