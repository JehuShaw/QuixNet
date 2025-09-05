/*
 * File:   TCPLinkInterface.cpp
 * Author: Jehu Shaw
 *
 * Created on 2011_9_13 14:50
 */

#include "TCPLinkInterface.h"
#include "NetworkTypes.h"
#include "NetworkTrace.h"
#include <algorithm>
#include <string.h>

namespace ntwk {

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined ( _WIN64 )
TCPLinkInterface::TCPLinkInterface()
	:TCPSocketWin(), m_pLinkEvent(NULL), packetSizeLimit(0) {

}
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
TCPLinkInterface::TCPLinkInterface()
	:TCPSocketBSD(sock_global_domain), m_pLinkEvent(NULL), packetSizeLimit(0) {

}

TCPLinkInterface::TCPLinkInterface(socket_domain domain)
	:TCPSocketBSD(domain), m_pLinkEvent(NULL), packetSizeLimit(0) {

}
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
TCPLinkInterface::TCPLinkInterface()
	:TCPSocketLinux(sock_global_domain), m_pLinkEvent(NULL), packetSizeLimit(0) {

}

TCPLinkInterface::TCPLinkInterface(socket_domain domain)
	:TCPSocketLinux(domain), m_pLinkEvent(NULL), packetSizeLimit(0) {

}
#endif

TCPLinkInterface::~TCPLinkInterface() {
}

int TCPLinkInterface::ReceiveCallback(
	SocketLink& socketLink,
    const unsigned char* buffer,
    const unsigned int length,
    const unsigned char* moreBuffer,
    const unsigned int moreLength)
{
    //split packet
    uint32_t offset(0);
    int32_t leave(0);
    while(true) {
        leave = length + moreLength - offset;
        if(leave < 8) {
			break;
		}
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
                ,length,moreBuffer,moreLength,offset);
            // read len uint32
            SwitchCopyBytes((unsigned char*)&len,sizeof(len),buffer,length
                ,moreBuffer,moreLength,offset+sizeof(total));
        }
        if(len > packetSizeLimit) {
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
        LinkData* ptr = dynamic_cast<LinkData*>(socketLink.GetLinkerData());
        if(ptr != 0){
            Packet * p = ptr->ingoingQueue.WriteLock();
            p->length = len;
            SwitchMemcpy(p->data,buffer,length,moreBuffer,moreLength,len,offset);
            ptr->ingoingQueue.WriteUnlock();

			if(NULL != m_pLinkEvent) {
				m_pLinkEvent->OnReceive();
			}
        }
        offset += len;
        Sleep(1);
    }
    return offset;
}

void TCPLinkInterface::SendCallback(
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

ILinkData* TCPLinkInterface::AllocateLinkerData() {
    return new LinkData();
}

void TCPLinkInterface::AcceptCallback(const SocketLink& socketLink) {

    SocketID *temp = newConnections.WriteLock();
    socketLink.GetSocketID(*temp);
    newConnections.WriteUnlock();

	if(NULL != m_pLinkEvent) {
		m_pLinkEvent->OnAccept();
	}
}

void TCPLinkInterface::DisconnectCallback(int nIndex, const SocketID& socketId, int nWhy) {

	LostData *temp = lostConnections.WriteLock();
    temp->socketId = socketId;
	temp->nWhy = nWhy;
    lostConnections.WriteUnlock();

	if(NULL != m_pLinkEvent) {
		m_pLinkEvent->OnDisconnect();
	}
}

void TCPLinkInterface::MisdataCallback(
	SocketLink& socketLink,
    const unsigned char* buffer,
    const unsigned int length) {

    if((unsigned char*)0 == buffer) {
        TRACE_MSG("@MisdataCallback: (unsigned char*)0 == buffer \n");
        return;
    }
    if(strncmp((char*)buffer,"<policy-file-request/>",length) == 0){
        const char* policyFile = "<?xml version=\"1.0\"?><cross-domain-policy>"
            "<allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>\0";
        int nLength = strlen(policyFile);
        RawSend(socketLink, policyFile, nLength);
    } else {
        TRACE_MSG("The packet no right.\n");
    }
}

} // end namespace ntwk
