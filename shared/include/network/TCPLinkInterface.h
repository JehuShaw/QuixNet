/*
 * File:   TCPLinkInterface.h
 * Author: Jehu Shaw
 *
 * Created on 2011_9_13 14:50
 */

#ifndef TCPLINKINTERFACE_H
#define	TCPLINKINTERFACE_H

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#include "TCPSocketWin32.h"
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

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
    class SHARED_DLL_DECL TCPLinkInterface : protected TCPSocketWin32 {
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
    class SHARED_DLL_DECL TCPLinkInterface : protected TCPSocketBSD {
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    class SHARED_DLL_DECL TCPLinkInterface : protected TCPSocketLinux {
#endif
    public:
        TCPLinkInterface();
#if !defined( __WIN32__ ) && !defined( WIN32 ) && !defined( _WIN32 )
        TCPLinkInterface(socket_domain domain);
#endif
        virtual ~TCPLinkInterface();

        //Start the TCP server on the indicated port
        inline bool Start(const char* address, unsigned short maxLink,
            uint32_t maxPacketSize = MAX_PACKET_SIZE)
        {
            this->packetSizeLimit = maxPacketSize;
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
            return this->TCPSocketWin32::Start(address, maxLink);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Start(address, maxLink);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Start(address, maxLink);
#endif
        }

        //Stop the TCP server
        inline bool Stop(void) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
            if(this->TCPSocketWin32::Stop()){
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
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
            return this->TCPSocketWin32::Connect(socketId, address);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Connect(socketId, address);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Connect(socketId, address);
#endif
        }

        // Sends a byte stream
        inline bool Send(unsigned char * data, unsigned int length, const SocketID& socketId) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
            return this->TCPSocketWin32::Send(data, length, socketId.index);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Send(data, length, socketId.index);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Send(data, length, socketId.index);
#endif
        }

		inline bool Send(unsigned char * data, unsigned int length, int socketIdx) {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
			return this->TCPSocketWin32::Send(data, length, socketIdx);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Send(data, length, socketIdx);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
			return this->TCPSocketLinux::Send(data, length, socketIdx);
#endif
		}

        //returns data received
		inline Packet* Receive(const SocketID& socketId) {
			return Receive(socketId.index);
		}
		inline Packet* Receive(int socketIdx) {

			if(!m_isStarted) {
				return NULL;
			}

			if(NULL == m_pSktLinks) {
				return NULL;
			}

			socket_links_t::value_t val(m_pSktLinks->Find(socketIdx));
			if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
				return NULL;
			}

			SocketLink* pSktLink = val.pObject;
			if(NULL == pSktLink) {
				assert(pSktLink);
				return NULL;
			}

			LinkData* ptr = dynamic_cast<LinkData*>(pSktLink->GetLinkerData());
			if(NULL != ptr){
				return ptr->ingoingQueue.ReadLock();
			}
			return NULL;
		}

        //Deallocates a packet returned by Receive
		inline void DeallocatePacket(Packet* packet, const SocketID& socketId) {
			DeallocatePacket(packet, socketId.index);
		}
		inline void DeallocatePacket(Packet* packet, int socketIdx) {

			if(!m_isStarted) {
				return;
			}

			if(NULL == m_pSktLinks) {
				return;
			}

			socket_links_t::value_t val(m_pSktLinks->Find(socketIdx));
			if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
				return;
			}

			SocketLink* pSktLink = val.pObject;
			if(NULL == pSktLink) {
				assert(pSktLink);
				return;
			}

			LinkData* ptr = dynamic_cast<LinkData*>(pSktLink->GetLinkerData());
			if(NULL != ptr){
				assert(ptr->ingoingQueue.CheckReadUnlockOrder(packet));
				ptr->ingoingQueue.ReadUnlock();
			}
		}

		// returns received size
		inline int ReceiveSize(const SocketID& socketId) {
			return ReceiveSize(socketId.index);
		}
		inline int ReceiveSize(int socketIdx) {

			if(!m_isStarted) {
				return 0;
			}

			if(NULL == m_pSktLinks) {
				return 0;
			}

			socket_links_t::value_t val(m_pSktLinks->Find(socketIdx));
			if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
				return 0;
			}

			SocketLink* pSktLink = val.pObject;
			if(NULL == pSktLink) {
				assert(pSktLink);
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

		inline void SetLinkEvent(ILinkEvent* pLinkEvent) {
			m_pLinkEvent = pLinkEvent;
		}

    protected:
        SingleProducerConsumer<SocketID> newConnections,lostConnections;
		ILinkEvent* m_pLinkEvent;

		// Limit the packet size, The unit is byte
		uint32_t packetSizeLimit;

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
    };
}
#endif	/* TCPLINKINTERFACE_H */


