#include <iostream>
#include <malloc.h>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include "BitSignSet.h"

namespace util {

#define MOD8(value) ((value) & 0x7)
#define DIV8(value) ((value) >> 3)
#define MUL8(value) ((value) << 3)

#define MOD16(value) ((value) & 0xf)
#define DIV16(value) ((value) >> 4)

#define MOD32(value) ((value) & 0x1f)
#define DIV32(value) ((value) >> 5)

#define MOD64(value) ((value) & 0x3f)
#define DIV64(value) ((value) >> 6)
#define MUL64(value) ((value) << 6)

#define FULL64 0xFFFFFFFFFFFFFFFF
#define FULL32 0xFFFFFFFF
#define FULL16 0xFFFF
#define FULL8 0xFF

#define EMPTY 0x0
/*
 * construction function
*/
BitSignSet::BitSignSet() 
	: m_uBitSize(MUL8(sizeof(m_stackBuf)))
	, m_stackBuf(0)
	, m_buf((uint8_t*)&m_stackBuf)
{
}

BitSignSet::BitSignSet(unsigned long uBitSize) {

    if(0 == uBitSize) {
		m_uBitSize = MUL8(sizeof(m_stackBuf));
		m_stackBuf = 0;
		m_buf = (uint8_t*)&m_stackBuf;
        return;
    }

    unsigned long uByteSize = BitToByteSize(uBitSize);

	if(uByteSize <= sizeof(m_stackBuf))  {
		m_stackBuf = 0;
		m_buf = (uint8_t*)&m_stackBuf;
	} else {
		if((m_buf = (uint8_t*)malloc(uByteSize)) == NULL){
			throw std::runtime_error("malloc failed");
		}
		memset(m_buf, 0, uByteSize);
	}
	m_uBitSize = uBitSize;
}

BitSignSet::BitSignSet(const BitSignSet& orig) {
	if(this == &orig) {
		return;
	}

	if(0 == orig.m_uBitSize) {
		m_uBitSize = MUL8(sizeof(m_stackBuf));
		m_stackBuf = 0;
		m_buf = (uint8_t*)&m_stackBuf;
		return;
	}

	unsigned long uByteSize = BitToByteSize(orig.m_uBitSize);

	if(uByteSize <= sizeof(m_stackBuf))  {
		m_buf = (uint8_t*)&m_stackBuf;
	} else {
		if((m_buf = (uint8_t*)malloc(uByteSize)) == NULL){
			throw std::runtime_error("malloc failed");
		}
	}
	m_uBitSize = orig.m_uBitSize;
	memcpy(m_buf, orig.m_buf, uByteSize);
}

BitSignSet& BitSignSet::operator = (const BitSignSet& right)
{
	if(this == &right) {
		return *this;
	}

	if(0 == right.m_uBitSize) {
		m_uBitSize = MUL8(sizeof(m_stackBuf));
		m_stackBuf = 0;
		m_buf = (uint8_t*)&m_stackBuf;
		return *this;
	}

	unsigned long uOldByteSize = BitToByteSize(m_uBitSize);	
	unsigned long uByteSize = BitToByteSize(right.m_uBitSize);

	if(uOldByteSize < uByteSize) {
		if(m_buf != (uint8_t*)&m_stackBuf) {
			free(m_buf);
		}
		if(uByteSize > sizeof(m_stackBuf)) {
			if((m_buf = (uint8_t*)malloc(uByteSize)) == NULL){
				throw std::runtime_error("malloc failed");
			}
		} else {
			m_buf = (uint8_t*)&m_stackBuf;
		}
		m_uBitSize = right.m_uBitSize;
	}
	memcpy(m_buf, right.m_buf, uByteSize);
	return *this;
}

BitSignSet::~BitSignSet(){

	if(m_buf != (uint8_t*)&m_stackBuf) {
		free(m_buf);
		m_buf = NULL;
	}
}

unsigned long BitSignSet::BitToByteSize(unsigned long value) {
	if(MOD8(value) != 0) {
		return DIV8(value) + 1;
	} else {
		return DIV8(value);
	}
}

void BitSignSet::SetBit(unsigned long index, bool isHit) {

	if(index >= m_uBitSize) {
		throw std::range_error("The index out of range !");
		return;
	}

	int sub_index = 7 - MOD8(index);
	uint8_t* location = m_buf + DIV8(index);

	if(isHit) {
		*location = Set1(*location, sub_index);
    } else {
		*location = Set0(*location, sub_index);
    }
};

bool BitSignSet::GetBit(unsigned long index){
    if(index >= m_uBitSize) {
        throw std::range_error("The index out of range !");
		return false;
    }
	return GetByteBit(*(m_buf + DIV8(index)), 7 - MOD8(index));
}

unsigned long BitSignSet::FindFirst0(unsigned long offset) {

    uint64_t* pUInt64 = (uint64_t*)(m_buf + BitToByteSize(offset));
    uint64_t* pU64End = ((uint64_t*)m_buf) + DIV64(m_uBitSize);
    while(pUInt64 < pU64End){
        if(FULL64 != *pUInt64){
            break;
        }
        ++pUInt64;
    }

    unsigned long index = MUL64(pUInt64 - (uint64_t*)m_buf);

	if(index == m_uBitSize) {
		return index;
	}

    uint32_t* pUInt32 = (uint32_t*)pUInt64;
    if(DIV32(m_uBitSize - index) > 0) {
        if(FULL32 == *pUInt32){
            ++pUInt32;
            index += 32;
        }
    }

    uint16_t* pUInt16 = (uint16_t*)pUInt32;
    if(DIV16(m_uBitSize - index) > 0) {
        if(FULL16 == *pUInt16){
            ++pUInt16;
            index += 16;
        }
    }

    uint8_t* pUInt8 = (uint8_t*)pUInt16;
    if(DIV8(m_uBitSize - index) > 0) {
        if(FULL8 == *pUInt8){
            ++pUInt8;
            index += 8;
        }
    }

    int floor = 7 - (m_uBitSize - index);
    for(int i = 7; i > floor; --i) {
        if(!GetByteBit(*pUInt8, i)) {
            break;
        }
        ++index;
    }

    return index;

}

unsigned long BitSignSet::FindFirst1(unsigned long offset) {

    uint64_t* pUInt64 = (uint64_t*)(m_buf + BitToByteSize(offset));
    uint64_t* pU64End = ((uint64_t*)m_buf) + DIV64(m_uBitSize);
    while(pUInt64 < pU64End){
        if(EMPTY != *pUInt64){
            break;
        }
        ++pUInt64;
    }

    unsigned long index = MUL64(pUInt64 - (uint64_t*)m_buf);

	if(index == m_uBitSize) {
		return index;
	}

    uint32_t* pUInt32 = (uint32_t*)pUInt64;
    if(DIV32(m_uBitSize - index) > 0) {
        if(EMPTY == *pUInt32){
            ++pUInt32;
            index += 32;
        }
    }

    uint16_t* pUInt16 = (uint16_t*)pUInt32;
    if(DIV16(m_uBitSize - index) > 0) {
        if(EMPTY == *pUInt16){
            ++pUInt16;
            index += 16;
        }
    }

    uint8_t* pUInt8 = (uint8_t*)pUInt16;
    if(DIV8(m_uBitSize - index) > 0) {
        if(EMPTY == *pUInt8){
            ++pUInt8;
            index += 8;
        }
    }

    int floor = 7 - (m_uBitSize - index);
    for(int i = 7; i > floor; --i) {
        if(GetByteBit(*pUInt8, i)) {
            break;
        }
        ++index;
    }

    return index;
}

void BitSignSet::PrintBitSet(std::string& outStr) {
	if(!outStr.empty()) {
		outStr.clear();
	}
	uint8_t* pByte = m_buf;
    uint8_t* pEnd = m_buf + DIV8(m_uBitSize);
    while(pByte < pEnd){
        PrintChar(outStr, *pByte);
        ++pByte;
    }
    unsigned int uLeaveBit = MOD8(m_uBitSize);
	if(uLeaveBit != 0) {
		PrintChar(outStr, *pByte, uLeaveBit);
    }
}

void BitSignSet::OrBitSet(const BitSignSet& right)
{
	unsigned long uByteSize = BitToByteSize(
		std::min(m_uBitSize, right.m_uBitSize));

	for(unsigned long i = 0; i < uByteSize; ++i) {
		m_buf[i] |= right.m_buf[i];
	}
}

void BitSignSet::ResetBitSet()
{
	unsigned long uByteSize = BitToByteSize(m_uBitSize);
	if(uByteSize < 1) {
		return;
	}
	memset(m_buf, 0, uByteSize);
}

} // end namespace util



