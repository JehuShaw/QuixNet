/*
 * File:   TCPSocketBSD.cpp
 * Author: Jehu Shaw
 *
 * Created on 2011_6_1 8:56
 */

#if defined( __NetBSD__ ) || defined( __APPLE__ )

#include "TCPSocketBSD.h"
#include "NetworkTypes.h"
#include "NetworkTrace.h"
#include <sys/event.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <wchar.h>
#include <string>
#include <string.h>
#include <iconv.h>
#include <time.h>
#include "TCPThreadPool.h"

#if defined(__APPLE__)
#include <mach/mach_host.h>
#else
#include <sys/sysinfo.h>
#endif

namespace ntwk
{
    void * ListenTcpInterfaceLoop(void * arguments) {
        TCPSocketBSD * sts = (TCPSocketBSD *) arguments;
        sts->ListenRun();
        return 0;
    }

    int GetCPUCoreNumber() {
#if defined(__APPLE__)
		host_basic_info_data_t hostInfo;
		mach_msg_type_number_t infoCount;

		infoCount = HOST_BASIC_INFO_COUNT;
		host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount) ;

		return (unsigned int)(hostInfo.max_cpus);
#else
		return get_nprocs();
#endif
    }

    int GetWorkerThreadNumber() {
        return (GetCPUCoreNumber() + 2);
    }

}
using namespace ntwk;

#ifdef VXWORKS
char* TCPSocketBSD::unix_socket_dir = (char *)"/comp/socket";
#else
char* TCPSocketBSD::unix_socket_dir = (char *)"/tmp/";
#endif // VXWORKS

TCPSocketBSD::TCPSocketBSD()
    : m_domain(sock_global_domain)
    , m_pSktLinks(NULL)
    , m_pEvents(0)
    , m_isStarted(false)
    , m_isListened(false)
    , m_listenfd(-1)
    , m_pThreadPool(NULL)
    , m_eventNum(0) {

    if(htonl(12345) != 12345){
        m_isLittleEndian = true;
    }else{
        m_isLittleEndian = false;
    }
}

TCPSocketBSD::TCPSocketBSD(socket_domain domain)
    : m_domain(domain)
    , m_maxLink(0)
    , m_pSktLinks(NULL)
    , m_pEvents(0)
    , m_isStarted(false)
    , m_isListened(false)
    , m_listenfd(-1)
    , m_pThreadPool(NULL)
    , m_eventNum(0) {

    if(htonl(12345) != 12345) {
        m_isLittleEndian = true;
    } else {
        m_isLittleEndian = false;
    }
}

TCPSocketBSD::~TCPSocketBSD() {
	// If assert false, call Stop() first.
    assert(!m_isStarted);
}

bool TCPSocketBSD::Start(const char* address, unsigned short maxLink, unsigned short threadNum/* = 0*/) {

    if(address == NULL){
        TRACE_MSG("Start, address == NULL \n");
        return false;
    }

    if(maxLink < 1){
        TRACE_MSG("Start, maxLink < 1 \n");
        return false;
    }

    m_maxLink = maxLink;

    if(atomic_cmpxchg8((volatile char*)&m_isListened
        , (char)true, (char)false) != (char)false) {

		TRACE_MSG("Start, Already listened \n");
		return false;
    }

    if(atomic_cmpxchg8((volatile char*)&m_isStarted
        , (char)true, (char)false) != (char)false) {

		m_isListened = false;
		TRACE_MSG("Start, Already started \n");
		return false;
    }

    char hostname[MAX_HOST_NAME];
    unsigned short port;
    const char* p;
    if ((p = strchr(address, ':')) == NULL
        || unsigned(p - address) >= sizeof(hostname)
        || sscanf(p+1, "%hu", &port) != 1)
    {
        m_isListened = false;
        m_isStarted = false;
        TRACE_MSG("Start, Invalid address: %s\n", address);
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';

    union {
        sockaddr    sock;
        sockaddr_in sock_inet;
#ifdef VXWORKS
        struct sockaddr_un usock;
#endif
        char        name[MAX_HOST_NAME];
    } u;
    int len;

    if (m_domain == sock_local_domain) {
#ifdef VXWORKS
        memset(&u.usock, 0, sizeof(struct sockaddr_un));
        u.usock.sun_family = AF_UNIX;
        u.usock.sun_len = len = sizeof (struct sockaddr_un);
        sprintf(u.usock.sun_path, "%s/0x%x", unix_socket_dir, port);
        unlink(u.usock.sun_path); // remove file if existed
#else
        u.sock.sa_family = AF_UNIX;
        assert(strlen(unix_socket_dir) + strlen(address)
                   < MAX_HOST_NAME - offsetof(sockaddr,sa_data));
        len = offsetof(sockaddr,sa_data) +
        sprintf(u.sock.sa_data, "%s%s.%u", unix_socket_dir, hostname, port);
        unlink(u.sock.sa_data);
#endif // VXWORKS
    }else{
        u.sock_inet.sin_family = AF_INET;
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
                m_isListened = false;
                m_isStarted = false;
                TRACE_MSG("Start, Failed to get host by name: %d\n", errno);
                return false;
            }
            memcpy(&u.sock_inet.sin_addr, hp->h_addr,
                   sizeof u.sock_inet.sin_addr);
        } else {
            u.sock_inet.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        u.sock_inet.sin_port = htons(port);
        len = sizeof(sockaddr_in);
    }

    if((m_listenfd = socket(u.sock.sa_family,SOCK_STREAM,0)) == -1){
        m_isListened = false;
        m_isStarted = false;
        TRACE_MSG("Start, soket failed\n");
        return false;
    }
    int opt = 1;
    setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&opt,sizeof(opt));

#ifdef VXWORKS
	if(bind(m_listenfd, reinterpret_cast<struct sockaddr*>(&u.usock), len) != 0) {
#else
    if(bind(m_listenfd,&u.sock,len) != 0){
#endif
        m_isListened = false;
        m_isStarted = false;
        close(m_listenfd);
        TRACE_MSG("Start, bind failed\n");
        return false;
    }

    if(listen(m_listenfd,maxLink) != 0){
        m_isListened = false;
        m_isStarted = false;
        close(m_listenfd);
        TRACE_MSG("Start, listen failed\n");
        return false;
    }

	m_pSktLinks = new socket_links_t(m_maxLink);
    //set file describe
    int maxfds = m_maxLink + 1;
    m_pEvents = new struct kevent[maxfds];
    m_kefd = kqueue();

    // memory barrier
    memory_barrier();

	m_listenDone.Suspend();
	// initiate thread
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&tid, &attr, ListenTcpInterfaceLoop, this) != 0){
        m_isListened = false;
        m_isStarted = false;
		m_listenDone.Resume();
        close(m_listenfd);
        close(m_kefd);
        delete [] m_pEvents;
        m_pEvents = NULL;
		delete m_pSktLinks;
		m_pSktLinks = NULL;
        TRACE_MSG("Start, LISTEN pthread_create failed\n");
        return false;
    }

    if(0 == threadNum) {
        threadNum = GetWorkerThreadNumber();
    }
    m_pThreadPool = TCPThreadPool::Create(threadNum, LaskFunc, LeaderFunc, this);
    if(NULL == m_pThreadPool){
        m_isListened = false;
        m_isStarted = false;
        close(m_listenfd);
        close(m_kefd);
        delete [] m_pEvents;
        m_pEvents = NULL;
		delete m_pSktLinks;
		m_pSktLinks = NULL;
        TRACE_MSG("Start, UPDATE pthread_create failed\n");
        return false;
    }

    return true;
}

bool TCPSocketBSD::Stop(void) {

    if(atomic_cmpxchg8((volatile char*)&m_isListened
        , (char)false, (char)true) == (char)true){

        shutdown(m_listenfd,SHUT_RDWR);
        if(m_listenfd != -1){
            close(m_listenfd);
            m_listenfd = -1;
        }
        //wait for the thread to stop
        m_listenDone.Wait();
    }

	if(atomic_cmpxchg8((volatile char*)&m_isStarted
		, (char)false, (char)true) != (char)true) {

		TRACE_MSG("Stop, Already started \n");
		return false;
	}

	if(m_pSktLinks) {
		socket_links_t::iterator it(m_pSktLinks->Begin());
		for(; m_pSktLinks->End() != it; ++it) {
			socket_links_t::value_t val(it.GetValue());
			if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
				continue;
			}
			SocketLink* pSktLink = val.pObject;
			if(pSktLink) {
				shutdown(pSktLink->fd, SHUT_RDWR);
			} else {
				assert(pSktLink);
			}
		}
	} else {
		TRACE_MSG("Stop, NULL == m_pSktLinks \n");
	}

	close(m_kefd);

    if(NULL != m_pThreadPool) {
		m_pThreadPool->Release();
		m_pThreadPool = NULL;
    }

	atomic_xchg(&m_eventNum, 0);

    delete [] m_pEvents;
    m_pEvents = NULL;

	delete m_pSktLinks;
	m_pSktLinks = NULL;

    return true;
}

bool TCPSocketBSD::Connect(SocketID& socketId, const char* address, unsigned short threadNum /* = 0*/) {

    if(atomic_cmpxchg8((volatile char*)&m_isStarted
        , (char)true, (char)false) != (char)false) {
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
        TRACE_MSG("Connect, Invalid address: %s \n", address);
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';

    union {
        sockaddr    sock;
        sockaddr_in sock_inet;
#ifdef VXWORKS
        struct sockaddr_un usock;
#endif
        char     name[MAX_HOST_NAME];
    } u;

    int len = 0;
    int sockfd = -1;
    if(m_domain == sock_local_domain) {
#ifdef VXWORKS
        memset(&u.usock, 0, sizeof(struct sockaddr_un));
        u.usock.sun_family = AF_UNIX;
        u.usock.sun_len = len = sizeof (struct sockaddr_un);
        sprintf(u.usock.sun_path, "%s/0x%x", unix_socket_dir, port);
#else
        u.sock.sa_family = AF_UNIX;
        assert(strlen(unix_socket_dir) + strlen(address)
                   < MAX_HOST_NAME - offsetof(sockaddr,sa_data));
        len = offsetof(sockaddr,sa_data) +
        sprintf(u.sock.sa_data, "%s%s.%u", unix_socket_dir, hostname, port);
#endif // VXWORKS
        sockfd = socket(u.sock.sa_family, SOCK_STREAM, 0);
        if (sockfd == -1) {
            m_isStarted = false;
            TRACE_MSG("Connect, socket failed with error: %d \n", errno);
            return false;
        }
		// This is blocking but whatever.  If need  non-blocking I will make it later.
#ifdef VXWORKS
        if (connect( sockfd, reinterpret_cast<struct sockaddr*>(&u.usock), len) != 0 ) {
#else
        if (connect(sockfd, &u.sock, len) != 0) {
#endif
            m_isStarted = false;
            close(sockfd);
            TRACE_MSG("Connect, connect failed with error: %d \n", errno);
            return false;
        }
    }else{
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
            TRACE_MSG("Connect, Failed to get host by name: %d \n", errno);
            return false;
        }

        memset(&u.sock_inet, 0, sizeof(u.sock_inet));
        u.sock_inet.sin_family = AF_INET;
        u.sock_inet.sin_port = htons( port );

#if !defined(_COMPATIBILITY_1)
        memcpy((char *)&u.sock_inet.sin_addr, (char *)hp->h_addr, sizeof u.sock_inet.sin_addr);
#else
        u.sock_inet.sin_addr.s_addr = inet_addr( host );
#endif
        sockfd = socket(u.sock_inet.sin_family, SOCK_STREAM, 0);
        if (sockfd  == -1) {
            m_isStarted = false;
            TRACE_MSG("Connect, socket failed with error: %d\n", errno);
            return false;
        }

        // This is blocking but whatever.  If need non-blocking I will make it so later.
        if (connect(sockfd,(struct sockaddr*) &u.sock_inet, sizeof(u.sock_inet)) != 0){
            m_isStarted = false;
            close(sockfd);
            TRACE_MSG("Connect, connect failed with error: %d\n", errno);
            return false;
        }
    }

    int keepAlive = 1;
    int keepSecs = KEEPALIVE_SECS;

    if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive,
        sizeof(keepAlive)) == -1) {

        TRACE_MSG("setsockopt SO_KEEPALIVE error!\n");
    }

    if(setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPALIVE,
        &keepSecs, sizeof(keepSecs)) == -1) {

        TRACE_MSG("setsockopt TCP_KEEPALIVE error!\n");
    }

    m_maxLink = 1;
    m_pSktLinks = new socket_links_t(m_maxLink);
    // init epoll
    int maxfds = m_maxLink + 1;
    m_pEvents = new struct kevent[maxfds];
    m_kefd = kqueue();
    // initiate thread
    if(0 == threadNum) {
        threadNum = 2;
    }

    m_pThreadPool = TCPThreadPool::Create(threadNum, LaskFunc, LeaderFunc, this);
    if(NULL == m_pThreadPool){
        m_isStarted = false;
        close(sockfd);
        close(m_kefd);
        delete [] m_pEvents;
        m_pEvents = NULL;
		delete m_pSktLinks;
		m_pSktLinks = NULL;
        TRACE_MSG("Connect, pthread_create failed\n");
        return false;
    }

    // add client
	socket_links_t::value_t val(m_pSktLinks->Add());
	SocketLink* pSktLink = val.pObject;
	assert(pSktLink);
	// write data
	SetNonblocking(sockfd);
	pSktLink->fd = sockfd;
    pSktLink->binaryAddress = inet_addr(hostname);
    pSktLink->port = port;
    pSktLink->CreateBuffer();
    if(NULL == pSktLink->ptr){
        pSktLink->ptr = AllocateLinkerData();
    }
    pSktLink->recLock = false;
    pSktLink->senLock = false;

    // set file describe
    struct kevent changes[2] = {{0,0,0,0,0,NULL},{0,0,0,0,0,NULL}};
    EV_SET(&changes[0], pSktLink->fd, EVFILT_READ, EV_ADD, 0, 0, pSktLink);
    EV_SET(&changes[1], pSktLink->fd, EVFILT_WRITE, EV_ADD, 0, 0, pSktLink);

    if(kevent(m_kefd, changes, 2, NULL, 0, NULL) == -1) {
        m_isStarted = false;
        close(sockfd);
        close(m_kefd);
        delete [] m_pEvents;
        m_pEvents = NULL;
		delete m_pSktLinks;
		m_pSktLinks = NULL;
        TRACE_MSG("Connect, kevent failed with error: %d\n", errno);
        return false;
    }

    pSktLink->GetSocketID(socketId, 0);
    return true;
}

void TCPSocketBSD::CloseConnection(const SocketID& socketId) {

	if(!m_isStarted) {
		TRACE_MSG("CloseConnection, !m_isStarted \n");
		return;
	}

	if(NULL == m_pSktLinks) {
		TRACE_MSG("CloseConnection, NULL == m_pSktLinks \n");
		return;
	}

	socket_links_t::value_t val(m_pSktLinks->Find(socketId.index));
	if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
		TRACE_MSG("CloseConnection, XQXTABLE0S_INDEX_NIL == val.nIndex"
			" : socketId.index = %d \n", socketId.index);
		return;
	}

	SocketLink* pSktLink = val.pObject;
	if(NULL == pSktLink) {
		assert(pSktLink);
		return;
	}

    shutdown(pSktLink->fd, SHUT_RDWR);
}

bool TCPSocketBSD::Send(
	unsigned char* data,
	unsigned int length,
	int socketIdx,
	unsigned char* prefixData,
	unsigned int prefixLength){

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

	socket_links_t::value_t val(m_pSktLinks->Find(socketIdx));
	if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
		return false;
	}

	SocketLink* pSktLink = val.pObject;
	if(NULL == pSktLink) {
		assert(pSktLink);
		return false;
	}

    //write data
    Packet * p = pSktLink->outgoingMessages.WriteLock();
    SendCallback(*p,data,length,prefixData, prefixLength);
    pSktLink->outgoingMessages.WriteUnlock();

	int nSize = pSktLink->outgoingMessages.Size();
	if(1 == nSize) {
		struct kevent changes = {0, 0, 0, 0, 0, NULL};
		EV_SET(&changes, pSktLink->fd, EVFILT_WRITE, EV_ENABLE, 0, 0, pSktLink);

        if(kevent(m_kefd, &changes, 1, NULL, 0, NULL) == -1) {
            TRACE_MSG("Send, kevent failed with error: %d fd = %d\n ", errno, pSktLink->fd);
			return false;
		}
	} else {
		assert(nSize > -1);
	}
	return true;
}

void TCPSocketBSD::ClearSocketLink(){

	if(NULL == m_pSktLinks) {
		TRACE_MSG("ClearSocketLink, NULL == m_pSktLinks \n");
		return;
	}

	socket_links_t::iterator it(m_pSktLinks->Begin());
	for(; m_pSktLinks->End() != it; ++it) {
		socket_links_t::value_t val(it.GetValue());
		if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
			continue;
		}
		SocketLink* pSktLink = val.pObject;
		if(pSktLink) {
			RemoveSocketLink(*pSktLink, val.nIndex);
		} else {
			assert(pSktLink);
		}
	}
}

bool TCPSocketBSD::RemoveSocketLink(SocketLink& socketLink, size_t index){

	if(NULL == m_pSktLinks) {
		return false;
	}

	if(XQXTABLE0S_INDEX_NIL == m_pSktLinks->Find(index).nIndex) {
		return false;
	}

	DisconnectCallback(socketLink);

	struct kevent changes[2] = {{0,0,0,0,0,NULL},{0,0,0,0,0,NULL}};
	EV_SET(&changes[0], socketLink.fd, EVFILT_READ, EV_DELETE, 0, 0, &socketLink);
	EV_SET(&changes[1], socketLink.fd, EVFILT_WRITE, EV_DELETE, 0, 0, &socketLink);

	if(kevent(m_kefd, changes, 2, NULL, 0, NULL) == -1) {
		TRACE_MSG("RemoveSocketLink, kevent failed with error: %d\n", errno);
	}
	close(socketLink.fd);

	// clear RemoteClient
	socketLink.Clear();

	m_pSktLinks->Remove(index);
	return true;
}

bool TCPSocketBSD::SetNonblocking(int sock){
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts == -1)
    {
        TRACE_MSG("fcntl(sock,GETFL)\n");
        return false;
    }
    opts |= O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts) == -1)
    {
        TRACE_MSG("fcntl(sock,SETFL,opts)\n");
        return false;
    }
    return true;
}

bool TCPSocketBSD::OnRecvCompletion(SocketLink& socketLink) {

    ReceiveBuffer* pBuffer = socketLink.pBuffer;
    if(NULL == pBuffer) {
        TRACE_MSG("OnRecvCompletion, failed with pBuffer == NULL.\n");
        return false;
    }

    for(;;) {
        size_t bufSize(0);
        unsigned char* buffer = pBuffer->GetCanWriteBuffer(bufSize);
        if(bufSize < 1){
            TRACE_MSG("SocketLinker failed with can't get buffer size.\n");
            return false;
        }
        int ret = recv(socketLink.fd, buffer, bufSize, 0);
        if(ret > 0){
            pBuffer->SetWriteSize(ret);
            bool more;
            buffer = pBuffer->GetCanReadBuffer(bufSize,more);
            int readLen(0);
            if(more){
                size_t moreBufLen(0);
                unsigned char* moreBuf = pBuffer->GetCanReadBuffer(moreBufLen,more);
                readLen = ReceiveCallback(socketLink,buffer,bufSize,moreBuf,moreBufLen);
            }else{
                readLen = ReceiveCallback(socketLink,buffer,bufSize,NULL,0);
            }
            if(readLen < 0){
                MisdataCallback(socketLink, buffer, bufSize);
                return false;
            }else{
                pBuffer->SetReadSize(readLen);
            }
            if(ret == (int)bufSize){
                continue;
            }
        } else {
            // ret == 0,  clientfd already had closed
            if(ret < 0){
                if(EAGAIN == errno
					// nonblocking socket no EINTR error.
                    //|| EINTR == errno
                    || EWOULDBLOCK == errno)
				{
                    return true;
                }
                TRACE_MSG((char*)"recv errno ........ = %d\n",errno);
            }
            return false;
        }
        break;
    }
	return true;
}

bool TCPSocketBSD::OnSendCompletion(SocketLink& socketLink) {

    const int nMsgSize = socketLink.outgoingMessages.Size();
    int nCount(0);

    for(;;) { // To avoid the accumulation of packet.
        Packet * p = socketLink.outgoingMessages.ReadLock();
        if(NULL == p) {
            break;
        }

		if(send(socketLink.fd, (char*)p->data,
			p->length, MSG_HAVEMORE) == -1)
		{
            if(EAGAIN == errno
				// nonblocking socket no EINTR error.
				//|| EINTR == errno
                || EWOULDBLOCK == errno)
			{
				socketLink.outgoingMessages.CancelReadLock(p);
				return true;
            }
			TRACE_MSG((char*)"send errno ........ = %d\n",errno);
			socketLink.outgoingMessages.ReadUnlock();
            return false;
        }

        if(++nCount < nMsgSize) {
			socketLink.outgoingMessages.ReadUnlock();
            continue;
        }

		socketLink.outgoingMessages.ReadUnlock();
        break;
    }

	if(socketLink.outgoingMessages.Size() < 1) {
			struct kevent changes = {0,0,0,0,0,NULL};
			EV_SET(&changes, socketLink.fd, EVFILT_WRITE, EV_DISABLE, 0, 0, &socketLink);
		do {
            if(kevent(m_kefd, &changes, 1, NULL, 0, NULL) == -1) {
               TRACE_MSG("OnSendCompletion, kevent failed with error: %d\n", errno);
            }
			if(socketLink.outgoingMessages.Size() > 0 && (changes.flags & EV_DISABLE)) {
				// If send is be doing and have a data, the flag can't be canceled.
				EV_SET(&changes, socketLink.fd, EVFILT_WRITE, EV_ENABLE, 0, 0, &socketLink);
				continue;
			}
		} while(false);
	}
	return true;
}

void TCPSocketBSD::ListenRun(){
    struct sockaddr_in sin;
    socklen_t sin_len = sizeof(struct sockaddr_in);

    while(m_isListened){
        int clientfd = accept(m_listenfd,(sockaddr *)&sin,&sin_len);
        if(clientfd == -1){
			TRACE_MSG("ListenRun, accept failed with error: %d\n", errno);
			continue;
		}

		if(NULL == m_pSktLinks) {
			close(clientfd);
			TRACE_MSG("ListenRun, NULL == m_pSktLinks \n");
			continue;
		}

		socket_links_t::value_t val(m_pSktLinks->Add());
		if(XQXTABLE0S_INDEX_NIL == val.nIndex) {
			close(clientfd);
			TRACE_MSG("Can't operate more then %d sockets \n", m_pSktLinks->Size());
			continue;
		}

		int keepAlive = 1;
		int keepSecs = KEEPALIVE_SECS;

		if(setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive,
			sizeof(keepAlive)) == -1) {

			TRACE_MSG("setsockopt SO_KEEPALIVE error!\n");
		}

		if(setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPALIVE,
			&keepSecs, sizeof(keepSecs)) == -1) {
			
			TRACE_MSG("setsockopt TCP_KEEPALIVE error!\n");
		}

        //create remoteClient
		SocketLink* pSktLink = val.pObject;
		if(NULL == pSktLink) {
			close(clientfd);
			m_pSktLinks->Remove(val.nIndex);
			assert(false);
			continue;
		}
		SetNonblocking(clientfd);
        pSktLink->fd = clientfd;
        pSktLink->binaryAddress=sin.sin_addr.s_addr;
        pSktLink->port=ntohs( sin.sin_port);
        pSktLink->CreateBuffer();
        if(NULL == pSktLink->ptr){
            pSktLink->ptr = AllocateLinkerData();
        }
        pSktLink->recLock = false;
        pSktLink->senLock = false;

		//set read event
		struct kevent changes[2] = {{0,0,0,0,0,NULL},{0,0,0,0,0,NULL}};
		EV_SET(&changes[0], pSktLink->fd, EVFILT_READ, EV_ADD, 0, 0, pSktLink);
		EV_SET(&changes[1], pSktLink->fd, EVFILT_WRITE, EV_ADD, 0, 0, pSktLink);

		if(kevent(m_kefd, changes, 2, NULL, 0, NULL) == -1) {
			TRACE_MSG("ListenRun, kevent failed with error: %d\n", errno);
			close(clientfd);
            pSktLink->Clear();
            m_pSktLinks->Remove(val.nIndex);
			continue;
		}

		AcceptCallback(*pSktLink);
    }
    m_listenDone.Resume();
}

#define EVERY_THREAD_PROCESS_EVENT_NUM 128
/* get need resume thread number. */
int TCPSocketBSD::LeaderFunc(void *argv)
{
    TCPSocketBSD * mgr = (TCPSocketBSD *) argv;

    /* wait event. */
    if (!mgr->m_isStarted) {
		return -1;
    } else {
		int num = kevent(mgr->m_kefd, NULL, 0, mgr->m_pEvents, mgr->m_maxLink + 1, NULL);
		if (num > 0) {
			atomic_xchg(&mgr->m_eventNum, num);
			//num = (num+(int)(EVERY_THREAD_PROCESS_EVENT_NUM)-1)/(int)(EVERY_THREAD_PROCESS_EVENT_NUM);
		} else if (num < 0) {

			if (num == -1 && errno == EINTR) {
				return 0;
			}
			TRACE_MSG("epoll_wait return value < 0, error, return value:%d, errno:%d", num, errno);
		}
		return num;
    }
}

int TCPSocketBSD::LaskFunc(void *argv)
{
    TCPSocketBSD * mgr = (TCPSocketBSD *) argv;

    for(;;)
    {
		if(!mgr->m_isStarted) {
            return -1;
		}

		struct kevent *ev = mgr->PopEvent();
		if(NULL == ev) {
            return 0;
		}

        assert(ev->udata != NULL);

        bool bRet = !(ev->flags & EV_DELETE) && !(ev->flags & EV_ERROR);
            
		if(bRet) {
			if(ev->filter & EVFILT_READ) {
				SocketLink& socketLink = *static_cast<SocketLink*>(ev->udata);
				if(atomic_cmpxchg8(&socketLink.recLock, true, false) == false) {
					if(!mgr->OnRecvCompletion(socketLink)) {
						bRet = false;
					}
					atomic_xchg8(&socketLink.recLock, false);
				}
			}

			if(ev->filter & EVFILT_WRITE) {
				SocketLink& socketLink = *static_cast<SocketLink*>(ev->udata);
            	if(atomic_cmpxchg8(&socketLink.senLock, true, false) == false) {
					if(!mgr->OnSendCompletion(socketLink)) {
						bRet = false;
					}
					atomic_xchg8(&socketLink.senLock, false);
				}
			}
		}

		if(!bRet) {
			SocketLink& socketLink = *static_cast<SocketLink*>(ev->udata);
			if(atomic_cmpxchg8(&socketLink.removeLock, true, false) == false) {
				size_t index = XQXTABLE0S_INDEX_NIL;
                socket_links_t* pSktLinks = mgr->m_pSktLinks;
				if(pSktLinks) {
					index = pSktLinks->GetIndexByPtr(&socketLink);
				} else {
					assert(pSktLinks);
				}
				mgr->RemoveSocketLink(socketLink, index);
				atomic_xchg8(&socketLink.removeLock, false);
			}
		}
	}

    return 0;
}


#endif
