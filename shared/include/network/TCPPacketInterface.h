/*
 * File:   TCPPacketInterface.h
 * Author: Jehu Shaw
 *
 * Created on 2011_9_14 14:50
 */

#ifndef TCPPACKETINTERFACE_H
#define	TCPPACKETINTERFACE_H

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#include "TCPSocketWin32.h"
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
#include "TCPSocketBSD.h"
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
#include "TCPSocketLinux.h"
#endif

#include <set>

namespace  ntwk {

    struct ReceivePacket {
        Packet packet;
        SocketID socketId;
    };

    typedef std::set<int> REGISTER_LIST_T;

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    class SHARED_DLL_DECL TCPPacketInterface :protected TCPSocketWin32 {
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
    class SHARED_DLL_DECL TCPPacketInterface :protected TCPSocketBSD {
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    class SHARED_DLL_DECL TCPPacketInterface :protected TCPSocketLinux {
#endif
    public:
        TCPPacketInterface();
#if !defined( __WIN32__ ) && !defined( WIN32 ) && !defined( _WIN32 )
        TCPPacketInterface(socket_domain domain);
#endif
        virtual ~TCPPacketInterface();

        //Start the TCP server on the indicated port
        bool Start(const char* address, unsigned short maxLink,
            uint32_t maxPacketSize = MAX_PACKET_SIZE);

        //Stop the TCP server
        bool Stop(void);

        //Connect to the specified host on the specified port
        bool Connect(SocketID& socketId, const char* address,
            uint32_t maxPacketSize = MAX_PACKET_SIZE);

        // Sends a byte stream
        inline bool Send(unsigned char * data,unsigned int length,const SocketID& socketId) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
            return this->TCPSocketWin32::Send(data,length,socketId.index);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Send(data,length,socketId.index);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Send(data,length,socketId.index);
#endif
        }

		inline bool Send(unsigned char * data,unsigned int length,int socketIdx) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
			return this->TCPSocketWin32::Send(data,length,socketIdx);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Send(data,length,socketIdx);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
			return this->TCPSocketLinux::Send(data,length,socketIdx);
#endif
		}

        //returns data received
        inline ReceivePacket* Receive() {
            return totalIngoingQueue.ReadLock();
        }
        //Deallocates a packet returned by Receive
        inline void DeallocatePacket(const ReceivePacket* packet) {
            if(NULL != packet && totalIngoingQueue.CheckReadUnlockOrder(packet)){
                totalIngoingQueue.ReadUnlock();
            }
        }
		// returns received size
		inline int ReceiveSize() {
			return totalIngoingQueue.Size();
		}
        //check new connections
		inline bool HasNewConnection(SocketID& socketId){
			SocketID *temp = newConnections.ReadLock();
			if (temp){
				socketId = *temp;
				newConnections.ReadUnlock();
				return true;
			}
			return false;
		}
        //check lost connections
		inline bool HasLostConnection(SocketID& socketId){
			SocketID *temp = lostConnections.ReadLock();
			if (temp){
				socketId = *temp;
				lostConnections.ReadUnlock();
				return true;
			}
			return false;
		}
        //Disconnects aplayer/address
		inline void CloseConnection(const SocketID& socketId){
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
			this->TCPSocketWin32::CloseConnection(socketId);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            this->TCPSocketBSD::CloseConnection(socketId);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
			this->TCPSocketLinux::CloseConnection(socketId);
#endif
		}

		inline int GetLinkCount() const {
			return (int)registerList.size();
		}

    protected:
        SingleProducerConsumer<SocketID> newConnections,lostConnections;
        SingleProducerConsumer<ReceivePacket> totalIngoingQueue;
        SingleProducerConsumer<int> loginQueue;
        SingleProducerConsumer<int> logoutQueue;

        REGISTER_LIST_T registerList;

        volatile bool bRegistered;
        ILinkData* AllocateLinkerData();
        void AcceptCallback(SocketLink& socketLink);
        int ReceiveCallback(
			SocketLink& socketLink,
            const unsigned char* buffer,
            const unsigned int length,
            const unsigned char* moreBuffer,
            const unsigned int moreLength);
		void SendCallback(
			Packet& dstPacket,
			const unsigned char* data,
			const unsigned int length,
			const unsigned char* prefixData,
			const unsigned int prefixLength);
        void DisconnectCallback(SocketLink& socketLink);
        void MisdataCallback(
			SocketLink& socketLink,
            const unsigned char* buffer,
            const unsigned int length);

private:
        void DispatchRun();

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
        friend unsigned int _stdcall DispatchInterfaceLoop(void * arguments);
#else
        friend void * DispatchInterfaceLoop(void * arguments);
#endif


        volatile bool isDispatched;
        thd::CSpinEvent dispatchDone;
        // Limit the packet size, The unit is byte
        uint32_t packetSizeLimit;
    };

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    extern unsigned int _stdcall DispatchInterfaceLoop(void * arguments);
#else
    extern void * DispatchInterfaceLoop(void * arguments);
#endif
}
#endif	/* TCPPACKETINTERFACE_H */


