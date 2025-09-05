/* 
 * File:   SessionEventManager.h
 * Author: Jehu Shaw
 *
 * Created on 2015_3_5 15:12
 */

#ifndef SESSIONEVENTMANAGER_H
#define	SESSIONEVENTMANAGER_H

#include "SimpleEvent.h"
#include "AgentMethod.h"
#include "BitStream.h"
#include "Singleton.h"

enum eSessionEventType {
    SESSION_EVENT_LOGIN_CHECK,
};

class CArgBitStream : public ntwk::BitStream, public evt::ArgumentBase
{
public:
	CArgBitStream(void):BitStream() {}
	CArgBitStream(int initialBytesToAllocate):BitStream(initialBytesToAllocate) {}
	CArgBitStream(const char* szData, unsigned int uSize, bool bCopy): BitStream((char*)szData, uSize, bCopy) {}

};

class CSessionEventManager
	: public util::Singleton<CSessionEventManager>
{
public:
    bool AddEventListener(int nId, const util::CAutoPointer<evt::MethodRIP1Base>& method) {
        return m_event.AddEventListener(nId, method);
    }
    int DispatchEvent(int nId, const util::CWeakPointer<evt::ArgumentBase>& arg) {
        return m_event.DispatchEvent(nId, arg);
    }
    bool HasEventListener(int nId) {
        return m_event.HasEventListener(nId);
    }
    void RemoveEventListener(int nId) {
        m_event.RemoveEventListener(nId);
    }

private:
    evt::SimpleEvent<int> m_event;
};

#endif /* SESSIONEVENTMANAGER_H */