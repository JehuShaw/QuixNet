/*
 * File:   TCPLinkInterface.h
 * Author: Jehu Shaw
 *
 * Created on 2011_9_13 14:50
 */

#ifndef TCPLINKINTERFACE_H
#define	TCPLINKINTERFACE_H

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
#include "TCPSocketWin.h"
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
#include "TCPSocketBSD.h"
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
#include "TCPSocketLinux.h"
#endif

namespace ntwk {

	class ILinkEvent {
	public:
		virtual void OnAccept() = 0;
		virtual void OnDisconnect() = 0;
		virtual void OnReceive() = 0;
	};

	class LinkData : public ILinkData {
	public:

		void Clear(){
			ingoingQueue.Clear();
		}
		SingleProducerConsumer<Packet> ingoingQueue;
	};

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
    class SHARED_DLL_DECL TCPLinkInterface : protected TCPSocketWin {
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
    class SHARED_DLL_DECL TCPLinkInterface : protected TCPSocketBSD {
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    class SHARED_DLL_DECL TCPLinkInterface : protected TCPSocketLinux {
#endif
    public:
        TCPLinkInterface();
#if !defined( __WIN32__ ) && !defined( WIN32 ) && !defined( _WIN32 ) && !defined( _WIN64 )
        TCPLinkInterface(socket_domain domain);
#endif
        virtual ~TCPLinkInterface();

        //Start the TCP server on the indicated port
        inline bool Start(const char* address, unsigned short maxLink,
            uint32_t maxPacketSize = MAX_PACKET_SIZE)
        {
            this->packetSizeLimit = maxPacketSize;
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
            return this->TCPSocketWin::Start(address, maxLink);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Start(address, maxLink);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Start(address, maxLink);
#endif
        }

        //Stop the TCP server
        inline bool Stop(void) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
            if(this->TCPSocketWin::Stop()){
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            if (this->TCPSocketBSD::Stop()) {
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            if(this->TCPSocketLinux::Stop()){
#endif
                newConnections.Clear();
                lostConnections.Clear();
                return true;
            }
            return false;
        }

        //Connect to the specified host on the specified port
        inline bool Connect(SocketID& socketId, const char* address,
            uint32_t maxPacketSize = MAX_PACKET_SIZE)
        {
            this->packetSizeLimit = maxPacketSize;
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
            return this->TCPSocketWin::Connect(socketId, address);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Connect(socketId, address);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Connect(socketId, address);
#endif
        }

        // Sends a byte stream
        inline bool Send(unsigned char * data, unsigned int length, const SocketID& socketId) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
            return this->TCPSocketWin::Send(data, length, socketId);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Send(data, length, socketId);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Send(data, length, socketId);
#endif
        }

		// Is exist ?
		inline bool Exist(const SocketID& socketId) const {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			return this->TCPSocketWin::Exist(socketId);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
			return this->TCPSocketBSD::Exist(socketId);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
			return this->TCPSocketLinux::Exist(socketId);
#endif
		}

		// Number of socket connections
		inline uint32_t Size() const {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			return this->TCPSocketWin::Size();
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
			return this->TCPSocketBSD::Size();
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
			return this->TCPSocketLinux::Size();
#endif
		}

        //returns data received
		inline LinkData* Receive(const SocketID& socketId) {

			if(!m_isStarted) {
				return NULL;
			}

			if(NULL == m_pSktLinks) {
				return NULL;
			}

			SocketLink* pSktLink = m_pSktLinks->Find(socketId.index);
			if(NULL == pSktLink || !pSktLink->IsSocketID(socketId)) {
				return NULL;
			}

			return dynamic_cast<LinkData*>(pSktLink->GetLinkerData());
		}

		inline Packet* AllocatePacket(LinkData* ptr) {
			if (NULL != ptr) {
				return ptr->ingoingQueue.ReadLock();
			}
			return NULL;
		}

        //Deallocates a packet returned by Receive
		inline bool DeallocatePacket(LinkData* ptr, Packet* packet) {
			if(NULL != ptr) {
				assert(ptr->ingoingQueue.CheckReadUnlockOrder(packet));
				ptr->ingoingQueue.ReadUnlock();
				return true;
			}
			return false;
		}

		// returns received size
		inline int ReceiveSize(const SocketID& socketId) {

			if(!m_isStarted) {
				return 0;
			}

			if(NULL == m_pSktLinks) {
				return 0;
			}

			SocketLink* pSktLink = m_pSktLinks->Find(socketId.index);
			if(NULL == pSktLink || !pSktLink->IsSocketID(socketId)) {
				return 0;
			}

			LinkData* ptr = dynamic_cast<LinkData*>(pSktLink->GetLinkerData());
			if(NULL != ptr) {
				return ptr->ingoingQueue.Size();
			}
			return 0;
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
		inline bool HasLostConnection(SocketID& socketId, int& nWhy){
			LostData *temp = lostConnections.ReadLock();
			if (temp){
				socketId = temp->socketId;
				nWhy = temp->nWhy;
				lostConnections.ReadUnlock();
				return true;
			}
			return false;
		}

        //Disconnects aplayer/address
		inline void CloseConnection(const SocketID& socketId, int nWhy){
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			this->TCPSocketWin::CloseConnection(socketId, nWhy);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            this->TCPSocketBSD::CloseConnection(socketId, nWhy);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
			this->TCPSocketLinux::CloseConnection(socketId, nWhy);
#endif
		}

		inline void SetLinkEvent(ILinkEvent* pLinkEvent) {
			m_pLinkEvent = pLinkEvent;
		}

    protected:
        SingleProducerConsumer<SocketID> newConnections;
		SingleProducerConsumer<LostData> lostConnections;
		ILinkEvent* m_pLinkEvent;

		// Limit the packet size, The unit is byte
		uint32_t packetSizeLimit;

        ILinkData* AllocateLinkerData();

        void AcceptCallback(const SocketLink& socketLink);

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

        void DisconnectCallback(
			int nIndex,
			const SocketID& socketId,
			int nWhy);

        void MisdataCallback(
			SocketLink& socketLink,
            const unsigned char* buffer,
            const unsigned int length);
    };
}
#endif	/* TCPLINKINTERFACE_H */


