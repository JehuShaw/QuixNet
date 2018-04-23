/*! @file       Socket.h
    @version    2.0
    @brief      Basic socket wrapper
 */

#ifndef INCLUDED_Socket
#define INCLUDED_Socket

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <vector>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

#ifdef CROSSBASE_API
# include <Core/clplatform.h>
# include <Trace/cltrace.h>
START_CL_NAMESPACE
#else
# include "Matilda.h"
#endif

/*! @brief Socket connection, disconnection, and buffered data receives. 
 */
class CROSSBASE_CLASS_API Socket
{
protected:
    const static int MAXBUF = 1024; //!< size of internal data buffer
#ifndef _WIN32
    typedef int SOCKET;
#endif

    ClTrace mTrace;             //!< trace output
    SOCKET  mSocket;            //!< socket being abstracted
    char    mBuf[MAXBUF];       //!< internal data buffer
    int     mIdx;               //!< current read index for mBuf
    int     mBufLen;            //!< current end index for mBuf

public:
    int     mConnectTimeout;    //!< timeout for connect in millisec
    int     mSendTimeout;       //!< timeout for send in millisec
    int     mRecvTimeout;       //!< timeout for recv in millisec
    int     mBufferSize;        //!< minimum socket buffer size in bytes

private:
    /*! @brief copy constructor is disabled */
    Socket(const Socket &); 

    /*! @brief copy operator is disabled */
    Socket & operator=(const Socket &); 

    void connectSocket(SOCKET aSocket, const sockaddr_in * aServer);
    void waitForConnect(SOCKET aSocket);
    void setSocketOptions(SOCKET aSocket);
    void setSocketBufferSize(SOCKET aSocket);
    void setSocketBlocking(SOCKET aSocket, bool aBlocking);
    int  setSocketTimeout(SOCKET aSocket, int aType, int aTimeout);

protected:
    /*! @brief Receive data into a buffer with as many bytes as possible up 
        to the size of the buffer.
        @param a_pszBuf     Buffer to be filled
        @param a_nBufSiz    Maximum number of bytes to receive
        @return number of bytes actually received
     */
    int ReceiveBytes(char * a_pszBuf, int a_nBufSiz);

public:
    /*! socket error category */
    enum ErrorCategory {
        ERR_OTHER = 0,
        ERR_DNS,        // connection error
        ERR_CONNECT,    // connection error
        ERR_SEND,       // comms error
        ERR_RECV,       // comms error
    };

    /*! @brief Exception thrown on any send or receive error */
    class Exception : public std::exception { 
    public:
        ErrorCategory   mType;      //!< error category
        int             mCode;      //!< raw socket error code
        const char *    mDetail;    //!< error details

        /*! constructor
            @param aWhat optional message 
         */
        Exception(ErrorCategory aType = ERR_OTHER, int aCode = 0, const char * aDetail = "") 
            : mType(aType), mCode(aCode), mDetail(aDetail) 
        { }
    };

    /*! @brief Resolve a name to an IPv4 IP address. This will return a global address
        in preference to returning a localhost loopback address. It will throw an exception
        on error, or when multiple global IP addresses are present.
        @param aHost        DNS name to resolve
        @param aBuf         Buffer to return IP address
        @param aBufSiz      Size of buffer
     */
    static bool Resolve(const char * aHost, std::vector<std::string> & aAddress); // throw Exception

    /*! return the last socket error */
    static void GetLastError(
        int             aError,
        WideString &    aErrorMsg
        );

public:
    /*! @brief constructor */
    Socket(const ClTrace & aTrace);

    /*! @brief destructor */
    ~Socket(); 

    /*! @brief connect the socket to an address
        @param a_nIpAddress         Server IP address (must be IPv4 dotted numeric)
        @param a_nPort              Server port
     */
    void Connect(const char * aIpAddress, int aPort); // throw Exception

    /*! @brief Determine if we are currently connected to a server */
    bool IsConnected() const;

    /*! @brief Disconnect from the server */
    void Disconnect();

    /*! @brief Send the supplied bytes to the server. 
    
        A blocking send is used, so the function will block until either 
        all bytes are sent or an error occurs. 
        
        @param a_pszBuf     Bytes to send to the server
        @param a_nBufSiz    Number of bytes to send to the server
        @throw Exception on socket error
     */
    void SendBytes(const char * a_pszBuf, size_t a_nBufSiz); // throw Exception

    /*! @brief Receive a block of data from the server. 
        @param a_pszBuf     Buffer to receive the data into
        @param a_nBufSiz    Maximum number of bytes to be received
        @return number of bytes that were actually received
        @throw Exception on socket error
     */
    int GetBytes(char * a_pszBuf, int a_nBufSiz); // throw Exception

    /*! @brief Receive and discard bytes.
        @param a_nBytes Number of bytes to be received and ignored.
        @throw Exception on socket error
     */
    void DiscardBytes(int a_nBytes); // throw Exception

    /*! @brief Receive a single byte
        @return received byte
        @throw Exception on socket error
     */
    char GetByte();

    /*! @brief Receive a line of text ending in \n or \r\n. 
        @param aTrim Remove all trailing whitespace from the returned data.
        @throw Exception on socket error
     */
    void ReceiveLine(std::string & a_sLine, bool aTrim); // throw Exception
};

#ifdef CROSSBASE_API
END_CL_NAMESPACE
#endif

#endif // INCLUDED_Socket
