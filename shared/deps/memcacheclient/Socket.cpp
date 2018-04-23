/*! @file       Socket.cpp
    @version    2.0
    @brief      Basic socket wrapper
 */
//#include "stdafx.h"

#include <errno.h>

#ifdef _WIN32
// C4127: conditional expression is constant
# pragma warning(disable: 4127) 
# pragma warning(push)
// C4702: unreachable code
// C4706: assignment within conditional expression
# pragma warning(disable: 4702 4706) 
# include <winsock2.h>
# include <ws2tcpip.h>
# pragma warning(pop)
// MSVC 2003-2008 never defined the EWOULDBLOCK etc errors,
// but MSVC 2010 does, however they are different values to
// the WSA errors that we need to check. This means that we 
// need to define our own error symbols to handle the MS FUBAR.
# define ERROR_WOULDBLOCK       WSAEWOULDBLOCK
# define ERROR_INPROGRESS       WSAEINPROGRESS
# define ERROR_TIMEDOUT         WSAETIMEDOUT
# define MSG_NOSIGNAL           0
# define GetLastSocketErrno     WSAGetLastError
// we don't need to handle EINTR on Windows, so just use false for this case
// which will be optimized out by the compiler
# define handleEINTR(rc)        false
typedef unsigned long           in_addr_t;

# pragma comment(lib, "ws2_32.lib")

#else
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <arpa/inet.h>
# include <netinet/tcp.h>
# include <netdb.h>
# include <fcntl.h>
// MSVC 2003-2008 never defined the EWOULDBLOCK etc errors,
// but MSVC 2010 does, however they are different values to
// the WSA errors that we need to check. This means that we 
// need to define our own error symbols to handle the MS FUBAR.
# define ERROR_WOULDBLOCK       EWOULDBLOCK
# define ERROR_INPROGRESS       EINPROGRESS
# define ERROR_TIMEDOUT         ETIMEDOUT
# define closesocket            close
# define ioctlsocket            ioctl
# define GetLastSocketErrno()   errno
# define handleEINTR(retval)    (retval < 0 && errno == EINTR)
# define INVALID_SOCKET         -1
#endif

#include "Socket.h"
#include <string>

#ifdef CROSSBASE_API
# include <xplatform/timer.h>
# include <Core/clpaths.h>
START_CL_NAMESPACE
#endif

// static function
bool
Socket::Resolve(
    const char *                aHost,
    std::vector<std::string> &  aAddress
    )
{
    aAddress.clear();

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;        // force IPv4
    hints.ai_socktype = SOCK_STREAM;    // force TCP
    hints.ai_protocol = IPPROTO_TCP;    // force IP

    // resolve the name
    struct addrinfo * results = NULL;
    int rc = getaddrinfo(aHost, "80", &hints, &results);
    if (rc != 0) return false;

    // loop over all results
    bool loopback = false;
    static const in_addr_t localhost = inet_addr("127.0.0.1");
    for (struct addrinfo * p = results; p != NULL; p = p->ai_next) {   
        if (p->ai_family != AF_INET) continue; // does this happen?
        
        const struct sockaddr_in * addr = (struct sockaddr_in *) p->ai_addr;
        if (addr->sin_addr.s_addr == INADDR_NONE) continue;
        if (addr->sin_addr.s_addr == INADDR_LOOPBACK || addr->sin_addr.s_addr == localhost) {
            loopback = true;
            continue;
        }

        aAddress.push_back(inet_ntoa(addr->sin_addr));
    }   
 
    freeaddrinfo(results);

    if (aAddress.empty() && loopback) {
        aAddress.push_back("127.0.0.1");
    }
    return true;
}

Socket::Socket(const ClTrace & aTrace) 
    : mTrace(aTrace)
    , mSocket(INVALID_SOCKET)
    , mIdx(0)
    , mBufLen(0) 
    , mConnectTimeout(2000)
    , mSendTimeout(1000)
    , mRecvTimeout(1000)
    , mBufferSize(0)
{ }

Socket::~Socket() 
{
    Disconnect();
}

bool Socket::IsConnected() const 
{ 
    return mSocket != INVALID_SOCKET; 
}

// Fuck you very much Microsoft for not following the specs for socket timeouts.
// This took me far too long to nail down the cause for the error.
//
// World:   http://pubs.opengroup.org/onlinepubs/009604599/functions/setsockopt.html
//  SO_RCVTIMEO     ...It accepts a timeval structure with the number of seconds and microseconds ...
// Windows: http://msdn.microsoft.com/en-us/library/windows/desktop/ms740476%28v=vs.85%29.aspx
//  SO_RCVTIMEO     DWORD or int depending on documentation     timeout in milliseconds
//
int Socket::setSocketTimeout(SOCKET aSocket, int aType, int aTimeout)
{
#ifdef _WIN32
    DWORD timeout = (DWORD) aTimeout;
#else
    struct timeval timeout;
    timeout.tv_sec  =  aTimeout / 1000;
    timeout.tv_usec = (aTimeout % 1000) * 1000;
#endif
    return setsockopt(aSocket, SOL_SOCKET, aType, (const char*) &timeout, sizeof(timeout));
}

void 
Socket::Disconnect()
{
    if (mSocket == INVALID_SOCKET) {
        return;
    }
    
    mTrace.Trace(CLINFO, "disconnect socket");

    // shutdown SD_SEND
    mTrace.Trace(CLDEBUG, "shutdown socket send");
    shutdown(mSocket, 1);

    // read all pending data
    mTrace.Trace(CLDEBUG, "drain the socket");
    setSocketTimeout(mSocket, SO_RCVTIMEO, 500);
    int rc = 1;
    while (rc > 0 || handleEINTR(rc)) {
        rc = recv(mSocket, mBuf, MAXBUF, 0);
        mTrace.Trace(CLDEBUG, "recv returned %d", rc);
    }

    // done
    mTrace.Trace(CLDEBUG, "closing socket");
    closesocket(mSocket);
    mSocket = INVALID_SOCKET;

    // clear the buffer
    mIdx = mBufLen = 0; 
}

void Socket::setSocketBlocking(SOCKET aSocket, bool aBlocking)
{
    const char * socktype = aBlocking ? "blocking" : "non-blocking";
    mTrace.Trace(CLDEBUG, "setting socket to %s", socktype);

#ifdef _WIN32
    u_long value = aBlocking ? 0 : 1;
    int rc = ioctlsocket(aSocket, FIONBIO, &value);
#else // !_WIN32
    int flags = fcntl(aSocket, F_GETFL, 0);
    if (aBlocking) {
        flags &= ~O_NONBLOCK;
    }
    else {
        flags |= O_NONBLOCK;
    }
    int rc = fcntl(aSocket, F_SETFL, flags);
#endif // _WIN32
    if (rc != 0) {
        const int errcode = GetLastSocketErrno();
        throw Exception(ERR_CONNECT, errcode, socktype);
    }
}

void Socket::waitForConnect(SOCKET aSocket)
{
    int rc;

#ifdef _WIN32
    try {
        // Windows doesn't implement fd_set as a bitmap but as an array of
        // socket handles. Because we are only adding a single socket here, 
        // we will never exceed the limits of FD_SET_SIZE.
        fd_set wr; FD_ZERO(&wr); FD_SET(aSocket, &wr); 
        fd_set ex; FD_ZERO(&ex); FD_SET(aSocket, &ex); 

        // non-blocking connect so that we can use a timeout. Windows 
        // doesn't return EINTR and so we don't need to loop
        struct timeval timeout;
        timeout.tv_sec  =  mConnectTimeout / 1000;
        timeout.tv_usec = (mConnectTimeout % 1000) * 1000;

        // clear the socket error status
        int err, len = sizeof(rc);
        getsockopt(aSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);

        mTrace.Trace(CLDEBUG, "connect: waiting for completion, timeout = %lums", mConnectTimeout);
        const int nfds = 0; // ignored by winsock
        rc = select(nfds, NULL, &wr, &ex, &timeout);

        const int errcode = GetLastSocketErrno();
        mTrace.Trace(CLDEBUG, "connect: select returned %d, errno %d", rc, errcode);

        if (rc != 1 || FD_ISSET(aSocket, &ex)) {
            // socket failed to connect, get the socket error status in case we 
            // can get more details from it
            len = sizeof(err);
            int rco = getsockopt(aSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            if (rco == 0) {
                mTrace.Trace(CLDEBUG, "connect: getsockopt(SO_ERROR) = %d", err);
            }
            else {
                mTrace.Trace(CLDEBUG, "connect: getsockopt(SO_ERROR) failed, returned %d, errno %d", 
                    rco, GetLastSocketErrno());
                err = 0;
            }

            if (rc < 0) {
                throw Exception(ERR_CONNECT, errcode, "select");
            }
            if (rc == 0) {
                throw Exception(ERR_CONNECT, ERROR_TIMEDOUT, "connection timeout");
            }

            // exception state is set
            throw Exception(ERR_CONNECT, err, "connection failed");
        }

        mTrace.Trace(CLDEBUG, "connect: success");
    }
    catch (const Exception & e) {
        mTrace.Trace(CLINFO, "connect exception: %s, %d", e.mDetail, e.mCode);
        throw;
    }
#else // !_WIN32
    fd_set *wr = NULL;
    try {
        // because the socket FD may be bigger than FD_SET_SIZE (e.g. if the client
        // has changed the max file desriptor limit using ulimit), we need to 
        // allocate the size for these sets first to avoid stack overflow.
        const unsigned long fd = (unsigned long) aSocket;
        const size_t fdsetBytes = sizeof(fd_set) * (fd + FD_SETSIZE - 1) / FD_SETSIZE;
        wr = (fd_set*) malloc(fdsetBytes);
        if (!wr) {
            throw Exception(ERR_CONNECT, ENOMEM, "malloc");
        }

        // non-blocking connect so that we can use a timeout
        struct timeval timeout;
        const unsigned long stop  = xplatform::GetCurrentTickCount() + mConnectTimeout;
        unsigned long delay = mConnectTimeout;
        int err;
        socklen_t len;
        rc = 0;
        do {
            if (rc) {
                delay = stop - xplatform::GetCurrentTickCount();
                if (delay == 0 || delay > (unsigned long) mConnectTimeout) break;
                mTrace.Trace(CLDEBUG, "connect: waiting for completion, timeout = %lums (EINTR)", delay);
            }
            else {
                mTrace.Trace(CLDEBUG, "connect: waiting for completion, timeout = %lums", delay);
            }

            // fdset gets modified by select, so reset it every time through loop
            memset(wr, 0, fdsetBytes);
            FD_SET(fd, wr); 

            // clear the socket error status
            len = sizeof(rc);
            getsockopt(aSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);

            errno = 0;
            timeout.tv_sec  =  delay / 1000;
            timeout.tv_usec = (delay % 1000) * 1000;
            rc = select(fd+1, NULL, wr, NULL, &timeout);
        } 
        while (handleEINTR(rc));

        const int errcode = GetLastSocketErrno();
        mTrace.Trace(CLDEBUG, "connect: select returned %d, errno %d", rc, errcode);

        if (rc != 1) {
            // socket failed to connect, get the socket error status in case we 
            // can get more details from it
            len = sizeof(err);
            int rco = getsockopt(aSocket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            if (rco == 0) {
                mTrace.Trace(CLDEBUG, "connect: getsockopt(SO_ERROR) = %d", err);
            }
            else {
                mTrace.Trace(CLDEBUG, "connect: getsockopt(SO_ERROR) failed, returned %d, errno %d", 
                    rco, GetLastSocketErrno());
            }

            if (rc < 0) {
                throw Exception(ERR_CONNECT, errcode, "select");
            }
            if (rc == 0) {
                throw Exception(ERR_CONNECT, ERROR_TIMEDOUT, "connection timeout");
            }
        }

        mTrace.Trace(CLDEBUG, "connect: success");
    }
    catch (const Exception & e) {
        mTrace.Trace(CLINFO, "connect exception: %s, %d", e.mDetail, e.mCode);
        if (wr) free(wr);
        throw;
    }
#endif // _WIN32
}

void Socket::connectSocket(SOCKET aSocket, const sockaddr_in * aServer)
{
    // socket must be in non-blocking mode already so that we can
    // connect and specify a maximum connection timeout

    int rc = connect(aSocket, (const struct sockaddr *) aServer, sizeof(*aServer));
    if (rc < 0) {
        const int errcode = GetLastSocketErrno();
        if (errcode != ERROR_WOULDBLOCK && errcode != ERROR_INPROGRESS) {
            throw Exception(ERR_CONNECT, errcode, "connect");
        }

        // wait for our socket to be connected
        waitForConnect(aSocket);
    }
}

void Socket::setSocketOptions(SOCKET aSocket)
{
    int rc;

    // turn off the Nagle algorithm as this socket is used for small requests which are
    // subject to delays otherwise. On Windows this can introduce delays of up to 200ms 
    // for a remote server (local servers are on the same TCP stack have no delay).
    int nodelay = 1;
    rc = setsockopt(aSocket, IPPROTO_TCP, TCP_NODELAY, (const char*) &nodelay, sizeof(nodelay));
    if (rc != 0) {
        const int errcode = GetLastSocketErrno();
        throw Exception(ERR_CONNECT, errcode, "setsockopt/ndelay");
    }

    rc = setSocketTimeout(aSocket, SO_RCVTIMEO, mRecvTimeout);
    if (rc != 0) {
        const int errcode = GetLastSocketErrno();
        throw Exception(ERR_CONNECT, errcode, "setsockopt/rcvtimeo");
    }

    rc = setSocketTimeout(aSocket, SO_SNDTIMEO, mSendTimeout);
    if (rc != 0) {
        const int errcode = GetLastSocketErrno();
        throw Exception(ERR_CONNECT, errcode, "setsockopt/sndtimeo");
    }
}

void Socket::setSocketBufferSize(SOCKET aSocket)
{
    if (mBufferSize <= 0) {
        return;
    }

    int rc;
    int len = 0;
    socklen_t siz;

    siz = sizeof(len);
    rc = getsockopt(aSocket, SOL_SOCKET, SO_RCVBUF, (char*) &len, &siz);
    if (rc < 0 || len < mBufferSize) {
        len = mBufferSize;
        rc = setsockopt(aSocket, SOL_SOCKET, SO_RCVBUF, (const char*) &len, sizeof(len));
        if (rc != 0) {
            const int errcode = GetLastSocketErrno();
            throw Exception(ERR_CONNECT, errcode, "setsockopt/rcvbuf");
        }
    }

    siz = sizeof(len);
    rc = getsockopt(aSocket, SOL_SOCKET, SO_SNDBUF, (char*) &len, &siz);
    if (rc < 0 || len < mBufferSize) {
        len = mBufferSize;
        rc = setsockopt(aSocket, SOL_SOCKET, SO_SNDBUF, (const char*) &len, sizeof(len));
        if (rc != 0) {
            const int errcode = GetLastSocketErrno();
            throw Exception(ERR_CONNECT, errcode, "setsockopt/sndbuf");
        }
    }
}

void Socket::Connect(const char * aIpAddress, int aPort)
{
    mTrace.Trace(CLINFO, 
        "Connect socket to %s:%d (connect: %dms, send: %dms, recv: %dms, buffer: %d)",
        aIpAddress, aPort, mConnectTimeout, mSendTimeout, mRecvTimeout, mBufferSize
        );

    Disconnect();

    SOCKET s = INVALID_SOCKET;
    try {
        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons((short)aPort);
        server.sin_addr.s_addr = inet_addr(aIpAddress);
        if (server.sin_addr.s_addr == INADDR_NONE) {
            const int errcode = GetLastSocketErrno();
            throw Exception(ERR_CONNECT, errcode, "invalid address");
        }

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET) {
            const int errcode = GetLastSocketErrno();
            throw Exception(ERR_CONNECT, errcode, "failed to create socket");
        }
        mTrace.Trace(CLDEBUG, "socket fd = int %d, unsigned %u", (int) s, (unsigned) s);

        setCloseOnExec(s);
        setSocketBlocking(s, false);
        connectSocket(s, &server);
        setSocketBlocking(s, true);
        setSocketBufferSize(s);
        setSocketOptions(s);

        mSocket = s;
    }
    catch (const Exception & e) {
        WideString sErrorMsg;
        GetLastError(e.mCode, sErrorMsg);
        mTrace.Trace(CLERROR, "Connect socket to %s:%d failed. Error: %s, %d, %S",
            aIpAddress, aPort, e.mDetail, e.mCode, sErrorMsg.c_str());
        if (s != INVALID_SOCKET) closesocket(s);
        throw;
    }

    int localPort = 0;
    std::string localIp;
    struct sockaddr_in local;
    socklen_t addrlen = sizeof(local);
    if (getsockname(mSocket, (struct sockaddr *)&local, &addrlen) == 0 
        && local.sin_family == AF_INET 
        && addrlen == sizeof(local))
    {
        localPort = ntohs(local.sin_port);
        localIp = inet_ntoa(local.sin_addr);
    }

    mTrace.Trace(CLINFO, "Connect socket to %s:%d succeeded (local %s:%d)", 
        aIpAddress, aPort, localIp.c_str(), localPort);
}

void
Socket::SendBytes(
    const char *    aBuf, 
    size_t          aBufSiz
    )
{
    if (aBufSiz == 0) {
        return;
    }

#ifdef CROSSBASE_API
    if (mTrace.IsThisModuleTracing(CLULTRA)) {
        // when ULTRA is enabled, log the actual data received
        std::string hex;
        const size_t MAX_LOG_SIZE = 4096;
        CrossTrace::hexdump(hex, aBuf, std::min(aBufSiz, MAX_LOG_SIZE));
        mTrace.Trace(CLULTRA, "sending %lu bytes:\n%s%s", 
            (unsigned long) aBufSiz, hex.c_str(), 
            aBufSiz > MAX_LOG_SIZE ? "\n    XXXX    *** TRUNCATED ***" : "");
    }
#endif

    // blocking send, will return the number of bytes sent or 
    // it is an error
    int rc = (int) send(mSocket, aBuf, (int) aBufSiz, MSG_NOSIGNAL);
    while (handleEINTR(rc)) {
        mTrace.Trace(CLDEBUG, "send returned EINTR, retrying");
        rc = send(mSocket, aBuf, (int) aBufSiz, MSG_NOSIGNAL);
    }
    if (rc == (int) aBufSiz) {
        return;
    }

    // on error disconnect and throw Exception
    const int errcode = GetLastSocketErrno();
    WideString sErrorMsg;
    GetLastError(errcode, sErrorMsg);
    mTrace.Trace(CLINFO, "send returned %d, errno = %d, %S", 
        rc, errcode, sErrorMsg.c_str());
    Disconnect();
    throw Exception(ERR_SEND, errcode, "send");
}

int 
Socket::ReceiveBytes(
    char *  aBuf, 
    int     aBufSiz
    ) 
{
    int rc = recv(mSocket, aBuf, aBufSiz, 0);
    while (handleEINTR(rc)) {
        mTrace.Trace(CLDEBUG, "recv returned EINTR, retrying");
        rc = recv(mSocket, aBuf, aBufSiz, 0);
    }

    if (rc <= 0) {
        // on error disconnect and throw Exception
        const int errcode = GetLastSocketErrno();
        WideString sErrorMsg;
        GetLastError(errcode, sErrorMsg);
        mTrace.Trace(CLINFO, "recv returned %d, errno = %d, %S", 
            rc, errcode, sErrorMsg.c_str());
        Disconnect();
        throw Exception(ERR_RECV, errcode, "recv");
    }

#ifdef CROSSBASE_API
    if (mTrace.IsThisModuleTracing(CLULTRA)) {
        // when ULTRA is enabled, log the actual data received
        std::string hex;
        const int MAX_LOG_SIZE = 4096;
        CrossTrace::hexdump(hex, aBuf, std::min(rc, MAX_LOG_SIZE));
        mTrace.Trace(CLULTRA, "received %d bytes:\n%s%s", 
            rc, hex.c_str(), rc > MAX_LOG_SIZE ? "\n    XXXX    *** TRUNCATED ***" : "");
    }
#endif

    return rc;
}

char 
Socket::GetByte() // throw Exception
{ 
    if (mIdx >= mBufLen) {
        mTrace.Trace(CLULTRA, "GetByte() reloading buffer");
        mIdx = 0;
        mBufLen = ReceiveBytes(mBuf, MAXBUF);
    }
    const int idx = mIdx++;
    const char ch = mBuf[idx];
    return ch;
}

int 
Socket::GetBytes(
    char *  aBuf, 
    int     aBufSiz
    ) 
{
    if (mIdx < mBufLen) {
        mTrace.Trace(CLULTRA, "GetBytes() returning fom buffer: req %d, curr idx %d, curr len %d, avail %d",
            aBufSiz, mIdx, mBufLen, mBufLen - mIdx);

        int nLen = mBufLen - mIdx;
        if (nLen > aBufSiz) {
            nLen = aBufSiz;
        }
        memcpy(aBuf, mBuf + mIdx, nLen);
        mIdx += nLen;
        if (mIdx == mBufLen) {
            mIdx = mBufLen = 0;
        }
        return nLen;
    }

    mTrace.Trace(CLULTRA, "GetBytes() receiving bytes: req %d", aBufSiz);
    return ReceiveBytes(aBuf, aBufSiz);
}

void 
Socket::DiscardBytes(
    int aCount
    ) 
{
    while (aCount > 0) {
        if (mIdx == mBufLen) {
            mIdx = 0;
            mBufLen = ReceiveBytes(mBuf, MAXBUF);
        }

        int nLen = mBufLen - mIdx;
        if (nLen > aCount) {
            mIdx += aCount;
            break;
        }

        aCount -= nLen;
        mIdx = mBufLen;
    }
}

void 
Socket::ReceiveLine(
    std::string & aLine,
    bool          aTrim
    )
{
    // get the value
    char c;
    aLine = (c = GetByte());
    while (c != '\n') {
        aLine += (c = GetByte());
    }

    // trim all space from the end
    if (aTrim) {
        while (!aLine.empty() && isspace((int)aLine[aLine.size()-1])) {
            aLine.resize(aLine.size()-1);
        }
    }
}

void
Socket::GetLastError(
    int             aError,
    WideString &    aErrorMsg
    )
{
#ifdef _WIN32
    // load wininet.dll for the error messages 
    DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    HMODULE hModule = LoadLibraryExA("wininet.dll", NULL,
        LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES);
    if (hModule) {
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    // note that we request a US English error message. This is to make the
    // message the same on servers regardless of the language that is running.
    // since US English is a fallback language for error messages it will always
    // return a valid message.
    LPWSTR pBuf = NULL;
    DWORD dwLen = FormatMessageW(
        dwFlags, hModule, (DWORD) aError, 
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
        (LPWSTR)&pBuf, 0, NULL);

    if (NULL == pBuf) {
        printf("Memcache is Close!!!\n");
        if (hModule) {
            FreeLibrary(hModule);
        }
        return;
    }
    // trim the CRLF off the end
    while (dwLen > 0 && iswspace(pBuf[dwLen-1])) {
        pBuf[--dwLen] = 0;
    }
    aErrorMsg = pBuf;
    LocalFree(pBuf);

    // free the library if we loaded it
    if (hModule) {
        FreeLibrary(hModule);
    }
#elif defined(CROSSBASE_API)
    ClGetLastError(aError, aErrorMsg);
#else
    const char * pError = strerror(aError);
    wchar_t buf[256];
    mbstowcs(buf, pError, 256);
    aErrorMsg = buf;
#endif
}

#ifdef CROSSBASE_API
END_CL_NAMESPACE
#endif
