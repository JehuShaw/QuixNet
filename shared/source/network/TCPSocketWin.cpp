/* 
 * File:   TCPSocketWin.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2011.10.15
 */
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined ( _WIN64 )

#include "TCPSocketWin.h"
#include "NetworkTypes.h"
#include "NetworkTrace.h"
#include <windows.h>
#include <process.h> 
#include <stdio.h>
#include "ScopedLock.h"

#pragma comment(lib, "WS2_32.lib")

using namespace thd;

namespace ntwk
{
    unsigned int _stdcall ListenTcpInterfaceLoop(void * arguments){
        TCPSocketWin * sts = (TCPSocketWin *) arguments;
        sts->ListenRun();
        return 0;
    }
    unsigned int _stdcall UpdateTcpInterfaceLoop(void * arguments){
        TCPSocketWin * sts = (TCPSocketWin *) arguments;
        sts->UpdateRun();
        return 0;
    }

    DWORD GetCPUCoreNumber(void)
    {
        SYSTEM_INFO SystemInfo;

        GetSystemInfo(&SystemInfo);	
        return(SystemInfo.dwNumberOfProcessors);
    }


    DWORD GetWorkerThreadNumber(void)
    {
        return (GetCPUCoreNumber() + 2);
    }

	//定义结构及宏  
	struct TCP_KEEPALIVE_T 
	{  
		u_long onoff;  
		u_long keepalivetime;  
		u_long keepaliveinterval;  
	} ;  

#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)


TCPSocketWin::TCPSocketWin()
					: m_pSktLinks(NULL)
                    , m_isStarted(false)
                    , m_isListened(false)
                    , m_dwListenCount(0)
                    , m_nWorkerThreadCount(0) {

    memset(m_listenFDs, INVALID_SOCKET, MAX_LISTENER_SIZE);

    if(htonl(12345) != 12345){
        m_isLittleEndian = true;
    }else{
        m_isLittleEndian = false;
    }
}

TCPSocketWin::~TCPSocketWin() {
	// If assert false, call Stop() first.
	assert(!m_isStarted);
}

bool TCPSocketWin::Start(const char* address, unsigned short maxLink, unsigned short threadNum/* = 0*/) {

    if(address == NULL) { 
        TRACE_MSG("Start, address == NULL \n");
        return false;
    }

    if(maxLink < 1){
        TRACE_MSG("Start, maxLink < 1 \n");
        return false;
    }

    if(atomic_cmpxchg8((volatile char*)&m_isListened
        , (char)false, (char)true) != (char)false) {
            TRACE_MSG("Start, Already Listened \n");
            return false;
    }

    if(atomic_cmpxchg8((volatile char*)&m_isStarted
        , (char)false, (char)true) != (char)false) {
            m_isListened = false;
            TRACE_MSG("Start, Already Started \n");
            return false;
    }

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) {
		m_isListened = false;
        m_isStarted = false;
		TRACE_MSG("WSAStartup failed with error code: %d\n", GetLastError());
		return false;
	}
	if(2 != HIBYTE(wsaData.wVersion) || 2 != LOBYTE(wsaData.wVersion)) {  
		m_isListened = false;
        m_isStarted = false;
		TRACE_MSG("Start, Socket version not supported.\n");
		WSACleanup();
		return false;
	}

    if(!CreateListen(address, maxLink)) {
        m_isListened = false;
        m_isStarted = false;
        WSACleanup();
        return false;
    }
    
	m_pSktLinks = new socket_links_t(maxLink);

	// Create I/O Completion Port  
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if(NULL == m_hIOCP){
		m_isListened = false;
        m_isStarted = false;
        for(int i = 0; i < MAX_LISTENER_SIZE; ++i) {
            closesocket(m_listenFDs[i]);
        }
		WSACleanup();
		delete m_pSktLinks;
		m_pSktLinks = NULL;
		TRACE_MSG("Start, CreateIoCompletionPort failed with error code: %d\n", GetLastError());
		return false;
	} 

    // memory barrier
    memory_barrier();

	// Create worker thread
    if(0 == threadNum) {
        threadNum = (unsigned short)GetWorkerThreadNumber();
    }
	m_receiveDone.Suspend();
    HANDLE threadHandle(NULL);
    for(DWORD i = 0; i < threadNum; ++i) {
	    threadHandle = (HANDLE)_beginthreadex(NULL, 0, UpdateTcpInterfaceLoop, (LPVOID)this, 0, NULL);
	    if(threadHandle == NULL){
            continue;
	    }
        atomic_inc(&m_nWorkerThreadCount);
        CloseHandle(threadHandle);
    }
    if(m_nWorkerThreadCount < 1) {
        m_isListened = false;
        m_isStarted = false;
		m_receiveDone.Resume();
        for(int i = 0; i < MAX_LISTENER_SIZE; ++i) {
            closesocket(m_listenFDs[i]);
        }
        CloseHandle(m_hIOCP);
		delete m_pSktLinks;
		m_pSktLinks = NULL;
        WSACleanup();
        TRACE_MSG("Start, UPDATE pthread_create failed\n");
        return false;
    }

	m_listenDone.Suspend();
    //鍒濆鍖栫嚎绋? 
	threadHandle = (HANDLE)_beginthreadex(NULL, 0, ListenTcpInterfaceLoop, (LPVOID)this, 0, NULL);
    if(threadHandle == NULL){
        m_isListened = false;
        m_isStarted = false;
		m_listenDone.Resume();
        for(int i = 0; i < MAX_LISTENER_SIZE; ++i) {
            closesocket(m_listenFDs[i]);
        }
		CloseHandle(m_hIOCP); // Close IOCP  
		delete m_pSktLinks;
		m_pSktLinks = NULL;
		WSACleanup();
        TRACE_MSG("Start, LISTEN pthread_create failed\n");
        return false;
    }
	CloseHandle( threadHandle );

    return true;
}

bool TCPSocketWin::CreateListen(const char* address, unsigned short maxLink) {

    char hostname[MAX_HOST_NAME] = { '\0' };
    unsigned short port(0);
    char* p(NULL);
    if ((p = (char*)strchr(address, ':')) == NULL
        || unsigned(p - address) >= sizeof(hostname)
        || sscanf(p+1, "%hu", &port) != 1)
    {
        TRACE_MSG("@%s, Invalid address: %s\n", __FUNCTION__, address);
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';

    sockaddr_in sock_inet;  
    sock_inet.sin_family = AF_INET;
    if (*hostname && strcmp(hostname, "localhost") != 0) {
        struct hostent* hp;
#if defined(HAVE_GETHOSTBYNAME_R) && !defined(NO_PTHREADS)
        struct hostent ent;  // entry in hosts table
        char buf[GETHOSTBYNAME_BUF_SIZE];
        int h_err;
#if defined(__sun)
        if ((hp = gethostbyname_r(hostname, &ent, buf, sizeof buf, &h_err)) == NULL
#else
        if (gethostbyname_r(hostname, &ent, buf, sizeof buf, &hp, &h_err) != 0
            || hp == NULL
#endif
            || hp->h_addrtype != AF_INET)
#else
        if ((hp = gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET)
#endif
        {
            TRACE_MSG("@%s, Failed to get host by name: %d\n", __FUNCTION__, errno);
            return false;
        }
        memcpy(&sock_inet.sin_addr, hp->h_addr,
            sizeof sock_inet.sin_addr);
    } else {
        sock_inet.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    sock_inet.sin_port = htons(port);

    SOCKET listenfd = WSASocket(sock_inet.sin_family,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
    if(listenfd == INVALID_SOCKET){

        TRACE_MSG("@%s, WSASocket failed with error code: %d\n", __FUNCTION__, GetLastError());
        return false;
    }

    ULONG nNoBlock = 1;
    if(SOCKET_ERROR == ioctlsocket(listenfd, (LONG)FIONBIO, &nNoBlock)) {
        TRACE_MSG("@%s, ioctlsocket FIONBIO failed with error code: %d\n", __FUNCTION__, GetLastError());
        return false;
    }

    //int opt = 1;
    //setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&opt,sizeof(opt));

    if(bind(listenfd,(struct sockaddr *)&sock_inet,sizeof(sockaddr_in)) == SOCKET_ERROR){
        closesocket(listenfd);
        TRACE_MSG("@%s, bind failed with error code: %d\n", __FUNCTION__, GetLastError());
        return false;
    }

    if((listen(listenfd, maxLink)) == SOCKET_ERROR){
        closesocket(listenfd);
        TRACE_MSG("@%s, listen failed with error code: %d\n", __FUNCTION__, GetLastError());
        return false;
    }
    // 设置接收事件
    m_listenFDs[m_dwListenCount] = listenfd;
    m_hListenEvent[m_dwListenCount] = WSACreateEvent();
    WSAEventSelect(m_listenFDs[m_dwListenCount], m_hListenEvent[m_dwListenCount], FD_ACCEPT);
    ++m_dwListenCount;
    return true;
}
    
bool TCPSocketWin::Stop(void){

    if(atomic_cmpxchg8((volatile char*)&m_isListened
        , (char)true, (char)false) == (char)true) {
                    
        for(int i = 0; i < MAX_LISTENER_SIZE; ++i) {

            closesocket(m_listenFDs[i]);
            m_listenFDs[i] = INVALID_SOCKET;
        }
        // 触发关闭事件
        if(m_dwListenCount > 0) {
            WSASetEvent(m_hListenEvent[0]);
        }
        //wait for the thread to stop
        m_listenDone.Wait();
        // close event
        DWORD dwCurEventSize = m_dwListenCount;
        for(int j = 0; j < (int)dwCurEventSize; ++j) {
            WSACloseEvent(m_hListenEvent[j]);
        }

		m_dwListenCount = 0;
    }

	if(atomic_cmpxchg8((volatile char*)&m_isStarted
		, (char)true, (char)false) != (char)true) {
			TRACE_MSG("Stop, !m_isStarted \n");
			return false;
	}

	if(m_pSktLinks) {
		socket_links_t::iterator it(m_pSktLinks->Begin());
		for(; m_pSktLinks->End() != it; ++it) {
			socket_links_t::pair_t pair(it.GetPair());
			if(XQXTABLE_INDEX_NIL == pair.GetIndex()) {
				continue;
			}
			const SocketLink* pSktLink = pair.GetValue();
			if(pSktLink) {
				shutdown(pSktLink->fd, SD_BOTH);
			} else {
				assert(pSktLink);
			}
		}
	} else {
		TRACE_MSG("Stop, NULL == m_pSktLinks \n");
	}

	CloseHandle(m_hIOCP);
    //wait for the thread to stop
    m_receiveDone.Wait();
	// wait delete m_pSktLinks;
	delete m_pSktLinks;
	m_pSktLinks = NULL;

    return true;
}

bool TCPSocketWin::Connect(SocketID& socketId, const char* address, unsigned short threadNum /* = 0*/){

    if(atomic_cmpxchg8((volatile char*)&m_isStarted
        , (char)false, (char)true) != (char)false) {

        TRACE_MSG("Connect, Already started \n");
        return false;
    }

    char hostname[MAX_HOST_NAME];
    unsigned short port;
    
    char* p;
    if ((p = strchr((char*)address, ':')) == NULL
        || unsigned(p - address) >= sizeof(hostname)
        || sscanf(p+1, "%hu", &port) != 1)
    {
        m_isStarted = false;
        TRACE_MSG("Connect, (p = strchr((char*)address, ':')) == NULL"
            "|| unsigned(p - address) >= sizeof(hostname)"
            "|| sscanf(p+1, \"%hu\", &port) != 1\n");
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) {
        m_isStarted = false;
		TRACE_MSG("Connect, WSAStartup failed with error code: %d\n", GetLastError());
		return false;
	}
	if(2 != HIBYTE(wsaData.wVersion) || 2 != LOBYTE(wsaData.wVersion)) {  
        m_isStarted = false;
		TRACE_MSG("Connect, Socket version not supported.\n");
		WSACleanup();
		return false;
	}

	sockaddr_in sock_inet;
    struct hostent* hp;
#if defined(HAVE_GETHOSTBYNAME_R) && !defined(NO_PTHREADS)
    struct hostent ent;  // entry in hosts table
    char buf[GETHOSTBYNAME_BUF_SIZE];
    int h_err;
#if defined(__sun)
    if ((hp = gethostbyname_r(hostname, &ent, buf, sizeof buf, &h_err)) == NULL
#else
    if (gethostbyname_r(hostname, &ent, buf, sizeof buf, &hp, &h_err) != 0
        || hp == NULL
#endif
        || hp->h_addrtype != AF_INET)
#else
    if ((hp = gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET)
#endif
    {
        m_isStarted = false;
		WSACleanup();
        TRACE_MSG("Connect, (hp = gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET"
            " hp = %lx \n", (long)hp);
        return false;
    }

    memset(&sock_inet, 0, sizeof(sock_inet));
    sock_inet.sin_family = AF_INET;
    sock_inet.sin_port = htons( port );

#if !defined(_COMPATIBILITY_1)
    memcpy((char *)&sock_inet.sin_addr, (char *)hp->h_addr, sizeof sock_inet.sin_addr);
#else
    sock_inet.sin_addr.s_addr = inet_addr( host );
#endif
    SOCKET sockfd = WSASocket(sock_inet.sin_family, SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
    if (sockfd == INVALID_SOCKET) {
        m_isStarted = false;
		WSACleanup();
        TRACE_MSG("Connect, WSASocket failed with error code: %d\n", GetLastError());
        return false;
    }

    // This is blocking but whatever.  If I need it non-blocking I will make it so later.
    if (connect(sockfd, (struct sockaddr*)&sock_inet, sizeof(sock_inet)) != 0){
        m_isStarted = false;
        closesocket(sockfd);
		WSACleanup();
        TRACE_MSG("Connect, connect failed with error code: %d\n", GetLastError());
        return false;
    }

	// KeepAlive实现  
	TCP_KEEPALIVE_T inKeepAlive = {0};                // 输入参数  
	TCP_KEEPALIVE_T outKeepAlive = {0};               // 输出参数  
	unsigned long ulInLen = sizeof(TCP_KEEPALIVE_T);   
	unsigned long ulOutLen = sizeof(TCP_KEEPALIVE_T);   
	unsigned long ulBytesReturn = 0;   

	// 设置socket的keep alive为5秒，并且发送次数为3次  
	inKeepAlive.onoff = 1;   
	// 两次KeepAlive探测间的时间间隔 
	inKeepAlive.keepaliveinterval = KEEPALIVE_INTERVAL_MS;
	// 开始首次KeepAlive探测前的TCP空闭时间 
	inKeepAlive.keepalivetime = KEEPALIVE_IDLE_MS; 

	if(WSAIoctl(sockfd, SIO_KEEPALIVE_VALS,(LPVOID)&inKeepAlive, ulInLen,
		(LPVOID)&outKeepAlive, ulOutLen, &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)   
	{   
		TRACE_MSG("WSAIoctl failed. error code(%d)!\n", WSAGetLastError());  
	} 

	m_pSktLinks = new socket_links_t(1);

    // init epoll
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if(NULL == m_hIOCP){
        m_isStarted = false;
		closesocket(sockfd);
		WSACleanup();
		delete m_pSktLinks;
		m_pSktLinks = NULL;
		TRACE_MSG("Connect, CreateIoCompletionPort failed with error code: %d\n", GetLastError());
		return false;
	}
    //鍒濆鍖栫嚎绋?
    if(0 == threadNum) {
        threadNum = 2;
    }
	m_receiveDone.Suspend();
    HANDLE threadHandle(NULL);
    for(DWORD i = 0; i < threadNum; ++i) {
        threadHandle = (HANDLE)_beginthreadex(NULL, 0, UpdateTcpInterfaceLoop, (LPVOID)this, 0, NULL);
        if(threadHandle == NULL){
            continue;
        }
        atomic_inc(&m_nWorkerThreadCount);
        CloseHandle(threadHandle);
    }
    if(m_nWorkerThreadCount < 1){
        m_isStarted = false;
		m_receiveDone.Resume();
        closesocket(sockfd);
        delete m_pSktLinks;
        m_pSktLinks = NULL;
		WSACleanup();
        TRACE_MSG("Connect, pthread_create failed\n");
        return false;
    }

    // add client
	SocketLink sktLink(false);
	sktLink.fd = sockfd;
    sktLink.binaryAddress = inet_addr(hostname);
    sktLink.port = port;
    sktLink.CreateBuffer();
    if(NULL == sktLink.ptr){
        sktLink.ptr = AllocateLinkerData();
    }
	SocketLink* pSktLink = m_pSktLinks->AddSetIndex(sktLink, (uint32_t&)sktLink.index);
	if(NULL == pSktLink) {
		m_isStarted = false;
		m_receiveDone.Resume();
        closesocket(sockfd);
        delete m_pSktLinks;
        m_pSktLinks = NULL;
		WSACleanup();
		TRACE_MSG("Can't operate more then %d sockets \n", m_pSktLinks->Size());
		assert(false);
		return false;
	}

	// Associate with IOCP
    ULONG nNoBlock = 1;
	if((SOCKET_ERROR == ioctlsocket(pSktLink->fd, (LONG)FIONBIO, &nNoBlock))
        || NULL == CreateIoCompletionPort((HANDLE)(pSktLink->fd), m_hIOCP, (ULONG_PTR)pSktLink, 0))  {  
		TRACE_MSG("Connect, CreateIoCompletionPort failed with error code: %d\n", GetLastError());
		m_isStarted = false;
		m_receiveDone.Resume();
		closesocket(pSktLink->fd); 
		CloseHandle(m_hIOCP);
		delete m_pSktLinks;
		m_pSktLinks = NULL;
		WSACleanup();
		return false;
	} 

	// Post Receive  
	if(!PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)pSktLink, &(pSktLink->perIoDataRec.ol))) {
		TRACE_MSG("Connect, PostQueuedCompletionStatus failed with error code: %d\n", GetLastError());
		m_isStarted = false;
		m_receiveDone.Resume();
		closesocket(pSktLink->fd);
		CloseHandle(m_hIOCP);
		delete m_pSktLinks;
		m_pSktLinks = NULL;
		WSACleanup();
		return false;
	}

    pSktLink->GetSocketID(socketId);
    return true;
}

void TCPSocketWin::CloseConnection(const SocketID& socketId, int nWhy) {

	if(!m_isStarted) {
		TRACE_MSG("CloseConnection, !m_isStarted \n");
		return;
	}

	if(NULL == m_pSktLinks) {
		TRACE_MSG("CloseConnection, NULL == m_pSktLinks \n");
		return;
	}

	SocketLink* pSktLink = m_pSktLinks->Find(socketId.index);
    if(NULL == pSktLink || !pSktLink->IsSocketID(socketId)) {
        TRACE_MSG("CloseConnection, NULL == pSktLink || !pSktLink->IsSocketID(socketId)"
            " : socketId.index = %d,  pSktLink = %x \n", socketId.index, pSktLink);
        return;
    }
	atomic_xchg(&pSktLink->lostReason, nWhy);
	shutdown(pSktLink->fd, SD_BOTH);
}

bool TCPSocketWin::Send(
	unsigned char* data,
    unsigned int length,
    const SocketID& socketId,
	unsigned char* prefixData,
	unsigned int prefixLength)
{
    if(NULL == data || 0 == length) {

        TRACE_MSG("Send, (unsigned char*)0  == data || 0 == length"
            " : data = %lx, length = %d \n", (long)data, length);
        return false;
    }

	if(!m_isStarted) {
		TRACE_MSG("Send, !m_isStarted \n");
		return false;
	}

	if(NULL == m_pSktLinks) {
		TRACE_MSG("Send, NULL == m_pSktLinks \n");
		return false;
	}

	SocketLink* pSktLink = m_pSktLinks->Find(socketId.index);
	if(NULL == pSktLink || !pSktLink->IsSocketID(socketId)) {
		TRACE_MSG("Send, NULL == m_pSktLinks || !pSktLink->IsSocketID(socketId) pSktLink = %x \n", pSktLink);
        return false;
    }

	//write data
	Packet * p = pSktLink->outgoingMessages.WriteLock();
	SendCallback(*p,data,length,prefixData, prefixLength);
	pSktLink->outgoingMessages.WriteUnlock();

	int nSize = pSktLink->outgoingMessages.Size();
	if(1 == nSize) {
		if(!PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)pSktLink, &(pSktLink->perIoDataSen.ol))) {
			int nError = WSAGetLastError();
			TRACE_MSG("Send, PostQueuedCompletionStatus failed with error: %d \n", nError);
			return false;
		}
	} else {
		assert(nSize > -1);
	}
    return true;
}

bool TCPSocketWin::Exist(const SocketID& socketId) const {
	if(!m_isStarted) {
		return false;
	}

	if(NULL == m_pSktLinks) {
		TRACE_MSG("Exist, NULL == m_pSktLinks \n");
		return false;
	}

	SocketLink* pSktLink = m_pSktLinks->Find(socketId.index);
	if (NULL == pSktLink || !pSktLink->IsSocketID(socketId)) {
		return false;
	}
	return true;
}

uint32_t TCPSocketWin::Size() const {
	if(NULL == m_pSktLinks) {
		TRACE_MSG("Size, NULL == m_pSktLinks \n");
		return 0;
	}
	return m_pSktLinks->Size();
}

void TCPSocketWin::ClearSocketLink(){

	if(NULL == m_pSktLinks) {
		TRACE_MSG("ClearSocketLink, NULL == m_pSktLinks \n");
		return;
	}

	socket_links_t::iterator it(m_pSktLinks->Begin());
	for(; m_pSktLinks->End() != it; ++it) {
		socket_links_t::pair_t pair(it.GetPair());
		if(XQXTABLE_INDEX_NIL == pair.GetIndex()) {
			continue;
		}
		SocketLink* pSktLink = pair.GetValue();
		if(pSktLink) {
			RemoveSocketLink(*pSktLink);
		} else {
			assert(pSktLink);
		}
	}
}

bool TCPSocketWin::RemoveSocketLink(SocketLink& socketLink) {

	if(NULL == m_pSktLinks) {
		return false;
	}

	int nIndex = socketLink.index;
	SocketID socketId;
	socketLink.GetSocketID(socketId);
	int nWhy = socketLink.GetLostReason();

	closesocket(socketLink.fd);

	// clear RemoteClient
	if(socketLink.outgoingMessages.ReadIsLocked()) {
		socketLink.outgoingMessages.ReadUnlock();
	}

	if (XQXTABLE_INDEX_NIL != nIndex) {
		m_pSktLinks->Remove(nIndex, &SocketLink::Clear);
	}

	DisconnectCallback(nIndex, socketId, nWhy);
	return true;
}

bool TCPSocketWin::OnRecvCompletion(SocketLink& socketLink, DWORD dwTrans)  {

	ReceiveBuffer* pBuffer = socketLink.pBuffer;
	if(NULL == pBuffer) {
		TRACE_MSG("SocketLinker failed with pBuffer == NULL.\n");
		return false;
	}

	if(dwTrans > 0){

		pBuffer->SetWriteSize(dwTrans);

		bool bReadMore = false;
		size_t bufLength = 0;
		unsigned char* buffer = pBuffer->GetCanReadBuffer(bufLength, bReadMore);
		int readLen = 0;
		if(bReadMore){

			size_t moreBufLength = 0;
			unsigned char* moreBuf = pBuffer->GetCanReadBuffer(moreBufLength, bReadMore);

			readLen = ReceiveCallback(socketLink,buffer,bufLength,moreBuf,moreBufLength);
		}else{
			readLen = ReceiveCallback(socketLink,buffer,bufLength,NULL,0);
		}
		if(readLen < 0) {
			MisdataCallback(socketLink, buffer, bufLength);
			return false;
		} else {
			pBuffer->SetReadSize(readLen);
		}
		socketLink.recZeroCount = 0;
	} else {
		if(++socketLink.recZeroCount > MAX_RECEVIE_ZERO_COUNT) {
			socketLink.recZeroCount = 0;
			TRACE_MSG("SocketLinker failed with dwTrans <= 0.\n");
			return false;
		}
	}

	size_t bufSize = 0;
    unsigned char* buffer = pBuffer->GetCanWriteBuffer(bufSize);
    if(bufSize < 1) {
		TRACE_MSG("SocketLinker failed with can't get buffer size.\n");
		return false;
	}

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)buffer;
	wsaBuf.len = (u_long)bufSize;
	DWORD dwFlags(0);
	DWORD dwRecTrans(0);

	if(SOCKET_ERROR == WSARecv(socketLink.fd, &wsaBuf, 1, &dwRecTrans,
		&dwFlags, &(socketLink.perIoDataRec.ol), NULL)) {

		int nError = WSAGetLastError();
		if(WSA_IO_PENDING != nError) {
			TRACE_MSG("WSARecv failed with error code: %d.\n", nError);
			return false;
		}
	}
	return true;
}

bool TCPSocketWin::OnSendCompletion(SocketLink& socketLink) {

	if(socketLink.outgoingMessages.ReadIsLocked()) {
		socketLink.outgoingMessages.ReadUnlock();
	}

    Packet * p = socketLink.outgoingMessages.ReadLock();
    if(NULL == p) {
		return true;
	}

    if(!SendCompletion(socketLink, static_cast<char*>(p->data), static_cast<long>(p->length))) {
		socketLink.outgoingMessages.ReadUnlock();
		TRACE_MSG("WSASend failed with error code: %d.\n", nError);
		return false;
	}
	return true;
}

bool TCPSocketWin::SendCompletion(SocketLink& socketLink, char* data, long length)
{
	WSABUF wsaBuf;
	wsaBuf.buf = data;
	wsaBuf.len = length;
	DWORD dwSendTrans = 0;  
	DWORD dwFlags = 0;  
	while (length > 0) {
		if (WSASend(socketLink.fd, &wsaBuf, 1, &dwSendTrans, dwFlags,
				&(socketLink.perIoDataSen.ol), NULL) == 0) {
			data += dwSendTrans;
			length -= dwSendTrans;
			wsaBuf.buf = data;
			wsaBuf.len = length;
		} else if(WSA_IO_PENDING == WSAGetLastError()) {
			continue;
		} else {
			return false;
		}
	}
	return true;
}

void TCPSocketWin::ListenRun(){
    struct sockaddr_in sin;
    int sin_len = sizeof(struct sockaddr_in);
    DWORD dwResult;
    //查看发生的网络事件
    WSANETWORKEVENTS networkEvents;
    while(m_isListened){

        dwResult = WSAWaitForMultipleEvents(m_dwListenCount, m_hListenEvent, FALSE, WSA_INFINITE, FALSE);
        
        WSAEnumNetworkEvents(m_listenFDs[dwResult-WSA_WAIT_EVENT_0]
        , m_hListenEvent[dwResult-WSA_WAIT_EVENT_0]
        , &networkEvents);

        if(networkEvents.lNetworkEvents & FD_ACCEPT) {

            if(networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                //Error
                break;
            }

            SOCKET clientfd = WSAAccept(m_listenFDs[dwResult-WSA_WAIT_EVENT_0],(sockaddr *)&sin,&sin_len, NULL, NULL);
            if(INVALID_SOCKET == clientfd){ 
                continue;
            }

			if(NULL == m_pSktLinks) {
				closesocket(clientfd);
				TRACE_MSG("ListenRun, NULL == m_pSktLinks \n");
				continue;
			}

			// KeepAlive实现  
			TCP_KEEPALIVE_T inKeepAlive = {0};                // 输入参数  
			TCP_KEEPALIVE_T outKeepAlive = {0};               // 输出参数  
			unsigned long ulInLen = sizeof(TCP_KEEPALIVE_T);   
			unsigned long ulOutLen = sizeof(TCP_KEEPALIVE_T);   
			unsigned long ulBytesReturn = 0;   

			// 设置socket的keep alive为5秒，并且发送次数为3次  
			inKeepAlive.onoff = 1;   
			// 两次KeepAlive探测间的时间间隔 
			inKeepAlive.keepaliveinterval = KEEPALIVE_INTERVAL_MS;
			// 开始首次KeepAlive探测前的TCP空闭时间 
			inKeepAlive.keepalivetime = KEEPALIVE_IDLE_MS; 

			if(WSAIoctl(clientfd, SIO_KEEPALIVE_VALS,(LPVOID)&inKeepAlive, ulInLen,
				(LPVOID)&outKeepAlive, ulOutLen, &ulBytesReturn, NULL, NULL) == SOCKET_ERROR)   
			{   
				TRACE_MSG("WSAIoctl failed. error code(%d)!\n", WSAGetLastError());  
			} 

            // create remoteClient
            SocketLink sktLink(false);
			sktLink.fd = clientfd;
			sktLink.binaryAddress=sin.sin_addr.s_addr;
			sktLink.port=ntohs(sin.sin_port);
			sktLink.CreateBuffer();
			if(NULL == sktLink.ptr){
				sktLink.ptr = AllocateLinkerData();
			}
			SocketLink* pSktLink = m_pSktLinks->AddSetIndex(sktLink, (uint32_t&)sktLink.index);
			if(NULL == pSktLink) {
				closesocket(clientfd);
				TRACE_MSG("Can't operate more then %d sockets \n", m_pSktLinks->Size());
				assert(false);
				continue;
			}

			// Associate with IOCP  
			ULONG nNoBlock = 1;
			if((SOCKET_ERROR == ioctlsocket(pSktLink->fd, (LONG)FIONBIO, &nNoBlock))
				|| NULL == CreateIoCompletionPort((HANDLE)(pSktLink->fd), m_hIOCP, (ULONG_PTR)pSktLink, 0))  {  
					TRACE_MSG("CreateIoCompletionPort failed with error code: %d\n", GetLastError());
					closesocket(pSktLink->fd);		
					m_pSktLinks->Remove(pSktLink->index, &SocketLink::Clear);
					continue;
			}

			// Post Receive
			if (!PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)pSktLink, &(pSktLink->perIoDataRec.ol))) {
				TRACE_MSG("listen PostQueuedCompletionStatus return false.\n");
				closesocket(clientfd);	
				m_pSktLinks->Remove(pSktLink->index, &SocketLink::Clear);
				continue;
			}
			
            AcceptCallback(*pSktLink);
        }
    }
    m_listenDone.Resume();
}

void TCPSocketWin::UpdateRun() {

	PER_HANDLE_DATA* pPerHandleData(NULL);  
	LPPER_IO_DATA pPerIoData(NULL);  
	WSAOVERLAPPED* lpOverlapped(NULL);  
	DWORD dwTrans = 0;
	SOCKET fd = INVALID_SOCKET;

    while(m_isStarted) {

        BOOL bRet = GetQueuedCompletionStatus(m_hIOCP, &dwTrans, (PULONG_PTR)&pPerHandleData, &lpOverlapped, INFINITE);

        if(NULL == pPerHandleData || NULL == lpOverlapped) {
            break;
        }

		fd = pPerHandleData->fd;

        pPerIoData = CONTAINING_RECORD(lpOverlapped, PER_IO_DATA, ol); 
        if(NULL == pPerIoData){
            break;
        }
		
		if(bRet) {

            if(OP_READ == pPerIoData->opType) {

				CScopedLock scopeSpinlock(pPerIoData->spinLock);
                if(!OnRecvCompletion(*pPerHandleData, dwTrans)) {
					bRet = FALSE;
				}

            } else if(OP_WRITE == pPerIoData->opType) {

				CScopedLock scopeSpinlock(pPerIoData->spinLock);
				if(!OnSendCompletion(*pPerHandleData)) {
					bRet = FALSE;
				}
			}
        } else {
			int nError = WSAGetLastError();
			if(0 != nError) {
				TRACE_MSG("GetQueuedCompletionStatus failed with error: %d\n", nError);
			}
		}
		// If false remove the socket.
		if(FALSE == bRet) {
			CScopedLock scopeReclock(pPerHandleData->perIoDataRec.spinLock);
			CScopedLock scopeSenlock(pPerHandleData->perIoDataSen.spinLock);
			if (fd == pPerHandleData->fd) {
				RemoveSocketLink(*pPerHandleData);
			} else {
				assert(false);
			}
        }
    }

    if(atomic_dec(&m_nWorkerThreadCount) < 1) {
        ClearSocketLink();
        m_receiveDone.Resume();
    }
}

} // end namespace ntwk

#endif

