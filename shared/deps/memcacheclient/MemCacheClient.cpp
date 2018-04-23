/*! @file       MemCacheClient.cpp
    @version    2.0
    @brief      Basic memcached client
 */
//#include "stdafx.h"

#include <algorithm>

#ifdef _WIN32
# include <winsock2.h>
# define strtoull _strtoui64
#else
# include <stdint.h>
#endif

// If OpenSSL is available, better to use it
//#include <openssl/sha.h>
#include "sha1.h"
#ifndef HEADER_SHA_H //openssl
#define SHA_DIGEST_LENGTH   SHA1_DIGEST_LENGTH
void SHA1(const unsigned char *d, size_t n, unsigned char *md) {
    SHA1((sha1_byte*)md, (const sha1_byte*)d, (unsigned int)n);
}
#endif

// local
#include "Socket.h"
#include "MemCacheClient.h"

// lib
#ifdef CROSSBASE_API
# include <xplatform/timer.h>
# include <Trace/cltrace.h>
START_CL_NAMESPACE
#else
# include "Matilda.h"
#endif

///////////////////////////////////////////////////////////////////////////////

/*! @brief Abstraction of a server connection */
class MemCacheClient::Server : public Socket
{
public:
    /*! @brief constructor */
    Server(const ClTrace & aTrace) 
        : Socket(aTrace)
        , mIp(INADDR_NONE)
        , mPort(0)
        , mLastConnect(0) 
    { 
        mAddress[0] = 0; 
    }
    
    /*! @brief copy constructor 
        @param rhs server to copy address of
     */
    Server(const Server & rhs) : Socket(rhs.mTrace) { operator=(rhs); }
    
    /*! @brief destructor */
    ~Server() { }

    /*! @brief copy operator
        @param rhs server to copy server listen address from
     */
    Server & operator=(const Server & rhs);
    
    /*! @brief equality based on server listen address 
        @param rhs server to compare to
        @return true when servers are using the same listen address
     */
    bool operator==(const Server & rhs) const;
    
    /*! @brief inequality based on server listen address 
        @param rhs server to compare to
        @return true when servers are not using the same listen address
     */
    inline bool operator!=(const Server & rhs) const { return !operator==(rhs); }
    
    /*! @brief Set the listen address for this server 
        @param aServer Listen address in IP:PORT format
        @return true if listen address was correctly parsed
     */
    bool Set(const char * aServer); 
    
    enum ConnectResult { CONNECT_SUCCESS, CONNECT_FAILED, CONNECT_WAITING };

    /*! @brief Attempt to connect to the server.
        @param aTimeout Timeout for connection in milliseconds 
        @return true if connection was successful
     */
    ConnectResult Connect(size_t aTimeout, size_t aRetryPeriod);
    
    /*! @brief Get the string representation of this server listen address.
    
        This may not be the same as the address passed in. It will always be
        in the format IP:PORT.
        
        @return Server listen address as IP:PORT
     */
    inline const char * GetAddress() const { return mAddress; }

    /*! @brief Get the listen port for this server. */
    inline int GetPort() const { return mPort; }

private:
    /*! @brief max length of a server address */
    const static size_t ADDRLEN = sizeof("aaa.bbb.ccc.ddd:PPPPP");

    char            mAddress[ADDRLEN];  //!< server IP:PORT as string 
    unsigned long   mIp;                //!< server IP address
    int             mPort;              //!< server port
    unsigned long   mLastConnect;       //!< last connection time
};

MemCacheClient::Server & 
MemCacheClient::Server::operator=(
    const Server & rhs
    ) 
{
    if (this != &rhs) {
        mTrace = rhs.mTrace;
        strcpy(mAddress, rhs.mAddress);
        mIp   = rhs.mIp;
        mPort = rhs.mPort;
        mLastConnect = 0;
    }
    return *this;
}

bool 
MemCacheClient::Server::operator==(
    const Server & rhs
    ) const
{
    return mIp == rhs.mIp && mPort == rhs.mPort;
}

bool 
MemCacheClient::Server::Set(
    const char * aServer
    ) 
{
    if (!aServer || !*aServer) return false;

    char server[200];
    size_t nLen = strlen(aServer);
    if (nLen >= sizeof(server)) return false; 
    strcpy(server, aServer);

    mPort = 11211;
    char * pszPort = strchr(server, ':');
    if (pszPort) {
        mPort = atoi(pszPort + 1);
        *pszPort = 0;
    }

    mIp = inet_addr(server);
    if (mIp == INADDR_NONE) return false;

    struct in_addr addr;
    addr.s_addr = mIp;
    snprintf(mAddress, ADDRLEN, "%s:%d", inet_ntoa(addr), mPort);

    return true;
}

MemCacheClient::Server::ConnectResult
MemCacheClient::Server::Connect(
    size_t aTimeout,
    size_t aRetryPeriod
    ) 
{
    // already connected? do nothing
    if (Socket::IsConnected()) {
        return CONNECT_SUCCESS;
    }

    struct in_addr addr;
    addr.s_addr = mIp;
    const char * pszAddress = inet_ntoa(addr);

    // only try to re-connect to a broken server occasionally if it is optional. 
    // a required server will be attempted every time.
    unsigned long nNow = xplatform::GetCurrentTickCount();
    if (mLastConnect && (nNow - mLastConnect) < aRetryPeriod) {
        mTrace.Trace(CLDEBUG, "Connection attempt to %s:%d ignored (last failed attempt %lu seconds ago)",
            pszAddress, mPort, (nNow - mLastConnect) / 1000);
        return CONNECT_WAITING;
    }
    mLastConnect = nNow;

    try {
        // use a decent size socket buffer 
        mBufferSize = 32 * 1024;
        mConnectTimeout = (int) aTimeout;
        mSendTimeout = (int) aTimeout;
        mRecvTimeout = (int) aTimeout;
        Socket::Connect(pszAddress, mPort);
    }
    catch (const Socket::Exception &) { 
        // message already logged
        return CONNECT_FAILED;
    }

    return CONNECT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// ConsistentHash
//

bool MemCacheClient::ConsistentHash::operator<(const MemCacheClient::ConsistentHash & rhs) const 
{ 
    if (mHash != rhs.mHash) {
        return mHash < rhs.mHash; 
    }

    // in case we get multiple servers with the same hash, compare the actual server
    // addresses to get a consistent ordering
    if (mServer != rhs.mServer) {
        return strcmp(mServer->GetAddress(), rhs.mServer->GetAddress()) < 0;
    }

    return mEntry == rhs.mEntry;
}

/*! Match a server by comparing pointers */
struct MemCacheClient::ConsistentHash::MatchServer
{
    /*! server being searched for */
    MemCacheClient::Server * mServer; 

    /*! constructor 
        @param aServer server to search for
     */
    MatchServer(MemCacheClient::Server * aServer) : mServer(aServer) { }

    /*! compare current entry against search entry 
        @param rhs  entry to compare
     */
    bool operator()(const ConsistentHash & rhs) const { return rhs.mServer == mServer; }
};

///////////////////////////////////////////////////////////////////////////////
// MemCacheClient

MemCacheClient::MemCacheClient()
    : mTrace("MEMCACHE")
    , mTimeoutMs(1000)
    , mRetryMs(300 * 1000)
{
}

MemCacheClient::~MemCacheClient()
{
    ClearServers();
}

void
MemCacheClient::ClearServers()
{
    for (size_t n = 0; n < mServer.size(); ++n) {
        delete mServer[n];
    }
    mServer.clear();
}

const char * 
MemCacheClient::ConvertResult(
    MCResult aResult
    ) 
{
    switch (aResult) {
    case MCERR_OK:        return "MCERR_OK";
    case MCERR_NOREPLY:   return "MCERR_NOREPLY";
    case MCERR_NOTSTORED: return "MCERR_NOTSTORED";
    case MCERR_NOTFOUND:  return "MCERR_NOTFOUND";
    case MCERR_NOSERVER:  return "MCERR_NOSERVER";
    default:              return "(unknown)";
    }
}

bool 
MemCacheClient::AddServer(
    const char *    aServerAddress,
    const char *    aServerName,
    unsigned        aServices
    )
{
    if (!aServerName) {
        aServerName = aServerAddress;
    }

    // if we the server address is valid then we allow the server 
    // to be added. All servers being added are assumed to be available
    // or to be soon made available. 
    Server * pServer = new Server(mTrace);
    if (!pServer->Set(aServerAddress)) {
        mTrace.Trace(CLERROR, "Ignoring invalid server: %s (%s)", 
            aServerAddress, aServerName);
        delete pServer;
        return false;
    }
    for (size_t n = 0; n < mServer.size(); ++n) {
        if (*pServer == *mServer[n]) {
            mTrace.Trace(CLERROR, "Ignoring duplicate server: %s (%s)", 
                aServerAddress, aServerName);
            return true; // already have it
        }
    }
    mServer.push_back(pServer);

    // for each salt we generate a string hash for the consistent hash 
    // table. To ensure stability of the hashing for multiple servers, 
    // we want to have a number of entries for each server. 
    static const char * rgpSalt[] = {
        "{DEA60AAB-CFF9-4a20-A799-4E5E93369656}",
        "{C05167CC-57DA-40f2-9EB8-18F65E56FD21}",
        "{57939537-0966-49e7-B675-ACE63246BFA5}",
        "{F0C8BE5C-A0F1-478f-BC45-28D42AF0CA1E}"
    };

    string_t sKey;
    ConsistentHash entry(0, pServer, aServices, 0);
    for (size_t n = 0; n < sizeof(rgpSalt)/sizeof(rgpSalt[0]); ++n) {
        sKey  = pServer->GetAddress();
        sKey += rgpSalt[n];
        entry.mEntry++;
        entry.mHash = CreateKeyHash(sKey.data());
        mServerHash.push_back(entry);
    }

    // sort the vector so that we can binary search it
    std::sort(mServerHash.begin(), mServerHash.end());

    mTrace.Trace(CLINFO, "Adding server: %s (%s:%u), services: 0x%x",
        aServerAddress, aServerName, pServer->GetPort(), aServices);
    return true;
}

void
MemCacheClient::DumpTables()
{
    // we need this information to ensure that different servers are
    // using the same consistent hashing tables.
    if (!mTrace.IsThisModuleTracing(CLDEBUG)) {
        return;
    }

    std::string verify;
    char buf[200];
    mTrace.Trace(CLDEBUG, "Consistent Hash Server Ring (%u entries):", mServerHash.size());
    for (size_t n = 0; n < mServerHash.size(); ++n) {
        const ConsistentHash & server = mServerHash[n];
        mTrace.Trace(CLDEBUG, "%2u: %08lx = %s (services: 0x%x, entry: %d)", 
            n, server.mHash, server.mServer->GetAddress(), server.mServices, server.mEntry);
        snprintf(buf, sizeof(buf), "%s>%d>%x>%lx>", server.mServer->GetAddress(), 
            server.mEntry, server.mServices, server.mHash);
        verify += buf;
    }

    mTrace.Trace(CLDEBUG, "Data verification code: %lx", CreateKeyHash(verify.c_str()));
}

bool 
MemCacheClient::DelServer(
    const char * aServer
    )
{
    Server test(mTrace);
    if (test.Set(aServer)) {
        std::vector<Server*>::iterator i = mServer.begin();
        for (; i != mServer.end(); ++i) {
            Server * pServer = *i;
            if (test != *pServer) continue;

            delete pServer;
            mServer.erase(i);
            ConsistentHash::MatchServer server(pServer);
            mServerHash.erase(
                std::partition(mServerHash.begin(), mServerHash.end(), server), 
                mServerHash.end());
            std::sort(mServerHash.begin(), mServerHash.end());
            return true;
        }
    }

    // not found
    return false;
}

void 
MemCacheClient::GetServers(
    std::vector<string_t> & aServers
    )
{
    string_t address;
    aServers.clear();
    aServers.reserve(mServer.size());
    for (size_t n = 0; n < mServer.size(); ++n) {
        address = mServer[n]->GetAddress();
        aServers.push_back(address);
    }
}

void 
MemCacheClient::SetTimeout(
    size_t aTimeoutMs
    )
{
    mTimeoutMs = aTimeoutMs;
}

void 
MemCacheClient::SetRetryPeriod(
    size_t aRetryMs
    )
{
    mRetryMs = aRetryMs;
}

unsigned long 
MemCacheClient::CreateKeyHash(
    const char * aKey
    )
{
    const size_t LONG_COUNT = SHA_DIGEST_LENGTH / sizeof(unsigned long);
    
    union {
        unsigned char as_char[SHA_DIGEST_LENGTH];
        unsigned long as_long[LONG_COUNT];
    } output;

    CR_ASSERT(sizeof(output.as_char) == SHA_DIGEST_LENGTH);
    CR_ASSERT(sizeof(output.as_long) == SHA_DIGEST_LENGTH);

    SHA1((const unsigned char *) aKey, (unsigned long) strlen(aKey), output.as_char);
    return output.as_long[LONG_COUNT-1];
}

MemCacheClient::Server *
MemCacheClient::FindServer(
    const string_t & aKey,
    unsigned         aService
    )
{
#ifdef CROSSBASE_API
    // in our private usage of this, the service must never be 0
    if (aService == 0) {
        mTrace.Trace(CLERROR, "FindServer: no service requested, supplied cache server may not be appropriate!!!");
        CR_ASSERT(!"FindServer: no service requested, supplied cache server may not be appropriate!!!");
    }
#endif

    // probably need some servers for this
    if (mServerHash.empty()) {
        //mTrace.Trace(CLDEBUG, "FindServer: server hash is empty");
        return NULL;
    }

    // find the next largest consistent hash value above this key hash
    ConsistentHash hash(CreateKeyHash(aKey.data()), NULL, 0, 0);
    std::vector<ConsistentHash>::iterator iBegin = mServerHash.begin();
    std::vector<ConsistentHash>::iterator iEnd = mServerHash.end();
    std::vector<ConsistentHash>::iterator iCurr = std::lower_bound(iBegin, iEnd, hash);
    if (iCurr == iEnd) iCurr = iBegin;

    // now find the next server that handles this service
    if (aService != 0) {
        //int nSkipped = 0;
        std::vector<ConsistentHash>::iterator iStart = iCurr;
        while (!iCurr->services(aService)) {
            //++nSkipped;
            ++iCurr; 
            if (iCurr == iEnd) iCurr = iBegin;
            if (iCurr == iStart) {
                mTrace.Trace(CLDEBUG, "FindServer: no server for required service: %u", aService);
                return NULL;
            }
        }
        //if (nSkipped > 0) mTrace.Trace(CLDEBUG, "skipped %d servers for service: %u", nSkipped, aService);
    }

    // ensure that this server is connected 
    Server * pServer = iCurr->mServer;
    Server::ConnectResult rc = pServer->Connect(mTimeoutMs, mRetryMs);
    switch (rc) {
    case Server::CONNECT_SUCCESS:
        //mTrace.Trace(CLDEBUG, "FindServer: using server %s", pServer->GetAddress());
        return pServer;
    case Server::CONNECT_WAITING:
        return NULL;
    default:
    case Server::CONNECT_FAILED:
        //mTrace.Trace(CLDEBUG, "FindServer: failed to connect to server %s", pServer->GetAddress());
        return NULL;
    }
}

/*! @brief Sort the requests into server order */
struct MemCacheClient::MemRequest::Sort 
{ 
    /*! @brief Compare two requests for less ordering
        @param pl Request item 
        @param pr Request item 
        @return true if pl is less than pr.
     */
    bool operator()(const MemRequest * pl, const MemRequest * pr) const {
        return pl->mServer < pr->mServer; // any order is fine
    }
}; 

int 
MemCacheClient::Combine(
    const char *    aType,
    MemRequest *    aItem, 
    int             aCount
    )
{
    if (aCount < 1) {
        mTrace.Trace(CLDEBUG, "%s: ignoring request for %d items",
            aType, aCount);
        return 0;
    }
    CR_ASSERT(*aType == 'g' || *aType == 'd'); // get, gets, del

    MemRequest * rgpItem[MAX_REQUESTS] = { NULL };
    if (aCount > MAX_REQUESTS) {
        mTrace.Trace(CLDEBUG, "%s: ignoring request for all %d items (too many)", 
            aType, aCount);
        return -1; // invalid args
    }

    // initialize and find all of the servers for these items
    int nItemCount = 0;
    for (int n = 0; n < aCount; ++n) {
        // ensure that the key doesn't have a space in it
        CR_ASSERT(NULL == strchr(aItem[n].mKey.data(), ' '));
        aItem[n].mServer = FindServer(aItem[n].mKey, aItem[n].mService);
        aItem[n].mData.SetEmpty();
        if (aItem[n].mServer) {
            rgpItem[nItemCount++] = &aItem[n];
        }
        else {
            aItem[n].mResult = MCERR_NOSERVER;
        }
    }
    if (nItemCount == 0) {
        mTrace.Trace(CLDEBUG, "%s: ignoring request for all %d items (no servers available)", 
            aType, aCount);
        return 0;
    }

    // sort all requests into server order
    const static MemRequest::Sort sortOnServer = MemRequest::Sort();
    std::sort(&rgpItem[0], &rgpItem[nItemCount], sortOnServer);

    // send all requests
    char szBuf[50];
    int nItem = 0, nNext;
    string_t sRequest, sTemp;
    while (nItem < nItemCount) {
        for (nNext = nItem; nNext < nItemCount; ++nNext) {
            if (rgpItem[nItem]->mServer != rgpItem[nNext]->mServer) break;
            CR_ASSERT(*aType == 'g' || *aType == 'd');
            rgpItem[nNext]->mData.SetEmpty();

            // create get request for all keys on this server
            if (*aType == 'g') {
                if (nNext == nItem) sRequest = "get";
                else sRequest.resize(sRequest.length() - 2);
                sRequest += ' ';
                sRequest += rgpItem[nNext]->mKey;
                sRequest += "\r\n";
                rgpItem[nNext]->mResult = MCERR_NOTFOUND;
            }
            // create del request for all keys on this server
            else if (*aType == 'd') {
                // delete <key> [<time>] [noreply]\r\n
                sRequest += "delete ";
                sRequest += rgpItem[nNext]->mKey;
                sRequest += ' ';
                snprintf(szBuf, sizeof(szBuf), "%ld", (long) rgpItem[nNext]->mExpiry);
                sRequest += szBuf;
                if (rgpItem[nNext]->mResult == MCERR_NOREPLY) {
                    sRequest += " noreply";
                }
                sRequest += "\r\n";
                if (rgpItem[nNext]->mResult != MCERR_NOREPLY) {
                    rgpItem[nNext]->mResult = MCERR_NOTFOUND;
                }
            }
        }

        // send the request. any socket error causes the server connection 
        // to be dropped, so we return errors for all requests using that server.
        try {
            rgpItem[nItem]->mServer->SendBytes(
                sRequest.data(), sRequest.length());
        }
        catch (const Socket::Exception & e) {
            mTrace.Trace(CLINFO, "%s: request error '%s' at %s, marking requests as NOSERVER",
                aType, e.mDetail, rgpItem[nItem]->mServer->GetAddress());
            for (int n = nItem; n < nNext; ++n) {
                rgpItem[n]->mServer = NULL;
                rgpItem[n]->mResult = MCERR_NOSERVER;
            }
        }
        nItem = nNext;
    }

    // receive responses from all servers
    int nResponses = 0;
    for (nItem = 0; nItem < nItemCount; nItem = nNext) {
        // find the end of this server
        if (!rgpItem[nItem]->mServer) { nNext = nItem + 1; continue; }
        for (nNext = nItem + 1; nNext < nItemCount; ++nNext) {
            if (rgpItem[nItem]->mServer != rgpItem[nNext]->mServer) break;
        }

        // receive the responses. any socket error causes the server connection 
        // to be dropped, so we return errors for all requests using that server.
        try {
            if (*aType == 'g') {
                nResponses += HandleGetResponse(
                    rgpItem[nItem]->mServer, 
                    &rgpItem[nItem], &rgpItem[nNext]);
            }
            else if (*aType == 'd') {
                nResponses += HandleDelResponse(
                    rgpItem[nItem]->mServer, 
                    &rgpItem[nItem], &rgpItem[nNext]);
            }
        }
        catch (const Socket::Exception & e) {
            mTrace.Trace(CLINFO, "%s: response error '%s' at %s, marking requests as NOSERVER",
                aType, e.mDetail, rgpItem[nItem]->mServer->GetAddress());
            rgpItem[nItem]->mServer->Disconnect();
            for (int n = nNext - 1; n >= nItem; --n) {
                if (rgpItem[nItem]->mServer != rgpItem[n]->mServer) continue;
                rgpItem[n]->mServer = NULL;
                rgpItem[n]->mResult = MCERR_NOSERVER;
            }
        }
    }

    mTrace.Trace(CLDEBUG, "%s: received %d responses to %d requests",
        aType, nResponses, aCount);
    return nResponses;
}

int 
MemCacheClient::HandleGetResponse(
    Server *        aServer, 
    MemRequest **   aBegin, 
    MemRequest **   aEnd
    )
{
    int nFound = 0;

    std::string sValue;
    for (;;) {
        // get the value
        aServer->ReceiveLine(sValue, false);
        if (sValue == "END\r\n") break;

        // if it isn't a value then we are in a bad state
        if (0 != strncmp(sValue.data(), "VALUE ", 6)) {
            throw Socket::Exception(Socket::ERR_OTHER, 0, "bad get response at VALUE");
        }

        // extract the key
        int n = (int) sValue.find(' ', 6);
        if (n < 1) throw Socket::Exception(Socket::ERR_OTHER, 0, "bad get response at key");
        std::string sKey(sValue, 6, n - 6);

        // extract the flags
        const char * pVal = sValue.data() + n + 1;
        unsigned nFlags = (unsigned) strtoul(pVal, (char**) &pVal, 10);
        if (*pVal++ != ' ') throw Socket::Exception(Socket::ERR_OTHER, 0, "bad get response at flags");

        // extract the size
        unsigned nBytes = (unsigned) strtoul(pVal, (char**) &pVal, 10);
        if (*pVal != ' ' && *pVal != '\r') throw Socket::Exception(Socket::ERR_OTHER, 0, "bad get response at size");

        // find this key in the array
        MemRequest * pItem = NULL; 
        for (MemRequest ** p = aBegin; p < aEnd; ++p) {
            if ((*p)->mKey == sKey.data()) { pItem = *p; break; }
        }
        if (!pItem) { // key not found, discard the response
            aServer->DiscardBytes(nBytes + 2); // +2 == include final "\r\n"
            continue;
        }
        pItem->mFlags = nFlags;

        // extract the cas
        if (*pVal == ' ') {
            char * last = NULL;
            pItem->mCas = strtoull(++pVal, &last, 10);
            if (*last != '\r') throw Socket::Exception(Socket::ERR_OTHER, 0, "bad get response at CAS");
        }

        // receive the data
        while (nBytes > 0) {
            char * pBuf = pItem->mData.GetWriteBuffer(nBytes);
            int nReceived = aServer->GetBytes(pBuf, nBytes);
            pItem->mData.CommitWriteBytes(nReceived);
            nBytes -= nReceived;
        }
        pItem->mResult = MCERR_OK;

        // discard the trailing "\r\n"
        if ('\r' != aServer->GetByte() ||
            '\n' != aServer->GetByte())
        {
            throw Socket::Exception(Socket::ERR_OTHER, 0, "bad get response at trail");
        }

        ++nFound;
    }

    return nFound;
}

int 
MemCacheClient::HandleDelResponse(
    Server *        aServer, 
    MemRequest **   aBegin, 
    MemRequest **   aEnd
    )
{
    std::string sValue;
    int nResponses = 0;
    for (MemRequest ** p = aBegin; p < aEnd; ++p) {
        MemRequest * pItem = *p; 

        // no response for this entry
        if (pItem->mResult == MCERR_NOREPLY) continue;

        // get the value
        aServer->ReceiveLine(sValue, false);

        // success
        if (sValue == "DELETED\r\n") {
            pItem->mResult = MCERR_OK;
            ++nResponses;
            continue;
        }

        // the item with this key was not found
        if (sValue == "NOT_FOUND\r\n") {
            pItem->mResult = MCERR_NOTFOUND;
            ++nResponses;
            continue;
        }

        aServer->Disconnect();
        throw Socket::Exception(Socket::ERR_OTHER, 0, "bad del response");
    }

    return nResponses;
}

MCResult 
MemCacheClient::IncDec(
    const char *    aType, 
    unsigned        aService,
    const char *    aKey, 
    uint64_t *      aNewValue,
    uint64_t        aDiff,
    bool            aWantReply
    )
{
    string_t key(aKey);
    Server * pServer = FindServer(key, aService);
    if (!pServer) return MCERR_NOSERVER;

    char szBuf[50];
    string_t sRequest(aType);
    sRequest += ' ';
    sRequest += aKey;
    snprintf(szBuf, sizeof(szBuf), " %" PRIu64, aDiff);
    sRequest += szBuf;
    if (!aWantReply) {
        sRequest += " noreply";
    }
    sRequest += "\r\n";

    try {
        pServer->SendBytes(sRequest.data(), sRequest.length());

        if (!aWantReply) {
            return MCERR_NOREPLY;
        }

        string_t sValue;
        sValue = pServer->GetByte();
        while (sValue[sValue.length()-1] != '\n') {
            sValue += pServer->GetByte();
        }

        if (sValue == "NOT_FOUND\r\n") {
            return MCERR_NOTFOUND;
        }

        if (aNewValue) {
            *aNewValue = strtoull(sValue.data(), NULL, 10);
        }
        return MCERR_OK;
    }
    catch (const Socket::Exception & e) {
        mTrace.Trace(CLINFO, "IncDec: error '%s' at %s, marking request as NOSERVER",
            e.mDetail, pServer->GetAddress());
        pServer->Disconnect();
        return MCERR_NOSERVER;
    }
}

int 
MemCacheClient::Store(
    const char *    aType,
    MemRequest *    aItem, 
    int             aCount
    )
{
    if (aCount < 1) {
        mTrace.Trace(CLDEBUG, "Store: ignoring request for %d items", aCount);
        return 0;
    }

    // initialize and find all of the servers for these items
    int nItemCount = 0;
    for (int n = 0; n < aCount; ++n) {
        // ensure that the key doesn't have a space in it
        CR_ASSERT(NULL == strchr(aItem[n].mKey.data(), ' '));
        aItem[n].mServer = FindServer(aItem[n].mKey, aItem[n].mService);
        if (aItem[n].mServer) {
            ++nItemCount;
        }
        else {
            aItem[n].mResult = MCERR_NOSERVER;
        }
    }
    if (nItemCount == 0) {
        mTrace.Trace(CLDEBUG, "Store: ignoring request for all %d items (no servers available)", 
            aCount);
        return 0;
    }

    char szBuf[50];
    int nResponses = 0;
    string_t sRequest;
    for (int n = 0; n < aCount; ++n) {
        if (!aItem[n].mServer) continue;

        // <command name> <key> <flags> <exptime> <bytes> [noreply]\r\n
        sRequest  = aType;
        sRequest += ' ';
        sRequest += aItem[n].mKey;
        snprintf(szBuf, sizeof(szBuf), " %u %ld %u", 
            aItem[n].mFlags, (long) aItem[n].mExpiry, 
            (unsigned)aItem[n].mData.GetReadSize());
        sRequest += szBuf;
        if (*aType == 'c') { // cas
            snprintf(szBuf, sizeof(szBuf), " %" PRIu64, aItem[n].mCas);
            sRequest += szBuf;
        }
        if (aItem[n].mResult == MCERR_NOREPLY) {
            sRequest += " noreply";
        }
        sRequest += "\r\n";

        // send the request. any socket error causes the server connection 
        // to be dropped, so we return errors for all requests using that server.
        try {
            aItem[n].mServer->SendBytes(
                sRequest.data(), sRequest.length());
            aItem[n].mServer->SendBytes(
                aItem[n].mData.GetReadBuffer(), 
                aItem[n].mData.GetReadSize());
            aItem[n].mServer->SendBytes("\r\n", 2);

            // done with these read bytes
            aItem[n].mData.CommitReadBytes(
                aItem[n].mData.GetReadSize());

            // if no reply is required then move on to the next request
            if (aItem[n].mResult == MCERR_NOREPLY) {
                continue;
            }

            // handle this response
            HandleStoreResponse(aItem[n].mServer, aItem[n]);
            ++nResponses;
        }
        catch (const Socket::Exception & e) {
            mTrace.Trace(CLINFO, "Store: error '%s' at %s, marking requests as NOSERVER",
                e.mDetail, aItem[n].mServer->GetAddress());
            for (int i = aCount - 1; i >= n; --i) {
                if (aItem[n].mServer != aItem[i].mServer) continue;
                aItem[i].mServer = NULL;
                aItem[i].mResult = MCERR_NOSERVER;
            }
            continue;
        }
    }

    return nResponses;
}

void
MemCacheClient::HandleStoreResponse(
    Server *        aServer, 
    MemRequest &    aItem
    )
{
    // get the value
    std::string sValue;
    aServer->ReceiveLine(sValue, false);

    // success
    if (sValue == "STORED\r\n") {
        aItem.mResult = MCERR_OK;
        return;
    }

    // data was not stored, but not because of an error. 
    // This normally means that either that the condition for 
    // an "add" or a "replace" command wasn't met, or that the
    // item is in a delete queue.
    if (sValue == "NOT_STORED\r\n") {
        aItem.mResult = MCERR_NOTSTORED;
        return;
    }

    // data was not stored, perhaps the key was too long?
    if (sValue == "ERROR\r\n") {
        aItem.mResult = MCERR_NOTSTORED;
        return;
    }

    // unknown response, connection may be bad
    aServer->Disconnect();
    throw Socket::Exception(Socket::ERR_OTHER, 0, "bad store response");
}

int
MemCacheClient::FlushAll(
    const char *    aServer, 
    int             aExpiry
    )
{
    char szRequest[50];
    snprintf(szRequest, sizeof(szRequest), 
        "flush_all %u\r\n", aExpiry);

    Server test(mTrace);
    if (aServer && !test.Set(aServer)) {
        return false;
    }

    int nSuccess = 0;
    for (size_t n = 0; n < mServer.size(); ++n) {
        Server * pServer = mServer[n];
        if (aServer && *pServer != test) continue;
    
        // ensure that we are connected
        if (pServer->Connect(mTimeoutMs, mRetryMs) != Server::CONNECT_SUCCESS) {
            continue;
        }

        try {
            // request
            pServer->SendBytes(szRequest, strlen(szRequest));

            // response
            string_t sValue;
            sValue = pServer->GetByte();
            while (sValue[sValue.length()-1] != '\n') {
                sValue += pServer->GetByte();
            }
            if (sValue == "OK\r\n") {
                // done
                ++nSuccess;
            }
            else {
                // unknown response, connection may be bad
                pServer->Disconnect();
            }
        }
        catch (const Socket::Exception &) {
            mTrace.Trace(CLINFO, "socket error, ignoring flush request");
            // data error
        }
    }

    return nSuccess;
}

#ifdef CROSSBASE_API
END_CL_NAMESPACE
#endif
