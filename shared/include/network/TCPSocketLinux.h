/*
 * File:   TCPSocketLinux.h
 * Author: Jehu Shaw
 *
 * Created on 2011_6_1 8:56
 */

#if defined( __LINUX__ ) || defined( __ANDROID__ ) || defined( ANDROID ) || defined (__GNUC__)

#ifndef TCPSOCKETLINUX_H
#define	TCPSOCKETLINUX_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <map>
#include "NetworkTypes.h"
#include "ReceiveBuffer.h"
#include "SingleProducerConsumer.h"
#include "SpinLock.h"
#include "SpinEvent.h"
#include "TCPSocketBase.h"
#include <stdio.h>
#include "XqxTable0S.h"

namespace ntwk
{

#if !defined(__APPLE__) && !defined(_AIX) && !defined(HAVE_GETHOSTBYNAME_R) && !defined(__NetBSD__) && !defined(VXWORKS)
#define HAVE_GETHOSTBYNAME_R
#endif

#ifndef MAX_HOST_NAME
#define MAX_HOST_NAME 256
#endif

#ifndef GETHOSTBYNAME_BUF_SIZE
#define GETHOSTBYNAME_BUF_SIZE 1024
#endif

#ifndef MAX_TEST_SEND_TIMES
#define MAX_TEST_SEND_TIMES 3
#endif

#define KEEPALIVE_INTERVAL_S 5

#define KEEPALIVE_IDLE_S 60

    class TCPThreadPool;

    class ILinkData {
    public:
        virtual ~ILinkData(){}
        virtual void Clear() = 0;
    };

    class SocketLink {
    public:
        SocketLink()
        : fd(-1)
        , binaryAddress(0)
        , port(0)
        , events(0)
		, recLock(false)
        , senLock(false)
		, pBuffer(NULL)
        , ptr(NULL)
        , removeLock(false) {
        }

        ~SocketLink(){
            Clear();
            delete ptr;
            ptr = NULL;
        }

        inline void GetSocketID(SocketID& outSocketID, int index) {
            outSocketID.index = index;
            outSocketID.binaryAddress = this->binaryAddress;
            outSocketID.port = this->port;
        }

        inline ILinkData* GetLinkerData() {
            return ptr;
        }

   private:
		friend class TCPSocketLinux;

		int fd;
        /// index SocketID instance in array
        volatile int index;
        /// The peer address from inet_addr.
        unsigned int binaryAddress;
        /// The port number
        unsigned short port;
        /// for epoll event.
        volatile int events;
        /// recive overlapped
		volatile bool recLock;
        /// send overlapped
		volatile bool senLock;
        /// recive buffer
		ReceiveBuffer* pBuffer;
        /// user data pointer
        ILinkData * ptr;
        /// remove lock
        volatile bool removeLock;

        SingleProducerConsumer<Packet> outgoingMessages;

        void CreateBuffer(){
            if(NULL != pBuffer) {
                pBuffer->Clear();
            } else {
                pBuffer = new ReceiveBuffer();
            }
        }

        void Clear(){
            delete pBuffer;
            pBuffer = NULL;

            if(ptr != NULL){
                ptr->Clear();
            }
            outgoingMessages.Clear();
            events = 0;
        }
    };

    //
    // Create client socket connected to local or global server socket
    //
    enum socket_domain {
        sock_local_domain, // local domain (i.e. Unix domain socket)
        sock_global_domain // global domain (i.e. INET sockets)
    };

    class TCPSocketLinux : public TCPSocketBase {
    public:
        TCPSocketLinux();
        TCPSocketLinux(socket_domain domain);
        virtual ~TCPSocketLinux();
        //Start the TCP server on the indicated port
        virtual bool Start(const char* address, unsigned short maxLink, unsigned short threadNum = 0);
        //Stop the TCP server
        virtual bool Stop(void);
        //Connect to the specified host on the specified port
        virtual bool Connect(SocketID& socketId, const char* address, unsigned short threadNum = 0);
        // Sends a byte stream
		virtual bool Send(
			unsigned char * data,
			unsigned int length,
			int socketIdx,
			unsigned char* prefixData = NULL,
			unsigned int prefixLength = 0);
        //Disconnects aplayer/address
        virtual void CloseConnection(const SocketID& socketId);

	protected:

		bool m_isLittleEndian;
		unsigned short m_maxLink;
		typedef util::CXqxTable0S<SocketLink> socket_links_t;
		socket_links_t* m_pSktLinks;

		bool m_isStarted;

		virtual ILinkData* AllocateLinkerData() = 0;

		virtual void AcceptCallback(SocketLink& socketLink) = 0;

		virtual int ReceiveCallback(
			SocketLink& socketLink,
			const unsigned char* buffer,
			const unsigned int length,
			const unsigned char* moreBuffer,
			const unsigned int moreLength) = 0;

		virtual void SendCallback(
			Packet& dstPacket,
			const unsigned char* data,
			const unsigned int length,
			const unsigned char* prefixData,
			const unsigned int prefixLength) = 0;

		virtual void DisconnectCallback(SocketLink& socketLink) = 0;

		virtual void MisdataCallback(
			SocketLink& socketLink,
			const unsigned char* buffer,
			const unsigned int length) = 0;

		inline int RawSend(
			const SocketLink& socketLink,
			const char* buffer,
			const unsigned int length) {

			return send(socketLink.fd, buffer, length, 0);
		}

	private:
        //
        // Directory for Unix Domain socket files. This directory should be
        // either empty or be terminated with "/". Dafault value is "/tmp/"
        //
        static char* unix_socket_dir;
        socket_domain m_domain;      // Unix domain or INET socket
        bool m_isListened;

        thd::CSpinEvent m_listenDone;

        TCPThreadPool* m_pThreadPool;

        int m_epfd;
        int m_listenfd;
        struct epoll_event * m_pEvents;
		volatile long m_eventNum;

        void ClearSocketLink();
		inline bool RemoveSocketLink(const SocketID& socketId){
			if(NULL == m_pSktLinks) {
				return false;
			}
			socket_links_t::value_t val(m_pSktLinks->Find(socketId.index));
			if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
				return false;
			}
			if(NULL == val.pObject) {
				assert(val.pObject);
				return false;
			}
			return RemoveSocketLink(*val.pObject, val.nIndex);
		}
        bool RemoveSocketLink(SocketLink& socketLink, size_t index);
		bool OnRecvCompletion(SocketLink& socketLink);
		bool OnSendCompletion(SocketLink& socketLink);

        void ListenRun();
        static int LeaderFunc(void *argv);
        static int LaskFunc(void *argv);

        friend void * ListenTcpInterfaceLoop(void * arguments);

    private:
        bool SetNonblocking(int sock);

		inline struct epoll_event* PopEvent() {
			int index = atomic_dec(&m_eventNum);

			if (index < 0) {
				return NULL;
			} else {
				return &m_pEvents[index];
			}
		}

	};

    extern void * ListenTcpInterfaceLoop(void * arguments);
}
#endif	/* INETSOKETLINUX_H */

#endif
