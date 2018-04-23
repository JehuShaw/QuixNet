/*
 * File:   TCPSocketBase.h
 * Author: Jehu Shaw
 *
 * Created on 2011.10.15
 */

#ifndef TCPSOCKETBASE_H
#define	TCPSOCKETBASE_H

#include <string.h>
#include "NetworkTypes.h"
#include "SmallBuffer.h"

namespace ntwk
{
    class SHARED_DLL_DECL TCPSocketBase {
    public:
        virtual ~TCPSocketBase(){}
        //Start the TCP server on the indicated port
        virtual bool Start(const char* address,unsigned short maxLink, unsigned short threadNum = 0) = 0;
        //Stop the TCP server
        virtual bool Stop(void) = 0;
        //Connect to the specified host on the specified port
        virtual bool Connect(SocketID& socketId, const char* address, unsigned short threadNum = 0) = 0;
        // Sends a byte stream
        virtual bool Send(unsigned char * data
						,unsigned int length
						,int socketIdx
						,unsigned char* prefixData = (unsigned char*)0
						,unsigned int prefixLength = 0) = 0;

		inline bool Send(unsigned char * data
			,unsigned int length
			,const SocketID& socketId
			,unsigned char* prefixData = (unsigned char*)0
			,unsigned int prefixLength = 0)
		{
			return Send(data, length, socketId.index, prefixData, prefixLength);
		}

        //Disconnects aplayer/address
        virtual void CloseConnection(const SocketID& socketId) = 0;
    protected:

		inline static void ReverseBytes(char *input, char *output, int length){
			for (int i=0; i < length; ++i){
				output[i]=input[length-i-1];
			}
		}

        inline static void SwitchReverseBytes(unsigned char *output
            , unsigned int outputSize
            , const unsigned char* input0
            , unsigned int length0
            , const unsigned char* input1
            , unsigned int length1
            , unsigned int offset) {

                unsigned int uVariable(0);
                for(unsigned int i = 0; i < outputSize; ++i) {
                    uVariable = offset + outputSize - 1 - i;
                    if(uVariable >= length0 || 0 == length0) {
                        assert((uVariable - length0) < length1);
                        output[i] = *(unsigned char*)(input1 + uVariable - length0);
                    } else {
                        output[i] = *(unsigned char*)(input0 + uVariable);
                    }
                }
        }

        inline static void SwitchCopyBytes(unsigned char *output
            , unsigned int outputSize
            , const unsigned char* input0
            , unsigned int length0
            , const unsigned char* input1
            , unsigned int length1
            , unsigned int offset) {

                unsigned int uVariable(0);
                for(unsigned int i = 0; i < outputSize; ++i) {
                    uVariable = offset + i;
                    if(uVariable >= length0 || 0 == length0) {
                        assert((uVariable - length0) < length1);
                        output[i] = *(unsigned char*)(input1 + uVariable - length0);
                    } else {
                        output[i] = *(unsigned char*)(input0 + uVariable);
                    }
                }
        }

        inline static void SwitchMemcpy(SmallBuffer& data
            , const unsigned char* input0
            , unsigned int size0
            , const unsigned char* input1
            , unsigned int size1
            , unsigned int len
            , unsigned int offset) {

                data.Put(len);

                if(offset >= size0 || 0 == size0) {

                    assert(size1 >= len);
                    memcpy((char*)data,(input1 + offset - size0),len);

                } else {

                    unsigned int firstRead = len > (size0 - offset) ? (size0 - offset) : len;
                    memcpy((char*)data, input0 + offset, firstRead);

                    unsigned int leaveRead = len - firstRead;
                    if(leaveRead > 0) {
                        assert(size1 >= leaveRead);
                        memcpy(((char*)data + firstRead),input1,leaveRead);
                    }
                }
        }

		inline static uint32_t Hash(uint32_t uKey) {
			// 32 bit Fibonacci 2654435769
			return uKey * 2654435769;
		}
	};

}

#endif	/* TCPSOCKETBASE_H */
