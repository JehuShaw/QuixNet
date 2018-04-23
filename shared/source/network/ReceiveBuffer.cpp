/* 
 * File:   ReceiveBuffer.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2011.4.15, MP2:09
 */

#include "ReceiveBuffer.h"

namespace ntwk {

ReceiveBuffer::ReceiveBuffer(size_t bufsize/* = DEFAULT_RECEIVE_BUFFER_SIZE*/){
    if(bufsize < 1) {
        bufsize = 1;
    }
    this->bufferSize = bufsize;
    this->buffer = new unsigned char[bufferSize];
    if((unsigned char*)0 == this->buffer) {
        throw std::bad_alloc();
    }
    Clear();
}

ReceiveBuffer::ReceiveBuffer(const ReceiveBuffer& orig) 
    :bufferSize(orig.bufferSize)
    ,pWrite(orig.pWrite)
    ,pRead(orig.pRead)
    ,bEmpty(orig.bEmpty)
    ,bFull(orig.bFull)
    ,moreRead(orig.moreRead)
    ,nCanWrite(orig.nCanWrite)
    ,nCanRead(orig.nCanRead) {

        memcpy(buffer,orig.buffer,bufferSize);
}

ReceiveBuffer::~ReceiveBuffer() {
    delete [] this->buffer;
    this->buffer = (unsigned char*)0;
}

unsigned char*  ReceiveBuffer::GetCanWriteBuffer(size_t& size)
{
    size = 0;
    nCanWrite = 0;
    unsigned char* pStart = buffer;
    unsigned char* pEnd = buffer + bufferSize;

    if(pRead == pWrite 
        || (pWrite == pEnd && pStart == pRead)
        || (pRead == pEnd && pStart == pWrite)) {
        if(bEmpty){
			if(pWrite == pEnd) {
				pWrite = pStart;
			}
            nCanWrite = pEnd - pWrite;
            size = nCanWrite;
        } else {
			// ·ÖÅä
			size_t newBufSize = bufferSize + DEFAULT_ALLOCATE_SIZE;
			unsigned char* pNewBuf = new unsigned char[newBufSize];
			if((unsigned char*)0 == pNewBuf) {
				throw std::bad_alloc();
			}
			bool bReadMore(false);
			size_t canReadSize(0);
			unsigned char* pNewWrite = pNewBuf;
			unsigned char* pCanReadBuf = GetCanReadBuffer(canReadSize, bReadMore);
			if(canReadSize > 0){
				assert(canReadSize <= bufferSize);
				memcpy(pNewBuf, pCanReadBuf, canReadSize);
				pNewWrite += canReadSize;
			}
			if(bReadMore) {
				size_t moreReadSize = 0;
				pCanReadBuf = GetCanReadBuffer(moreReadSize, bReadMore);
				if(moreReadSize > 0) {
					assert((canReadSize + moreReadSize) <= bufferSize);
					memcpy(pNewWrite, pCanReadBuf, moreReadSize);
					pNewWrite += moreReadSize;
					// all read size
					canReadSize += moreReadSize;
				}
			}
			assert(canReadSize == bufferSize);
			// reset 
			bufferSize = newBufSize;
			pWrite = pNewWrite;
			pRead = pNewBuf;
			bEmpty = false;
			bFull = false;
			moreRead = false;
			nCanWrite = newBufSize - canReadSize;
			nCanRead = canReadSize;
			delete [] buffer;
			buffer = pNewBuf;
			// cal size
			size = nCanWrite;
		}
        return pWrite;
    }

    if(pWrite > pRead) {
        nCanWrite = pEnd - pWrite;
        if(nCanWrite > 0){
            size = nCanWrite;
            return pWrite;
        }else {
            pWrite = pStart;
            nCanWrite = pRead - pWrite;
            size = nCanWrite;
            return pWrite;
        }
    }
    nCanWrite = pRead - pWrite;
    size = nCanWrite;
    return pWrite;
}

void  ReceiveBuffer::SetWriteSize(size_t size)
{
    if(pRead == pWrite) {
        if(bFull) {
            return;
        }
    }

	if(0 == size || size > nCanWrite) {
		assert(size <= nCanWrite);
		return;
	}

    pWrite += size;

	unsigned char* pStart = buffer;
	unsigned char* pEnd = buffer + bufferSize;
	assert(pStart <= pWrite && pWrite <= pEnd);

    bEmpty = false;

    if(pWrite == pRead){
        bFull = true;
    }
}

unsigned char* ReceiveBuffer::GetCanReadBuffer(size_t& size, bool& more)
{
    size = 0;
    more = false;
    unsigned char* pStart = buffer;
    unsigned char* pEnd = buffer + bufferSize;
    
    if(moreRead){
        moreRead = false;
        if(pStart == pWrite){
            if(bFull) {
                size = pEnd - pStart;
            }
        }else if(pWrite > pStart){
            size = pWrite - pStart;
        }
        return pStart;
    }else{
        nCanRead = CalAllCanReadSize();
        //Get data
        if(pRead == pWrite){
            if(bFull) {
				if(pRead == pEnd) {
					pRead = pStart;
				}
                size = pEnd - pRead;
            }
        }else if(pWrite > pRead){
            size = pWrite - pRead;
        }else {
			if(bFull && pRead == pEnd) {
				pRead = pStart;
			}
			size = pEnd - pRead;
        }
        
        if(nCanRead > size){
            moreRead = true;
            more = moreRead;
        }
        return pRead;
    }
}

void ReceiveBuffer::SetReadSize(size_t size)
{
    if(pWrite == pRead) {
        if(bEmpty) {
            return;
        }
    }

    if(0 == size || nCanRead < size) {
		assert(nCanRead >= size);
        return;
    }
    unsigned char* pStart = buffer;
    unsigned char* pEnd = buffer + bufferSize;
    int nDif = (int)(pEnd - pRead);

    assert(nDif > -1);

    if((size_t)nDif >= size){
        pRead += size;
    }else{
        pRead = pStart + (size - nDif);
    }
    bFull = false;
    
    if(pRead == pWrite){
        bEmpty = true;
    }

}

} // end namespace ntwk
