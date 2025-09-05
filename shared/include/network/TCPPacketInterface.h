/*
 * File:   TCPPacketInterface.h
 * Author: Jehu Shaw
 *
 * Created on 2011_9_14 14:50
 */

#ifndef TCPPACKETINTERFACE_H
#define	TCPPACKETINTERFACE_H

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
#include "TCPSocketWin.h"
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
#include "TCPSocketBSD.h"
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
#include "TCPSocketLinux.h"
#endif

#include <set>

namespace  ntwk {

	class CThreadCtrler
	{
	public:
		CThreadCtrler()
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			: m_hSema(NULL)
#endif
		{}

		~CThreadCtrler() {
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
#else
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&cond);
#endif
		}

		void Setup()
		{
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			m_hSema = CreateSemaphore(NULL, 0, 2147483647, NULL);
#else
			pthread_mutex_init(&mutex, NULL);
			pthread_cond_init(&cond, NULL);
#endif
		}

		void Suspend()
		{
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			WaitForSingleObject(m_hSema, INFINITE);
#else
			pthread_cond_wait(&cond, &mutex);
#endif
		}

		bool Resume()
		{
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			return ReleaseSemaphore(m_hSema, 1, NULL) != FALSE;
#else
			return pthread_cond_signal(&cond) == 0;
#endif
		}

	private:
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
		HANDLE m_hSema;
#else
		pthread_cond_t cond;
		pthread_mutex_t mutex;
#endif
	};

    struct ReceivePacket {
        Packet packet;
        SocketID socketId;
    };

    typedef std::set<int> REGISTER_LIST_T;

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
    class SHARED_DLL_DECL TCPPacketInterface :protected TCPSocketWin {
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
    class SHARED_DLL_DECL TCPPacketInterface :protected TCPSocketBSD {
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
    class SHARED_DLL_DECL TCPPacketInterface :protected TCPSocketLinux {
#endif
    public:
        TCPPacketInterface();
#if !defined( __WIN32__ ) && !defined( WIN32 ) && !defined( _WIN32 ) && !defined( _WIN64 )
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
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
            return this->TCPSocketWin::Send(data,length,socketId);
#elif defined( __NetBSD__ ) || defined( __APPLE__ )
            return this->TCPSocketBSD::Send(data,length,socketId);
#elif defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)
            return this->TCPSocketLinux::Send(data,length,socketId);
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
		inline bool HasLostConnection(SocketID& socketId, int& nWhy){
			LostData* temp = lostConnections.ReadLock();
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

		inline int GetLinkCount() const {
			return (int)linkCount_;
		}

    protected:
		SingleProducerConsumer<SocketID> newConnections;
		SingleProducerConsumer<LostData> lostConnections;
        SingleProducerConsumer<ReceivePacket> totalIngoingQueue;
        SingleProducerConsumer<int> loginQueue;
        SingleProducerConsumer<int> logoutQueue;


        volatile bool bRegistered;
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

private:
        void DispatchRun();

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
        friend unsigned int _stdcall DispatchInterfaceLoop(void * arguments);
#else
        friend void * DispatchInterfaceLoop(void * arguments);
#endif


        volatile bool isDispatched_;
		volatile long linkCount_;
		CThreadCtrler threadCtrler_;
		thd::CSpinEvent threadDone_;
        // Limit the packet size, The unit is byte
        uint32_t packetSizeLimit_;
    };

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
    extern unsigned int _stdcall DispatchInterfaceLoop(void * arguments);
#else
    extern void * DispatchInterfaceLoop(void * arguments);
#endif
}
#endif	/* TCPPACKETINTERFACE_H */


