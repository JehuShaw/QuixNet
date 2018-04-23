/* 
 * File:   ReceiveBuffer.h
 * Author: Jehu Shaw
 *
 * Created on 2011.4.15, 2:09
 */

#ifndef RECEIVEBUFFER_H
#define	RECEIVEBUFFER_H

#include "Common.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdexcept>

namespace ntwk
{

#ifndef DEFAULT_RECEIVE_BUFFER_SIZE
#define DEFAULT_RECEIVE_BUFFER_SIZE 1024
#endif

#ifndef DEFAULT_ALLOCATE_SIZE
#define DEFAULT_ALLOCATE_SIZE 512
#endif

    class SHARED_DLL_DECL ReceiveBuffer {
    public:

        ReceiveBuffer(size_t bufsize = DEFAULT_RECEIVE_BUFFER_SIZE);

        ReceiveBuffer(const ReceiveBuffer& orig);

        ~ReceiveBuffer();

        //get empty buffer size (byte)
        unsigned char*  GetCanWriteBuffer(size_t& size);
        //set you have already writed bytes
        void   SetWriteSize(size_t size);
        //get can be readed bytes
        unsigned char*  GetCanReadBuffer(size_t& size, bool& more);
        //set you have already readed bytes
        void   SetReadSize(size_t size);

        //get size of bytes can be readed
        inline size_t GetAllCanReadSize(){
            return nCanRead;
        }

		inline size_t GetCanWriteSize() {
			return nCanWrite;
		}

        //calculate all size of bytes can be readed
        inline size_t CalAllCanReadSize(){

            //how much bytes can be readed
            unsigned char* pStart = buffer;
            unsigned char* pEnd = buffer + bufferSize;

            assert(pStart <= pRead && pRead <= pEnd);

            assert(pStart <= pWrite && pWrite <= pEnd);

            int len = 0;
            if(pRead == pWrite){
                if(bFull) {
                    len = (int)(pEnd - pStart);
                }
            }else if(pWrite > pRead) {
                len = (int)(pWrite - pRead);
            }else {
                len = (int)(pEnd - pRead);
                len += (int)(pWrite - pStart);
            }
            return len;
        }

        //clear buffer
        inline void Clear(){
            pWrite = buffer;
            pRead = buffer;
            bEmpty = true;
            bFull = false;
            moreRead = false;
            nCanWrite = bufferSize;
            nCanRead = 0;
        }
    private:
        size_t                  bufferSize;
        unsigned char*          pWrite;
        unsigned char*          pRead;
        bool                    bEmpty;
        bool                    bFull;
        bool                    moreRead;
        size_t                  nCanWrite;
        size_t                  nCanRead;
        unsigned char*          buffer;
    };
}
#endif	/* RECEIVEBUFFER_H */

