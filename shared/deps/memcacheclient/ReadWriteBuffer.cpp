/*! @file       ReadWriteBuffer.cpp
    @version    2.0
    @brief      Byte buffer management
*/

#include <new>
#include <stdexcept>

// local
#include "ReadWriteBuffer.h"

///////////////////////////////////////////////////////////////////////////////
// ReadWriteBuffer

ReadWriteBuffer::ReadWriteBuffer()
    : mBufSiz(0)
    , mBufLen(0)
    , mBufIdx(0)
    , mGrowBy(1024)
    , mIsExt(false)
    , mBuf(NULL)
{
}

ReadWriteBuffer::ReadWriteBuffer(
    const ReadWriteBuffer & rhs
    )
    : mBufSiz(0)
    , mBufLen(0)
    , mBufIdx(0)
    , mGrowBy(1024)
    , mIsExt(false)
    , mBuf(NULL)
{
    operator=(rhs);
}

ReadWriteBuffer & 
ReadWriteBuffer::operator=(
    const ReadWriteBuffer & rhs
    )
{
    Deallocate();
    if (rhs.mBufSiz > 0) {
        SetInternalBuffer(rhs.mBufSiz, rhs.mGrowBy ? rhs.mGrowBy : 1024);
        char * pBuf = GetWriteBuffer(rhs.mBufLen);
        memcpy(pBuf, rhs.mBuf, rhs.mBufLen);
        mBufIdx = rhs.mBufIdx;
        mBufLen = rhs.mBufLen;
    }
    return *this;
}

ReadWriteBuffer::~ReadWriteBuffer()
{
    Deallocate();
}

void
ReadWriteBuffer::Deallocate()
{
    if (mBuf && !mIsExt) {
        free(mBuf);
    }

    mBufSiz = 0;
    mBufIdx = 0;
    mBufLen = 0;
    mGrowBy = 1024;
    mIsExt  = false;
    mBuf    = NULL;
}

void
ReadWriteBuffer::SetEmpty()
{
    mBufIdx = 0;
    mBufLen = 0;
}

void 
ReadWriteBuffer::SetExternalBuffer(
    void *  a_pBuf, 
    size_t  a_nBufSiz, 
    size_t  a_nBufLen,
    size_t  a_nGrowBy
    )
{
    if (!a_pBuf) throw std::invalid_argument("a_pBuf");
    if (a_nBufSiz < 1) throw std::invalid_argument("a_nBufSiz");
    if (a_nBufLen > a_nBufSiz) throw std::invalid_argument("a_nBufLen");

    Deallocate();

    mBufSiz = a_nBufSiz;
    mBufIdx = 0;
    mBufLen = a_nBufLen;
    mGrowBy = a_nGrowBy; 
    mIsExt  = true;
    mBuf    = (char *) a_pBuf;
}

/*! round a value up to the nearest multiple of the block 
    @param nValue   value to be rounded up
    @param nBlock   block size to round the value to
 */
static inline size_t roundup(size_t nValue, size_t nBlock) {
    if ((nValue % nBlock) != 0) {
        nValue = (nValue / nBlock) * nBlock + nBlock;
    }
    return nValue;
}

void 
ReadWriteBuffer::SetInternalBuffer(
    size_t a_nInitialSize, 
    size_t a_nGrowBy
    )
{
    if (a_nInitialSize < 1) throw std::invalid_argument("a_nInitialSize");
    if (a_nGrowBy < 1) throw std::invalid_argument("a_nGrowBy");

    Deallocate();

    // internal buffer
    a_nInitialSize = roundup(a_nInitialSize, a_nGrowBy);
    mBuf = (char *) malloc(a_nInitialSize);
    if (!mBuf) throw std::bad_alloc();
    mBufSiz = a_nInitialSize;
    mGrowBy = a_nGrowBy;
}

char * 
ReadWriteBuffer::GetWriteBuffer(
    size_t  a_nMinBytes
    )
{
    if (mBufLen + a_nMinBytes > mBufSiz) {
        if (mIsExt) {
            if (0 == mGrowBy) throw std::overflow_error("external buffer full");

            size_t nNewSiz = mBufSiz + roundup(a_nMinBytes, mGrowBy);
            char * pBuf = (char *) malloc(nNewSiz);
            if (!pBuf) throw std::bad_alloc();

            memcpy(pBuf, mBuf, mBufLen);
            mBuf    = pBuf;
            mBufSiz = nNewSiz;
            mIsExt  = false;
        }
        else {
            size_t nNewSiz = mBufSiz + roundup(a_nMinBytes, mGrowBy);
            char * pBuf = (char *) realloc(mBuf, nNewSiz);
            if (!pBuf) throw std::bad_alloc();
            mBuf    = pBuf;
            mBufSiz = nNewSiz;
        }
    }
    return mBuf + mBufLen;
}

void 
ReadWriteBuffer::CommitWriteBytes(
    size_t  a_nBytes
    )
{
    if (mBufLen + a_nBytes > mBufSiz) throw std::invalid_argument("a_nBytes");
    mBufLen += a_nBytes;
}

size_t 
ReadWriteBuffer::GetWriteSize() const
{
    return mBufSiz - mBufLen;
}

void 
ReadWriteBuffer::WriteBytes(
    const void *    a_pBuf, 
    size_t          a_nBufLen
    )
{
    char * pBuf = GetWriteBuffer(a_nBufLen);
    memcpy(pBuf, a_pBuf, a_nBufLen);
    CommitWriteBytes(a_nBufLen);
}

const char * 
ReadWriteBuffer::GetReadBuffer() const
{
    return mBuf + mBufIdx;
}

void 
ReadWriteBuffer::CommitReadBytes(
    size_t  a_nBytes
    )
{
    if (mBufIdx + a_nBytes > mBufLen) throw std::invalid_argument("a_nBytes");
    mBufIdx += a_nBytes;
}

size_t 
ReadWriteBuffer::GetReadSize() const
{
    return mBufLen - mBufIdx;
}

void 
ReadWriteBuffer::ReadBytes(
    void * a_pBuf, 
    size_t a_nBufLen
    )
{
    if (GetReadSize() < a_nBufLen) {
        throw std::invalid_argument("a_nBufLen");
    }

    const char * pBuf = GetReadBuffer();
    memcpy(a_pBuf, pBuf, a_nBufLen);
    CommitReadBytes(a_nBufLen);
}

void
ReadWriteBuffer::Compact()
{
    if (GetReadSize() > 0) {
        memmove(mBuf, GetReadBuffer(), GetReadSize());
    }

    mBufLen -= mBufIdx;
    mBufIdx  = 0;
}

bool 
ReadWriteBuffer::operator==(
    const ReadWriteBuffer & rhs
    ) const
{
    if (GetReadSize() != rhs.GetReadSize()) {
        return false;
    }
    return 0 == memcmp(GetReadBuffer(), rhs.GetReadBuffer(), GetReadSize());
}
