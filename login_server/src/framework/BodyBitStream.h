/* 
 * File:   BodyBitStream.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#pragma once

#include "Common.h"
#include "INotificationBody.h"
#include "BitStream.h"

class CBodyBitStream : public ntwk::BitStream, public mdl::IBody
{
public:
	CBodyBitStream(void):BitStream() {}
	CBodyBitStream(int initialBytesToAllocate):BitStream(initialBytesToAllocate) {}
	CBodyBitStream(const char* szData, unsigned int uSize, bool bCopy): BitStream((char*)szData, uSize, bCopy) {}

    /** 
     * reset body data;
     */
    virtual void ResetBody() {
		this->BitStream::Reset();
	}
};

