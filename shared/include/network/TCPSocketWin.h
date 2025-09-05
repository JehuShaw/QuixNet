/*
 * File:   TCPSocketWin.h
 * Author: Jehu Shaw
 *
 * Created on 2011.10.15
 */
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined ( _WIN64 )

#ifndef TCPSOCKETWIN_H
#define	TCPSOCKETWIN_H

#include <winsock2.h>
#include "NetworkTypes.h"
#include "ReceiveBuffer.h"
#include "SingleProducerConsumer.h"
#include "SpinLock.h"
#include "TCPSocketBase.h"
#include "SpinEvent.h"
#include "XqxTable0S.h"

namespace ntwk
{

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64) && !defined(_AIX) && !defined(HAVE_GETHOSTBYNAME_R) && !defined(__NetBSD__) && !defined(VXWORKS)
#define HAVE_GETHOSTBYNAME_R
#endif

#ifndef MAX_HOST_NAME
#define MAX_HOST_NAME 256
#endif

#ifndef GETHOSTBYNAME_BUF_SIZE
#define GETHOSTBYNAME_BUF_SIZE 1024
#endif

#ifndef MAX_TEST_SEND_TIMES
#define MAX_TEST_SEND_TIMES 6
#endif

#ifndef MORE_RECEIVE_MIN_SIZE
#define MORE_RECEIVE_MIN_SIZE 256
#endif

#define MAX_LISTENER_SIZE 1

#define KEEPALIVE_INTERVAL_MS 5000

#define KEEPALIVE_IDLE_MS 60000

#define MAX_RECEVIE_ZERO_COUNT 10

    class ILinkData {
    public:
		virtual ~ILinkData(){}
        virtual void Clear() = 0;
    };

	typedef enum OperationInfo {
		OP_NULL,
		OP_READ,
		OP_WRITE
	}OPERATIONINFO;

	typedef struct PerIoData {
		WSAOVERLAPPED ol;
		OPERATIONINFO opType;
        thd::CSpinLock spinLock;
	}PER_IO_DATA,* LPPER_IO_DATA;

    typedef class SocketLink {
    public:
		SocketLink(bool bDel = true)
			: fd(0)
		    , binaryAddress(0)
			, port(0)
			, bDelete(bDel)
			, pBuffer(NULL)
			, ptr(NULL)
			, recZeroCount(0)
			, index(XQXTABLE_INDEX_NIL)
			, lostReason(0) {}

		SocketLink(const SocketLink& orig, int idx)
			: fd(orig.fd)
			, binaryAddress(orig.binaryAddress)
			, port(orig.port)
			, bDelete(true)
			, pBuffer(orig.pBuffer)
			, ptr(orig.ptr)
			, recZeroCount(0)
			, index(idx)
			, lostReason(0)
		{
			memset(&(perIoDataRec.ol),0,sizeof(OVERLAPPED));
			perIoDataRec.opType = OP_READ;
			memset(&(perIoDataSen.ol),0,sizeof(OVERLAPPED));
			perIoDataSen.opType = OP_WRITE;
		}

        ~SocketLink(){
			if(bDelete) {
				Clear();
				delete ptr;
				ptr = NULL;
			} else {
				recZeroCount = 0;
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
        friend class TCPSocketWin;

		SOCKET fd;
        /// The peer address from inet_addr.
        unsigned int binaryAddress;
        /// The port number
        unsigned short port;
		/// be delete 
		bool bDelete;
        /// recive overlapped
		PER_IO_DATA perIoDataRec;
        /// send overlapped
		PER_IO_DATA perIoDataSen;
        ///// remove lock
        //volatile bool removeLock;
        /// recive buffer
		ReceiveBuffer* pBuffer;
        /// user data pointer
        ILinkData * ptr;
		/// receive zero count
		int recZeroCount;
		/// socket_links_t key
		volatile long index;
		/// lost reason
		volatile long lostReason;

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
			recZeroCount = 0;
			atomic_xchg(&lostReason, 0);
        }
    } PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

    class SHARED_DLL_DECL TCPSocketWin : public TCPSocketBase {
    public:
        TCPSocketWin();
        virtual ~TCPSocketWin();
        //Start the TCP server on the indicated port
        virtual bool Start(const char* address,unsigned short maxLink, unsigned short threadNum = 0);
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

		// Is exist ?
		virtual bool Exist(const SocketID& socketId) const;

		// Number of socket connections
		virtual uint32_t Size() const;

    protected:
        bool m_isLittleEndian;

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

            return send(socketLink.fd,buffer,length,0);
        }

    private:

        volatile bool m_isListened;

        volatile long m_nWorkerThreadCount;
		thd::CSpinEvent m_receiveDone;
        thd::CSpinEvent m_listenDone;

        //SOCKET m_listenfd;
        SOCKET m_listenFDs[MAX_LISTENER_SIZE];
        HANDLE m_hListenEvent[MAX_LISTENER_SIZE];
        DWORD m_dwListenCount;
		HANDLE m_hIOCP;

        void ClearSocketLink();
        inline bool RemoveSocketLink(const SocketID& socketId) {
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
        bool OnRecvCompletion(SocketLink& socketLink, DWORD dwTrans);
        bool OnSendCompletion(SocketLink& socketLink);

		static bool SendCompletion(SocketLink& socketLink, char* data, long length);

        void ListenRun();
        void UpdateRun();
        friend unsigned int _stdcall ListenTcpInterfaceLoop(void * arguments);
        friend unsigned int _stdcall UpdateTcpInterfaceLoop(void * arguments);

        // Create listen
        bool CreateListen(const char* address, unsigned short maxLink);

    };

    extern unsigned int _stdcall ListenTcpInterfaceLoop(void * arguments);
    extern unsigned int _stdcall UpdateTcpInterfaceLoop(void * arguments);
}
#endif	/* TCPSOCKETWIN_H */

#endif
