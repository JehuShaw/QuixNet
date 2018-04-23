/*
 * File:   TCPSocketWin32.h
 * Author: Jehu Shaw
 *
 * Created on 2011.10.15
 */
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )

#ifndef TCPSOCKETWIN32_H
#define	TCPSOCKETWIN32_H

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

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_AIX) && !defined(HAVE_GETHOSTBYNAME_R) && !defined(__NetBSD__) && !defined(VXWORKS)
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
        SocketLink()
			: fd(INVALID_SOCKET)
			, binaryAddress(0)
			, port(0)
            , pBuffer(NULL)
            , ptr(NULL)
			, recZeroCount(0)
        {
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
        friend class TCPSocketWin32;

		SOCKET fd;
        /// The peer address from inet_addr.
        unsigned int binaryAddress;
        /// The port number
        unsigned short port;
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
			recZeroCount = 0;
        }
    } PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

    class SHARED_DLL_DECL TCPSocketWin32 : public TCPSocketBase {
    public:
        TCPSocketWin32();
        virtual ~TCPSocketWin32();
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
			int socketIdx,
			unsigned char* prefixData = NULL,
			unsigned int prefixLength = 0);

        //Disconnects aplayer/address
        virtual void CloseConnection(const SocketID& socketId);

    protected:

        bool m_isLittleEndian;

		typedef util::CXqxTable0S<SocketLink> socket_links_t;
		socket_links_t* m_pSktLinks;

		volatile bool m_isStarted;

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
        inline bool RemoveSocketLink(SocketID& socketId) {
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
        bool OnRecvCompletion(SocketLink& socketLink, DWORD dwTrans);
        bool OnSendCompletion(SocketLink& socketLink);
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
#endif	/* INETSOKETWIN32_H */

#endif
