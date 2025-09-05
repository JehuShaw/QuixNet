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
        SocketLink(bool bDel = true)
        : bDelete(bDel)
		, pBuffer(NULL)
        , ptr(NULL)
		, events(0)
		, index(XQXTABLE_INDEX_NIL)
		, lostReason(0) {}

		SocketLink(const SocketLink& orig, int idx)
			: fd(orig.fd)
			, binaryAddress(orig.binaryAddress)
			, port(orig.port)
			, bDelete(true)
			, events(0)
			, recLock(orig.recLock)
			, senLock(orig.senLock)
			, pBuffer(orig.pBuffer)
			, ptr(orig.ptr)
			, removeLock(orig.removeLock)
			, index(idx)
			, lostReason(0)
		{}

        ~SocketLink(){
			if (bDelete) {
				Clear();
				delete ptr;
				ptr = NULL;
			} else {
				atomic_xchg(&events, 0);
			}
        }

        inline void GetSocketID(SocketID& outSocketID) const {
			outSocketID.fd = this->fd;
            outSocketID.index = this->index;
            outSocketID.binaryAddress = this->binaryAddress;
            outSocketID.port = this->port;
        }

		inline bool IsSocketID(const SocketID& socketID) const {
			return socketID.index == this->index && socketID.fd == this->fd &&
				socketID.binaryAddress == this->binaryAddress && socketID.port == this->port;
		}

        inline ILinkData* GetLinkerData() const {
            return ptr;
        }

		inline int GetIndex() const {
			return this->index;
		}

		inline int GetLostReason() const {
			return lostReason;
		}

   private:
		friend class TCPSocketLinux;

		int fd;
        /// socket_links_t key
        volatile int index;
        /// The peer address from inet_addr.
        unsigned int binaryAddress;
        /// The port number
        unsigned short port;
		/// be delete 
		bool bDelete;
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
		/// lost reason
		volatile int lostReason;

        SingleProducerConsumer<Packet> outgoingMessages;

        void CreateBuffer(){
            if(NULL != pBuffer) {
                pBuffer->Clear();
            } else {
                pBuffer = new ReceiveBuffer();
            }
        }

        void Clear(){
			atomic_xchg(&index, XQXTABLE_INDEX_NIL);
            delete pBuffer;
            pBuffer = NULL;

            if(ptr != NULL){
                ptr->Clear();
            }
            outgoingMessages.Clear();
			atomic_xchg(&events, 0);
			atomic_xchg(&lostReason, 0);
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
			const SocketID& socketId,
			unsigned char* prefixData = NULL,
			unsigned int prefixLength = 0);
        //Disconnects aplayer/address
        virtual void CloseConnection(const SocketID& socketId, int nWhy);
		//Is exist ?
		virtual bool Exist(const SocketID& socketId) const;
		// Number of socket connections
		virtual uint32_t Size() const;

	protected:
		bool m_isLittleEndian;
		unsigned short m_maxLink;
		typedef util::CXqxTable0S<SocketLink> socket_links_t;
		socket_links_t* m_pSktLinks;

		volatile bool m_isStarted;

		virtual ILinkData* AllocateLinkerData() = 0;

		virtual void AcceptCallback(const SocketLink& socketLink) = 0;

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

		virtual void DisconnectCallback(
			int nIndex,
			const SocketID& socketId,
			int nWhy) = 0;

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
        volatile bool m_isListened;

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
			SocketLink* pSktLink = m_pSktLinks->Find(socketId.index);
			if(NULL == pSktLink || !pSktLink->IsSocketID(socketId)) {
				return false;
			}
			return RemoveSocketLink(*pSktLink);
		}
        bool RemoveSocketLink(SocketLink& socketLink);
		bool OnRecvCompletion(SocketLink& socketLink);
		bool OnSendCompletion(SocketLink& socketLink);

		inline static bool TaskRemoveSocket(TCPSocketLinux * mgr, SocketLink& socketLink)
		{
			bool bRet = true;
			if(atomic_cmpxchg8(&socketLink.removeLock, false, true) == false) {
				bRet = mgr->RemoveSocketLink(socketLink);
				atomic_xchg8(&socketLink.removeLock, false);
			}
			return bRet;
		}

		inline static bool TaskRecvCompletion(TCPSocketLinux * mgr, SocketLink& socketLink)
		{
			bool bRet = true;
			if(atomic_cmpxchg8(&socketLink.recLock, false, true) == false) {
				bRet = mgr->OnRecvCompletion(socketLink);
				atomic_xchg8(&socketLink.recLock, false);
			}
			return bRet;
		}

		inline static bool TaskSendCompletion(TCPSocketLinux * mgr, SocketLink& socketLink)
		{
			bool bRet = true;
			if(atomic_cmpxchg8(&socketLink.senLock, false, true) == false) {
				bRet = mgr->OnSendCompletion(socketLink);
				atomic_xchg8(&socketLink.senLock, false);
			}
			return bRet;
		}

		inline static bool SendCompletion(int fd, char* data, ssize_t length)
		{
			ssize_t result = 0;
			while (length > 0) {
				result = send(fd, data, length, MSG_NOSIGNAL);
				if (result > 0) {
					data += result;
					length -= result;
				} else if(EAGAIN == errno
					// nonblocking socket no EINTR error.
					//|| EINTR == errno
					|| EWOULDBLOCK == errno) {
					continue;
				} else {
					return false;
				}
			}
			return true;
		}

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
