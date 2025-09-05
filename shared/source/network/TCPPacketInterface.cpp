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

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )
#include <process.h>
#endif

using namespace thd;

namespace ntwk
{
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )
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


#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )
TCPPacketInterface::TCPPacketInterface():TCPSocketWin(), isDispatched_(true), packetSizeLimit_(0), linkCount_(0) {

}
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
TCPPacketInterface::TCPPacketInterface():TCPSocketBSD(sock_global_domain), isDispatched_(true), packetSizeLimit_(0), linkCount_(0) {

}

TCPPacketInterface::TCPPacketInterface(socket_domain domain):TCPSocketBSD(domain), isDispatched_(true), packetSizeLimit_(0), linkCount_(0) {

}
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
TCPPacketInterface::TCPPacketInterface():TCPSocketLinux(sock_global_domain), isDispatched_(true), packetSizeLimit_(0), linkCount_(0) {

}

TCPPacketInterface::TCPPacketInterface(socket_domain domain):TCPSocketLinux(domain), isDispatched_(true), packetSizeLimit_(0), linkCount_(0) {

}
#endif

TCPPacketInterface::~TCPPacketInterface() {
}

bool TCPPacketInterface::Start(const char* address,unsigned short maxLink, uint32_t maxPacketSize) {
    this->packetSizeLimit_ = maxPacketSize;

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )
    bool ret = this->TCPSocketWin::Start(address,maxLink);
    if(!ret) {
        return false;
    }

    HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, DispatchInterfaceLoop, (LPVOID)this, 0, NULL);
    if(threadHandle == NULL) {
        TRACE_MSG("Dispatch, _beginthreadex failed\n");
        this->TCPSocketWin::Stop();
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

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&tid, &attr, DispatchInterfaceLoop, this) != 0){
        TRACE_MSG("Dispatch, pthread_create failed\n");
#if defined( __NetBSD__ ) || defined( __APPLE__ )
		this->TCPSocketBSD::Stop();
#else
        this->TCPSocketLinux::Stop();
#endif
        return false;
    }
#endif

	threadCtrler_.Setup();
    return ret;
}

bool TCPPacketInterface::Stop(void) {

    if(atomic_cmpxchg8(&isDispatched_
        , true, false) != (char)true) {

            return false;
    }

	threadCtrler_.Resume();
	threadDone_.Wait();

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )
    if(this->TCPSocketWin::Stop()){
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
    this->packetSizeLimit_ = maxPacketSize;

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )

    if(!this->TCPSocketWin::Connect(socketId, address)) {
        return false;
    }

    HANDLE threadHandle = (HANDLE)_beginthreadex(NULL, 0, DispatchInterfaceLoop, (LPVOID)this, 0, NULL);
    if(threadHandle == NULL) {
        TRACE_MSG("Dispatch, _beginthreadex failed\n");
        this->TCPSocketWin::Stop();
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

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&tid, &attr, DispatchInterfaceLoop, this) != 0){
        TRACE_MSG("Dispatch, pthread_create failed\n");
#if defined( __NetBSD__ ) || defined( __APPLE__ )
		this->TCPSocketBSD::Stop();
#else
        this->TCPSocketLinux::Stop();
#endif
        return false;
    }
#endif

	threadCtrler_.Setup();

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
    uint32_t offset = 0;
    int32_t leave = 0;
	bool bTrigger = false;
    while(true) {
        leave = length + moreLength - offset;
        if(leave < 8) {
			break;
		}
        uint32_t len(0),total(0);
        if(m_isLittleEndian) {
            // read total uint32
            SwitchReverseBytes((unsigned char*)&total,sizeof(total),buffer
                ,length,moreBuffer,moreLength,offset);
            // read len uint32
            SwitchReverseBytes((unsigned char*)&len,sizeof(len),buffer,length
                ,moreBuffer,moreLength,offset+sizeof(total));
        } else {
            // read total uint32
            SwitchCopyBytes((unsigned char*)&total,sizeof(total),buffer
                ,length, moreBuffer, moreLength,offset);
            // read len uint32
            SwitchCopyBytes((unsigned char*)&len,sizeof(len),buffer,length
                ,moreBuffer,moreLength,offset+sizeof(total));
        }
        if(len > packetSizeLimit_) {
            // Limit the size of packet
            return -1;
        }
		if((len + 4) != total) {
			return -1;
		}
        if((int)(4 + total) > leave){
			break;
		}
        offset += 8;
        //push the data
        PacketLinkData* ptr = dynamic_cast<PacketLinkData*>(socketLink.GetLinkerData());
        if(ptr != NULL){
            Packet * p = ptr->ingoingQueue.WriteLock();
            p->length = len;
            SwitchMemcpy(p->data,buffer,length,moreBuffer,moreLength,len,offset);
            ptr->ingoingQueue.WriteUnlock();
			bTrigger = true;
        }
        offset += len;
		Sleep(1);
    }

	if(bTrigger) {
		threadCtrler_.Resume();
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

void TCPPacketInterface::AcceptCallback(const SocketLink& socketLink) {

    SocketID *temp = newConnections.WriteLock();
    socketLink.GetSocketID(*temp);
    newConnections.WriteUnlock();

    int* pInt = loginQueue.WriteLock();
    *pInt = socketLink.GetIndex();
    loginQueue.WriteUnlock();

	atomic_inc(&linkCount_);

	threadCtrler_.Resume();
}

void TCPPacketInterface::DisconnectCallback(int nIndex, const SocketID& socketId, int nWhy) {

	atomic_dec(&linkCount_);

    LostData *temp = lostConnections.WriteLock();
    temp->socketId = socketId;
	temp->nWhy = nWhy;
    lostConnections.WriteUnlock();

    int* pInt = logoutQueue.WriteLock();
    *pInt = nIndex;
    logoutQueue.WriteUnlock();

	threadCtrler_.Resume();
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
	threadDone_.Suspend();

	REGISTER_LIST_T registerList;
    while(isDispatched_) {

		int nLgoSize = logoutQueue.Size();
		for (int i = 0; i < nLgoSize; ++i) {
			int* pIntLogout = logoutQueue.ReadLock();
			if (NULL == pIntLogout || !isDispatched_) {
				break;
			}
			int nIndex = *pIntLogout;
			logoutQueue.ReadUnlock();
			registerList.erase(nIndex);
		}

		int nLgnSize = loginQueue.Size();
		for(int i = 0; i < nLgnSize; ++i) {
			int* pIntLogin = loginQueue.ReadLock();
			if(NULL == pIntLogin || !isDispatched_) {
				break;
			}
			int nIndex = *pIntLogin;
			loginQueue.ReadUnlock();
			if(XQXTABLE_INDEX_NIL != nIndex){
				registerList.insert(nIndex);
			} else {
				assert(false);
			}
		}

		REGISTER_LIST_T::iterator it = registerList.begin();
		while(registerList.end() != it) {
			if(!isDispatched_) {
				break;
			}
			if(!m_isStarted) {
				++it;
				continue;
			}
			if(NULL == m_pSktLinks) {
				++it;
				continue;
			}

			SocketLink* pSktLink = m_pSktLinks->Find(*it);
			if(NULL == pSktLink) {
				++it;
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
				pSktLink->GetSocketID(pReceive->socketId);
				totalIngoingQueue.WriteUnlock();

				ptr->ingoingQueue.ReadUnlock();
			}
			++it;
		}

		threadCtrler_.Suspend();
    }

	threadDone_.Resume();
}

} // end namespace ntwk
