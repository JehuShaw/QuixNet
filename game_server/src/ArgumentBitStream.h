/* 
 * File:   ArgumentBitStream.h
 * Author: Jehu Shaw
 *
 * Created on 2020_07_07, 14:35
 */

#ifndef ARGUMENTBITSTREAM_H
#define	ARGUMENTBITSTREAM_H

#include "BitStream.h"
#include "AgentMethod.h"

class CArgumentBitStream : public ntwk::BitStream, public evt::ArgumentBase, public util::PoolBase<CArgumentBitStream>
{
public:
	CArgumentBitStream(void) : BitStream() {}
	CArgumentBitStream(int initialBytesToAllocate) : BitStream(initialBytesToAllocate) {}
	CArgumentBitStream(const char* szData, unsigned int uSize, bool bCopy) : BitStream((char*)szData, uSize, bCopy) {}

	virtual void Reset() {
		this->ntwk::BitStream::SetReadOffset(0);
	}
};

#endif	/* ARGUMENTBITSTREAM_H */

