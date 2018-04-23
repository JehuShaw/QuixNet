/*
 * File:   TCPPacketInterface.cpp
 * Author: Jehu Shaw
 *
 * Created on 2011_9_14 14:50
 */

#include "TCPPacketInterface.h"
#include "NetworkTypes.h"
#include "NetworkTrace.h"
#include <algorithm>
#include <string.h>

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#include <process.h>
#endif

using namespace thd;

namespace ntwk
{
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    unsigned int _stdcall DispatchInterfaceLoop(void * arguments) {
#else
    void * DispatchInterfaceLoop(void * arguments) {
#endif
        TCPPacketInterface * sts = (TCPPacketInterface *) arguments;
        sts->DispatchRun();
        return 0;
    }

    class PacketLinkData : public ILinkData {
    public:
        PacketLinkData() {}

        void Clear(){ ingoingQueue.Clear(); }

        SingleProducerConsumer<Packet> ingoingQueue;

    };

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
TCPPacketInterface::TCPPacketInterface():TCPSocketWin32(), isDispatched(true),packetSizeLimit(0) {

}
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
TCPPacketInterface::TCPPacketInterface():TCPSocketBSD(sock_global_domain), isDispatched(true),packetSizeLimit(0) {

}

TCPPacketInterface::TCPPacketInterface(socket_domain domain):TCPSocketBSD(domain), isDispatched(true),packetSizeLimit(0) {

}
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
TCPPacketInterface::TCPPacketInterface():TCPSocketLinux(sock_global_domain), isDispatched(true),packetSizeLimit(0) {

}

TCPPacketInterface::TCPPacketInterface(socket_domain domain):TCPSocketLinux(domain), isDispatched(true),packetSizeLimit(0) {

}
#endif

TCPPacketInterface::~TCPPacketInterface() {
}

bool TCPPacketInterface::Start(const char* address,unsigned short maxLink, uint32_t maxPacketSize) {
    this->packetSizeLimit = maxPacketSize;

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    bool ret = this->TCPSocketWin32::Start(address,maxLink);
    if(!ret) {
        return false;
    }

	dispatchDone.Suspend();
    HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, DispatchInterfaceLoop, (LPVOID)this, 0, NULL);
    if(threadHandle == NULL) {
        TRACE_MSG("Dispatch, _beginthreadex failed\n");
		dispatchDone.Resume();
        this->TCPSocketWin32::Stop();
        return false;
    }
    CloseHandle(threadHandle);
#else
#if defined( __NetBSD__ ) || defined( __APPLE__ )
    bool ret = this->TCPSocketBSD::Start(address,maxLink);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    bool ret = this->TCPSocketLinux::Start(address,maxLink);
#endif
    if(!ret) {
        return false;
    }

	dispatchDone.Suspend();
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&tid, &attr, DispatchInterfaceLoop, this) != 0){
        TRACE_MSG("Dispatch, pthread_create failed\n");
		dispatchDone.Resume();
#if defined( __NetBSD__ ) || defined( __APPLE__ )
		this->TCPSocketBSD::Stop();
#else
        this->TCPSocketLinux::Stop();
#endif
        return false;
    }
#endif

    return ret;
}

bool TCPPacketInterface::Stop(void) {

    if(atomic_cmpxchg8(&isDispatched
        , false, true) != (char)true) {

            return false;
    }

    dispatchDone.Wait();

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    if(this->TCPSocketWin32::Stop()){
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
    if(this->TCPSocketBSD::Stop()){
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    if(this->TCPSocketLinux::Stop()){
#endif
        newConnections.Clear();
        lostConnections.Clear();
        return true;
    }
    return false;
}

bool TCPPacketInterface::Connect(SocketID& socketId, const char* address, uint32_t maxPacketSize) {
    this->packetSizeLimit = maxPacketSize;

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )

    if(!this->TCPSocketWin32::Connect(socketId, address)) {
        return false;
    }

	dispatchDone.Suspend();
    HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, DispatchInterfaceLoop, (LPVOID)this, 0, NULL);
    if(threadHandle == NULL) {
        TRACE_MSG("Dispatch, _beginthreadex failed\n");
		dispatchDone.Resume();
        this->TCPSocketWin32::Stop();
        return false;
    }
    CloseHandle(threadHandle);
#else
#if defined( __NetBSD__ ) || defined( __APPLE__ )
    if(!this->TCPSocketBSD::Connect(socketId,address)) {
        return false;
    }
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    if(!this->TCPSocketLinux::Connect(socketId,address)) {
        return false;
    }
#endif

	dispatchDone.Suspend();
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&tid, &attr, DispatchInterfaceLoop, this) != 0){
        TRACE_MSG("Dispatch, pthread_create failed\n");
		dispatchDone.Resume();
#if defined( __NetBSD__ ) || defined( __APPLE__ )
		this->TCPSocketBSD::Stop();
#else
        this->TCPSocketLinux::Stop();
#endif
        return false;
    }
#endif

	int* pInt = loginQueue.WriteLock();
	*pInt = socketId.index;
	loginQueue.WriteUnlock();

    return true;
}

int TCPPacketInterface::ReceiveCallback(
	SocketLink& socketLink,
    const unsigned char* buffer,
    const unsigned int length,
    const unsigned char* moreBuffer,
    const unsigned int moreLength)
{
    //split packet
    uint32_t offset(0);
    int32_t leave(0);
    for(;;) {
        leave = length + moreLength - offset;
        if(leave > 7){
            uint32_t len(0),total(0);
            if(m_isLittleEndian){
                // read total uint32
                SwitchReverseBytes((unsigned char*)&total,sizeof(total),buffer
                    ,length,moreBuffer,moreLength,offset);
                // read len uint32
                SwitchReverseBytes((unsigned char*)&len,sizeof(len),buffer,length
                    ,moreBuffer,moreLength,offset+sizeof(total));
            }else{
                // read total uint32
                SwitchCopyBytes((unsigned char*)&total,sizeof(total),buffer
                    ,length, moreBuffer, moreLength,offset);
                // read len uint32
                SwitchCopyBytes((unsigned char*)&len,sizeof(len),buffer,length
                    ,moreBuffer,moreLength,offset+sizeof(total));
            }
            if(len > packetSizeLimit) {
                // Limit the size of packet
                return -1;
            }
            if((len + 4) == total){
                if((int)(4 + total) <= leave){
                    offset += 8;
                    //push the data
                    PacketLinkData* ptr = dynamic_cast<PacketLinkData*>(socketLink.GetLinkerData());
                    if(ptr != NULL){
                        Packet * p = ptr->ingoingQueue.WriteLock();
                        p->length = len;
                        SwitchMemcpy(p->data,buffer,length,moreBuffer,moreLength,len,offset);
                        ptr->ingoingQueue.WriteUnlock();
                    }
                    offset += len;
                    continue;
                }
            }else{
                return -1;
            }
        }
        break;
    }

    return offset;
}

void TCPPacketInterface::SendCallback(
	Packet& dstPacket,
	const unsigned char* data,
	const unsigned int length,
	const unsigned char* prefixData,
	const unsigned int prefixLength)
{
	uint32_t dataLen  = length;
	if(NULL != prefixData) {
		dataLen += prefixLength;
	}
	//write data
	uint32_t total = (uint32_t)(dataLen + 4);
	dstPacket.length = total + 4;
	dstPacket.data.Put(dstPacket.length);
	char * temp =  (char*)dstPacket.data;
	if(m_isLittleEndian){
		ReverseBytes((char *)&total,temp,4);
		ReverseBytes((char *)&dataLen,temp + 4,4);
	}else{
		memcpy(temp,&total,4);
		memcpy(temp + 4,&dataLen,4);
	}
	if(NULL != prefixData) {
		memcpy((temp + 8),(const char*)prefixData,prefixLength);
		memcpy((temp + 8 + prefixLength),(const char*)data,length);
	} else {
		memcpy((temp + 8),(const char*)data,length);
	}
}

ILinkData* TCPPacketInterface::AllocateLinkerData() {
    return new PacketLinkData();
}

void TCPPacketInterface::AcceptCallback(SocketLink& socketLink) {

	size_t index = XQXTABLE0S_INDEX_NIL;
	if(m_pSktLinks) {
		index = m_pSktLinks->GetIndexByPtr(&socketLink);
	} else {
		assert(m_pSktLinks);
	}

	if(XQXTABLE0S_INDEX_NIL == index) {
		assert(false);
		return;
	}

    SocketID *temp = newConnections.WriteLock();
    socketLink.GetSocketID(*temp, index);
    newConnections.WriteUnlock();

    int* pInt = loginQueue.WriteLock();
    *pInt = (int)index;
    loginQueue.WriteUnlock();
}

void TCPPacketInterface::DisconnectCallback(SocketLink& socketLink) {

	size_t index = XQXTABLE0S_INDEX_NIL;
	if(m_pSktLinks) {
		index = m_pSktLinks->GetIndexByPtr(&socketLink);
	} else {
		assert(m_pSktLinks);
	}

	if(XQXTABLE0S_INDEX_NIL == index) {
		assert(false);
		return;
	}

    SocketID *temp = lostConnections.WriteLock();
    socketLink.GetSocketID(*temp, index);
    lostConnections.WriteUnlock();

    int* pInt = logoutQueue.WriteLock();
    *pInt = (int)index;
    logoutQueue.WriteUnlock();
}

#pragma warning( push )
#pragma warning( disable : 4100 )

void TCPPacketInterface::MisdataCallback(
	SocketLink& socketLink,
    const unsigned char* buffer,
    const unsigned int length) {

}

#pragma warning( pop )

void TCPPacketInterface::DispatchRun() {
    while(isDispatched){

		int nLgnSize = loginQueue.Size();
		for(int i = 0; i < nLgnSize; ++i) {
			int* pIntLogin = loginQueue.ReadLock();
			if(NULL == pIntLogin) {
				break;
			}
			int nIndex = *pIntLogin;
			loginQueue.ReadUnlock();
			if(XQXTABLE0S_INDEX_NIL != nIndex){
				registerList.insert(nIndex);
			} else {
				assert(false);
			}
		}

		int nLgoSize = logoutQueue.Size();
		for(int i = 0; i < nLgoSize; ++i) {
			int* pIntLogout = logoutQueue.ReadLock();
			if(NULL == pIntLogout) {
				break;
			}
			int nIndex = *pIntLogout;
			logoutQueue.ReadUnlock();
			registerList.erase(nIndex);
		}

		REGISTER_LIST_T::iterator it = registerList.begin();
		while(registerList.end() != it) {
			if(!m_isStarted) {
				++it;
				continue;
			}
			if(NULL == m_pSktLinks) {
				++it;
				continue;
			}

			socket_links_t::value_t val(m_pSktLinks->Find(*it));
			if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
				++it;
				continue;
			}

			SocketLink* pSktLink = val.pObject;
			if(NULL == pSktLink) {
				++it;
				assert(pSktLink);
				continue;
			}
			//assert(socketLink.GetIndex() != SOCKETID_INDEX_NULL);
			PacketLinkData* ptr = dynamic_cast<PacketLinkData*>(pSktLink->GetLinkerData());
			if(NULL == ptr){
				++it;
				continue;
			}
			int nIgoSize = ptr->ingoingQueue.Size();
			for(int i = 0; i < nIgoSize; ++i) {
				Packet* pPacket = ptr->ingoingQueue.ReadLock();
				if(NULL == pPacket) {
					break;
				}
				ReceivePacket * pReceive = totalIngoingQueue.WriteLock();
				pReceive->packet.data.Put(pPacket->length);
				memcpy((char*)pReceive->packet.data, (char*)pPacket->data, pPacket->length);
				pReceive->packet.length = pPacket->length;
				pSktLink->GetSocketID(pReceive->socketId, val.nIndex);
				totalIngoingQueue.WriteUnlock();

				ptr->ingoingQueue.ReadUnlock();
			}
			++it;
		}

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
        Sleep(1);
#else
        usleep(1000);
#endif
    }
    dispatchDone.Resume();
}

} // end namespace ntwk
