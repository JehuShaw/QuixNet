
// This should be on by default for speed.  Turn it off if you actually need endian swapping
#define __BITSTREAM_BIG_END
//#define __BITSTREAM_NATIVE_END

#include "BitStream.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#ifdef _WIN32
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <float.h>
#else
#define _copysign copysign
#endif


#if defined ( __APPLE__ ) || defined ( __APPLE_CC__ )
#include <malloc/malloc.h>
#else
#include <malloc.h>
#include <string>
#endif

namespace ntwk {

#ifdef __BITSTREAM_BIG_END
// Set up the read/write routines to produce Big-End network streams.
#define B16_1 0
#define B16_0 1

#define B32_3 0
#define B32_2 1
#define B32_1 2
#define B32_0 3

#define B64_7 0
#define B64_6 1
#define B64_5 2
#define B64_4 3
#define B64_3 4
#define B64_2 5
#define B64_1 6
#define B64_0 7

#else
// Default to producing Little-End network streams.
#define B16_1 1
#define B16_0 0

#define B32_3 3
#define B32_2 2
#define B32_1 1
#define B32_0 0

#define B64_7 7
#define B64_6 6
#define B64_5 5
#define B64_4 4
#define B64_3 3
#define B64_2 2
#define B64_1 1
#define B64_0 0
#endif

#define STRING_MAX_SIZE 65535

#define MOD8(value) ((value) & 0x7)
#define DIV8(value) ((value) >> 3)
#define MUL8(value) ((value) << 3)

BitStream::BitStream()
{
	m_writeOffset = 0;
	m_allocBitSize = BITSTREAM_STACK_ALLOCATION_SIZE * 8;
	m_readOffset = 0;
	m_data = (unsigned char*) m_stackData;
	m_copyData = true;
}

BitStream::BitStream( uint32_t initByteSize )
{
	m_writeOffset = 0;
	m_readOffset = 0;
	if(initByteSize <= BITSTREAM_STACK_ALLOCATION_SIZE) {
		m_data = (unsigned char*) m_stackData;
		m_allocBitSize = BITSTREAM_STACK_ALLOCATION_SIZE * 8;
	} else {
		m_data = (unsigned char*) malloc(initByteSize);
		m_allocBitSize = MUL8(initByteSize);
	}
#ifdef _DEBUG
	assert(m_data);
#endif
	m_copyData = true;
}

BitStream::BitStream( const char* data, uint32_t lengthInBytes, bool copyData )
{
	m_writeOffset = MUL8(lengthInBytes);
	m_readOffset = 0;
	m_copyData = copyData;
	m_allocBitSize = MUL8(lengthInBytes);
	
	if(m_copyData) {
		if(lengthInBytes > 0) {
			if(lengthInBytes < BITSTREAM_STACK_ALLOCATION_SIZE) {
				m_data = ( unsigned char* ) m_stackData;
				m_allocBitSize = MUL8(BITSTREAM_STACK_ALLOCATION_SIZE);
			} else {
				m_data = (unsigned char*) malloc(lengthInBytes);
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, data, lengthInBytes);
		} else {
			m_data = NULL;
		}
	} else {
		m_data = (unsigned char*)data;
	}
}

// Use this if you pass a pointer copy to the constructor (_copyData==false) and want to overallocate to prevent reallocation
void BitStream::SetNumberOfBitsAllocated( const uint32_t lengthInBits )
{
#ifdef _DEBUG
	assert(lengthInBits >= m_allocBitSize);
#endif	
	m_allocBitSize = lengthInBits;
}

BitStream::~BitStream()
{
	if(m_copyData && m_stackData != m_data) {
		// Use realloc and free so we are more efficient than delete and new for resizing
		free(m_data);
		m_data = NULL;
	}
}

void BitStream::Reset( void )
{
	m_writeOffset = 0;
	m_readOffset = 0;
}

// Write the native types to the end of the buffer
void BitStream::WriteBool( const bool input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 0;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
	if ( input ) {
		Write1();
	} else {
		Write0();
	}
}

void BitStream::WriteUInt8( const uint8_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 1;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
}

void BitStream::WriteInt8( const int8_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 2;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
}

void BitStream::WriteUInt16( const uint16_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 3;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif

#ifdef __BITSTREAM_NATIVE_END
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else	
	static unsigned char uint16w[2];
	uint16w[B16_1] =  (input >> 8)&(0xff);
	uint16w[B16_0] = input&(0xff);

	WriteBits( uint16w, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteInt16( const int16_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 4;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char int16w[2];
	int16w[B16_1] =  (input >> 8)&(0xff);
	int16w[B16_0] = input&(0xff);
	
	WriteBits( int16w, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteUInt32( const uint32_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 5;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char uint32w[4];
	uint32w[B32_3] = (input >> 24)&(0x000000ff);
	uint32w[B32_2] = (input >> 16)&(0x000000ff);
	uint32w[B32_1] = (input >> 8)&(0x000000ff);
	uint32w[B32_0] = (input)&(0x000000ff);
	
	WriteBits( uint32w, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteInt32( const int32_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 6;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char int32w[4];
	int32w[B32_3] = (input >> 24)&(0x000000ff);
	int32w[B32_2] = (input >> 16)&(0x000000ff);
	int32w[B32_1] = (input >> 8)&(0x000000ff);
	int32w[B32_0] = (input)&(0x000000ff);
	
	WriteBits( int32w, sizeof( input ) * 8, true );
#endif
}

#ifndef NO_INT64
void BitStream::WriteUInt64( const uint64_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 7;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char uint64w[8];
	uint64w[B64_7] = (input >> 56) & 0xff;
	uint64w[B64_6] = (input >> 48) & 0xff;
	uint64w[B64_5] = (input >> 40) & 0xff;
	uint64w[B64_4] = (input >> 32) & 0xff;
	uint64w[B64_3] = (input >> 24) & 0xff;
	uint64w[B64_2] = (input >> 16) & 0xff;
	uint64w[B64_1] = (input >> 8) & 0xff;
	uint64w[B64_0] = input & 0xff;
	
	WriteBits( uint64w, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteInt64( const int64_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 8;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char int64w[8];
	int64w[B64_7] = (input >> 56) & 0xff;
	int64w[B64_6] = (input >> 48) & 0xff;
	int64w[B64_5] = (input >> 40) & 0xff;
	int64w[B64_4] = (input >> 32) & 0xff;
	int64w[B64_3] = (input >> 24) & 0xff;
	int64w[B64_2] = (input >> 16) & 0xff;
	int64w[B64_1] = (input >> 8) & 0xff;
	int64w[B64_0] = input & 0xff;
	
	WriteBits( int64w, sizeof( input ) * 8, true );
#endif
}

#endif

void BitStream::WriteFloat( const float input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 9;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif

#ifndef __BITSTREAM_NATIVE_END
	unsigned int intval = *((unsigned int *)(&input));
	WriteUInt32(intval);
#else
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteDouble( const double input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 10;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif

#if defined ( __BITSTREAM_NATIVE_END ) || defined (NO_INT64)
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	uint64_t intval = *((uint64_t *)(&input));
	WriteUInt64(intval);
#endif
}
// Write an array or casted stream
void BitStream::WriteBytes( const char* input, const int numberOfBytes )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 11;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
	WriteBits( ( unsigned char* ) & numberOfBytes, sizeof( int ) * 8, true );
#endif
	
	WriteBits( ( unsigned char* ) input, numberOfBytes * 8, true );
}

void BitStream::WriteString(char input[])
{
    uint16_t len = (uint16_t)strlen(input);
    if(len > 0){
		WriteUInt16(len);
        WriteBytes(input, len);
    }
}

void BitStream::WriteString(const std::string& input)
{
	uint16_t len = (uint16_t)input.size();
	if(len > 0){
		WriteUInt16(len);
		WriteBytes(input.data(), len);
	}
}

void BitStream::WriteBS( const BitStream& bitStream )
{
	WriteBits((unsigned char*)bitStream.GetData(), bitStream.GetWriteOffset(), false);
}

// Write the native types with simple compression.
// Best used with  negatives and positives close to 0
void BitStream::WriteCompUInt8( const uint8_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 12;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
}

void BitStream::WriteCompInt8( const int8_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 13;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, false );
}

void BitStream::WriteCompUInt16( const uint16_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 14;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char uint16wc[2];
	uint16wc[B16_1] = (input >> 8)&(0xff);
	uint16wc[B16_0] = input&(0xff);
	
	WriteCompressed( uint16wc, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteCompInt16( const int16_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 15;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char int16wc[2];
	int16wc[B16_1] =  (input >> 8)&(0xff);
	int16wc[B16_0] = input&(0xff);
	
	WriteCompressed( int16wc, sizeof( input ) * 8, false );
#endif
}

void BitStream::WriteCompUInt32( const uint32_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 16;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char uint32wc[4];
	uint32wc[B32_3] = (input >> 24)&(0x000000ff);
	uint32wc[B32_2] = (input >> 16)&(0x000000ff);
	uint32wc[B32_1] = (input >> 8)&(0x000000ff);
	uint32wc[B32_0] = (input)&(0x000000ff);
	
	WriteCompressed( uint32wc, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteCompInt32( const int32_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 17;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char int32wc[4];
	int32wc[B32_3] = (input >> 24)&(0x000000ff);
	int32wc[B32_2] = (input >> 16)&(0x000000ff);
	int32wc[B32_1] = (input >> 8)&(0x000000ff);
	int32wc[B32_0] = (input)&(0x000000ff);
	
	WriteCompressed( int32wc, sizeof( input ) * 8, false );
#endif
}

#ifndef NO_INT64
void BitStream::WriteCompUInt64( const uint64_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 18;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char uint64wc[8];
	uint64wc[B64_7] = (input >> 56) & 0xff;
	uint64wc[B64_6] = (input >> 48) & 0xff;
	uint64wc[B64_5] = (input >> 40) & 0xff;
	uint64wc[B64_4] = (input >> 32) & 0xff;
	uint64wc[B64_3] = (input >> 24) & 0xff;
	uint64wc[B64_2] = (input >> 16) & 0xff;
	uint64wc[B64_1] = (input >> 8) & 0xff;
	uint64wc[B64_0] = input & 0xff;
	
	WriteCompressed( uint64wc, sizeof( input ) * 8, true );
#endif
}

void BitStream::WriteCompInt64( const int64_t input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 19;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif
	
#ifdef __BITSTREAM_NATIVE_END
	WriteCompressed( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	static unsigned char int64wc[8];
	int64wc[B64_7] = (input >> 56) & 0xff;
	int64wc[B64_6] = (input >> 48) & 0xff;
	int64wc[B64_5] = (input >> 40) & 0xff;
	int64wc[B64_4] = (input >> 32) & 0xff;
	int64wc[B64_3] = (input >> 24) & 0xff;
	int64wc[B64_2] = (input >> 16) & 0xff;
	int64wc[B64_1] = (input >> 8) & 0xff;
	int64wc[B64_0] = input & 0xff;
	
	WriteCompressed( int64wc, sizeof( input ) * 8, false );
#endif
}
#endif


void BitStream::WriteCompFloat( const float input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 20;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif

// Not yet implemented (no compression)
#if defined ( __BITSTREAM_NATIVE_END )
	WriteBits( ( unsigned char* ) &input, sizeof( input ) * 8, true );
#else
	WriteFloat( input );
#endif
}

void BitStream::WriteCompDouble( const double input )
{
#ifdef TYPE_CHECKING
	unsigned char ID = 21;
	WriteBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8, true );
#endif

	// Not yet implemented (no compression)
#if defined ( __BITSTREAM_NATIVE_END )
	WriteBits( ( unsigned char* ) & input, sizeof( input ) * 8, true );
#else
	WriteDouble( input );
#endif
}

void BitStream::WriteNormVector( float x, float y, float z )
{
#ifdef _DEBUG
	assert( x <= 1.01f && y <= 1.01f && z <= 1.01f && x >= -1.01f && y >= -1.01f && z >= -1.01f );
#endif

	if( x > 1.0f ) {
		x = 1.0f;
	}
	
	if( y > 1.0f ) {
		y = 1.0f;
	}
	
	if( z > 1.0f ) {
		z = 1.0f;
	}
	
	if( x < -1.0f ) {
		x = -1.0f;
	}
	
	if( y < -1.0f ) {
		y = -1.0f;
	}
	
	if( z < -1.0f ) {
		z = -1.0f;
	}

	WriteBool((bool) (x < 0.0f));

	if( y == 0.0f ) {
		WriteBool(true);
	} else {
		WriteBool(false);
		WriteUInt16((unsigned short)((y+1.0f)*32767.5f));
	}
	
	if( z == 0.0f ) {
		WriteBool(true);
	} else {
		WriteBool(false);
		WriteUInt16((unsigned short)((z+1.0f)*32767.5f));
	}
}

void BitStream::WriteVector( float x, float y, float z )
{
	float magnitude = sqrtf(x * x + y * y + z * z);
	WriteFloat(magnitude);
	if (magnitude > 0.0f)
	{
		WriteUInt16((unsigned short)((x/magnitude+1.0f)*32767.5f));
		WriteUInt16((unsigned short)((y/magnitude+1.0f)*32767.5f));
		WriteUInt16((unsigned short)((z/magnitude+1.0f)*32767.5f));
	}
}

void BitStream::WriteNormQuat( float w, float x, float y, float z)
{
	WriteBool((bool)(w<0.0f));
	WriteBool((bool)(x<0.0f));
	WriteBool((bool)(y<0.0f));
	WriteBool((bool)(z<0.0f));
	WriteUInt16((unsigned short)(fabs(x)*65535.0));
	WriteUInt16((unsigned short)(fabs(y)*65535.0));
	WriteUInt16((unsigned short)(fabs(z)*65535.0));
	// Leave out w and calcuate it on the target
}

void BitStream::WriteOrthMatrix( 
					 float m00, float m01, float m02,
					 float m10, float m11, float m12,
					 float m20, float m21, float m22 )
{
	double qw;
	double qx;
	double qy;
	double qz;

	// Convert matrix to quat
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	qw = sqrt( 1 + m00 + m11 + m22  ) / 2; 
	qx = sqrt( 1 + m00 - m11 - m22  ) / 2; 
	qy = sqrt( 1 - m00 + m11 - m22  ) / 2; 
	qz = sqrt( 1 - m00 - m11 + m22  ) / 2;
	if (qw < 0.0) qw=0.0;
	if (qx < 0.0) qx=0.0;
	if (qy < 0.0) qy=0.0;
	if (qz < 0.0) qz=0.0;
	qx = _copysign( qx, m21 - m12 );
	qy = _copysign( qy, m02 - m20 );
	qz = _copysign( qz, m20 - m02 );

	WriteNormQuat((float)qw,(float)qx,(float)qy,(float)qz);
}

// Read the native types from the front of the buffer
// Write the native types to the end of the buffer
bool BitStream::ReadBool()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return false;
		
#ifdef _DEBUG
	assert( ID == 0 );
#endif
    return true;

#endif
	//// If this assert is hit the stream wasn't long enough to read from
	//assert(readOffset+1 <=numberOfBitsUsed);
	if (m_readOffset + 1 > m_writeOffset) {
		return false;
	}

	//// Check that bit	
	//if (ReadBit()) 
	// Is it faster to just write it out here?
	if (m_data[DIV8(m_readOffset)] & (0x80 >> MOD8(m_readOffset++))) {
		return true;
	}
		
	return false;
}

uint8_t BitStream::ReadUInt8()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 1 );
    return ID;
#endif
        static uint8_t uint8r;
	if(ReadBits( ( unsigned char* ) & uint8r, sizeof( uint8r ) * 8 )){
		return uint8r;
    }
    return 0;
}

int8_t BitStream::ReadInt8()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 2 );
    return ID;
#endif
	static int8_t int8r;
	if(ReadBits( ( unsigned char* ) & int8r, sizeof( int8r ) * 8 )) {
		return int8r;
    }
    return 0;
}

uint16_t BitStream::ReadUInt16()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 3 );
    return ID;
#endif
	static unsigned char uint16r[2];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadBits( uint16r, sizeof( uint16_t ) * 8 )){
		return *(uint16_t*)uint16r;
    }
    return 0;
#else
	if (ReadBits( uint16r, sizeof( uint16_t ) * 8 ) != true) return 0;
	return (((uint16_t) uint16r[B16_1])<<8)|((uint16_t)uint16r[B16_0]);
#endif
}

int16_t BitStream::ReadInt16()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 4 );
    return ID;
#endif
	static unsigned char int16r[2];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadBits( int16r, sizeof( int16_t ) * 8 )){
		return *(int16_t*)int16r;
    }
    return 0;
#else
	if (ReadBits( int16r, sizeof( int16_t ) * 8 ) != true) return 0;
	return (((int16_t) int16r[B16_1])<<8)|((int16_t)int16r[B16_0]);
#endif
}

uint32_t BitStream::ReadUInt32()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 5 );
    return ID;
#endif
	static unsigned char uint32r[4];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadBits( uint32r, sizeof( uint32_t ) * 8 )){
		return *(uint32_t*)uint32r;
    }
    return 0;
#else
	if(ReadBits( uint32r, sizeof( uint32_t ) * 8 ) != true)
		return 0;
	return (((uint32_t) uint32r[B32_3])<<24)|
		(((uint32_t) uint32r[B32_2])<<16)|
		(((uint32_t) uint32r[B32_1])<<8)|
		((uint32_t) uint32r[B32_0]);
#endif
}

int32_t BitStream::ReadInt32()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 6 );
        return ID;
#endif
	static unsigned char int32r[4];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadBits( int32r, sizeof( int32_t ) * 8 )){
		return *(int32_t*)int32r;
    }
    return 0;
#else
	if(ReadBits( int32r, sizeof( int32_t ) * 8 ) != true)
		return 0;
	return (((int32_t) int32r[B32_3])<<24)|
		(((int32_t) int32r[B32_2])<<16)|
		(((int32_t) int32r[B32_1])<<8)|
		((int32_t) int32r[B32_0]);
#endif
}

#ifndef NO_INT64
uint64_t BitStream::ReadUInt64()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 7 );
    return ID;
#endif
	static unsigned char uint64r[8];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadBits( uint64r, sizeof( uint64_t ) * 8 )){
		return *(uint64_t*)uint64r;
    }
    return 0;
#else
	if(ReadBits( uint64r, sizeof( uint64_t ) * 8 ) != true)
		return 0;
	return (((uint64_t) uint64r[B64_7])<<56)|(((uint64_t) uint64r[B64_6])<<48)|
		(((uint64_t) uint64r[B64_5])<<40)|(((uint64_t) uint64r[B64_4])<<32)|
		(((uint64_t) uint64r[B64_3])<<24)|(((uint64_t) uint64r[B64_2])<<16)|
		(((uint64_t) uint64r[B64_1])<<8)|((uint64_t) uint64r[B64_0]);
#endif
}

int64_t BitStream::ReadInt64()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 8 );
    return ID;
#endif
	static unsigned char int64r[8];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadBits(int64r, sizeof( int64_t ) * 8 )){
            return *(int64_t*)int64r;
        }
        return 0;
#else
	if(ReadBits( int64r, sizeof( int64_t ) * 8 ) != true)
		return 0;
	return (((uint64_t) int64r[B64_7])<<56)|(((uint64_t) int64r[B64_6])<<48)|
		(((uint64_t) int64r[B64_5])<<40)|(((uint64_t) int64r[B64_4])<<32)|
		(((uint64_t) int64r[B64_3])<<24)|(((uint64_t) int64r[B64_2])<<16)|
		(((uint64_t) int64r[B64_1])<<8)|((uint64_t) int64r[B64_0]);
#endif
}
#endif

float BitStream::ReadFloat()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 9 );
    return ID;
#endif

#ifdef __BITSTREAM_NATIVE_END
	static float floatr;
	if(ReadBits( ( unsigned char* ) & floatr, sizeof( floatr ) * 8 )){
		return floatr;
    }
    return 0;
#else
	unsigned int val = ReadUInt32();
	return *((float *)(&val));
#endif
}

double BitStream::ReadDouble()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 10 );
    return ID;
#endif
	
#if defined ( __BITSTREAM_NATIVE_END ) || defined ( NO_INT64 )
    static double doubler;
	if(ReadBits( ( unsigned char* ) & doubler, sizeof( doubler ) * 8 )){
		return doubler;
    }
    return 0;
#else
	uint64_t val = ReadUInt64();
	return *((double *)(&val));
#endif
}
// Read an array or casted stream
bool BitStream::ReadBytes( char* output, const int numberOfBytes )
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return false;
		
	assert( ID == 11 );
	
	int NOB;
	
	ReadBits( ( unsigned char* ) & NOB, sizeof( int ) * 8 );
	
	assert( NOB == numberOfBytes );
	
#endif
	
	return ReadBits( ( unsigned char* ) output, numberOfBytes * 8 );
}

bool BitStream::ReadString(char output[], int size) {
	if(size < 1) {
		assert(size > 0);
		return false;
	}
	uint16_t len = ReadUInt16();
	if(0 == len) {
		output[0] = '\0';
		return true;
	}
	if(len < size) {
		ReadBytes(output, len);
		output[len] = '\0';
		return true;
	} else {
		int nReadLen = size - 1;
		ReadBytes(output, nReadLen);
		output[nReadLen] = '\0';
		int nLeaveLen = len - nReadLen;
		if(nLeaveLen > 0) {
			IgnoreBits(BYTES_TO_BITS(nLeaveLen));
		}
		return true;
	}
	return false;
}

std::string BitStream::ReadString()
{
	uint16_t len = ReadUInt16();
	if(0 == len) {
		return std::string();
	}
	char szBuf[STRING_MAX_SIZE];
	ReadBytes(szBuf, len);
	szBuf[len] = '\0';
	return std::string(szBuf);
}

// Read the types you wrote with WriteCompressed
uint8_t BitStream::ReadCompUInt8()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 12 );
        return ID;
#endif
    static uint8_t uint8rc;
	if(ReadCompressed( ( unsigned char* ) & uint8rc, sizeof( uint8rc ) * 8, true )) {
		return uint8rc;
	}
	return 0;
}

int8_t BitStream::ReadCompInt8()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 13 );
        return ID;
#endif
    static int8_t int8rc;
	if(ReadCompressed( ( unsigned char* ) & int8rc, sizeof( int8rc ) * 8, false )) {
		return int8rc;
	}
	return 0;
}

uint16_t BitStream::ReadCompUInt16()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 14 );
        return ID;
#endif
	static unsigned char uint16rc[2];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadCompressed( uint16rc, sizeof( uint16_t ) * 8, true )) {
		return *(uint16_t*)uint16rc;
	}
	return 0;
#else
	if (ReadCompressed( uint16rc, sizeof( uint16_t ) * 8, true ) != true) return 0;
	return (((uint16_t) uint16rc[B16_1])<<8)|
		((uint16_t) uint16rc[B16_0]);
#endif
}

int16_t BitStream::ReadCompInt16()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 15 );
        return ID;
#endif
	static unsigned char int16rc[2];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadCompressed( int16rc, sizeof( int16_t ) * 8, true )) {
		return *(int16_t*)int16rc;
	}
	return 0;
#else
	if (ReadCompressed( int16rc, sizeof( int16_t ) * 8, false ) != true) return 0;
	return (((uint16_t) int16rc[B16_1])<<8)|((uint16_t)int16rc[B16_0]);
#endif
}

uint32_t BitStream::ReadCompUInt32()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 16 );
        return ID;
	
#endif
	static unsigned char uint32rc[4];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadCompressed( uint32rc, sizeof( uint32_t ) * 8, true )) {
		return *(uint32_t*)uint32rc;
	}
	return 0;
#else
	if(ReadCompressed( uint32rc, sizeof( uint32_t ) * 8, true ) != true)
		return 0;
	return (((uint32_t) uint32rc[B32_3])<<24)|
		(((uint32_t) uint32rc[B32_2])<<16)|
		(((uint32_t) uint32rc[B32_1])<<8)|
		((uint32_t) uint32rc[B32_0]);
#endif
}

int32_t BitStream::ReadCompInt32()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 17 );
        return ID;
#endif

	static unsigned char int32rc[4];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadCompressed(int32rc, sizeof( int32_t ) * 8, true )){
            return *(int32_t*)int32rc;
    }
    return 0;
#else
	if(ReadCompressed( int32rc, sizeof( int32_t ) * 8, false ) != true)
		return 0;
	return (((uint32_t) int32rc[B32_3])<<24)|
		(((uint32_t) int32rc[B32_2])<<16)|
		(((uint32_t) int32rc[B32_1])<<8)|
		((uint32_t) int32rc[B32_0]);

#endif
}

#ifndef NO_INT64
uint64_t BitStream::ReadCompUInt64()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 18 );
        return ID;
#endif
        
	static unsigned char uint64rc[8];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadCompressed( uint64rc, sizeof( uint64_t ) * 8, true )) {
		return *(uint64_t*)uint64rc;
	}
	return 0;
#else
	if(ReadCompressed( uint64rc, sizeof( uint64_t ) * 8, true ) != true)
		return 0;
	return (((uint64_t) uint64rc[B64_7])<<56)|(((uint64_t) uint64rc[B64_6])<<48)|
		(((uint64_t) uint64rc[B64_5])<<40)|(((uint64_t) uint64rc[B64_4])<<32)|
		(((uint64_t) uint64rc[B64_3])<<24)|(((uint64_t) uint64rc[B64_2])<<16)|
		(((uint64_t) uint64rc[B64_1])<<8)|((uint64_t) uint64rc[B64_0]);
#endif
}

int64_t BitStream::ReadCompInt64()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 19 );
        return ID;
#endif
	static unsigned char int64rc[8];
#ifdef __BITSTREAM_NATIVE_END
	if(ReadCompressed( int64rc, sizeof( int64_t ) * 8, true )) {
		return *(int64_t*)int64rc;
	}
	return 0;
#else
	if(ReadCompressed( int64rc, sizeof( int64_t ) * 8, false ) != true)
		return 0;
	return (((uint64_t) int64rc[B64_7])<<56)|(((uint64_t) int64rc[B64_6])<<48)|
		(((uint64_t) int64rc[B64_5])<<40)|(((uint64_t) int64rc[B64_4])<<32)|
		(((uint64_t) int64rc[B64_3])<<24)|(((uint64_t) int64rc[B64_2])<<16)|
		(((uint64_t) int64rc[B64_1])<<8)|((uint64_t) int64rc[B64_0]);
#endif
}
#endif

float BitStream::ReadCompFloat()
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	
	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;
		
	assert( ID == 20 );
        return ID;
#endif
	
	return ReadFloat();
}

double BitStream::ReadCompDouble()
{
#ifdef TYPE_CHECKING
	unsigned char ID;

	if ( ReadBits( ( unsigned char* ) & ID, sizeof(unsigned char) * 8 ) == false )
		return 0;

	assert( ID == 21 );
        return ID;
#endif
        
	return ReadDouble();
}

bool BitStream::ReadNormVector( float &x, float &y, float &z )
{
	uint16_t sy, sz;

	bool xNeg = ReadBool();
	bool yZero = ReadBool();
	
	if (yZero) {
		y = 0.0f;	
	} else {
		sy = ReadUInt16();
		y = ((float)sy / 32767.5f - 1.0f);
	}
	
	bool zZero = ReadBool();
	if (zZero) {
		z = 0.0f;
	} else {
		sz = ReadUInt16();
		z = ((float)sz / 32767.5f - 1.0f);
	}

	x = sqrtf(1.0f - y*y - z*z);
	if (xNeg) {
		x = -x;
	}
	return true;
}

bool BitStream::ReadVector( float &x, float &y, float &z )
{
	uint16_t sx,sy,sz;
	float magnitude = ReadFloat();
	
	if (magnitude!=0.0f) {
	
		sx = ReadUInt16();
		sy = ReadUInt16();
		sz = ReadUInt16();
                
		x=((float)sx / 32767.5f - 1.0f) * magnitude;
		y=((float)sy / 32767.5f - 1.0f) * magnitude;
		z=((float)sz / 32767.5f - 1.0f) * magnitude;
		
	} else {
	
		x=0.0f;
		y=0.0f;
		z=0.0f;
	}
	return true;
}

bool BitStream::ReadNormQuat( float &w, float &x, float &y, float &z)
{
	bool cwNeg = ReadBool();
	bool cxNeg = ReadBool();
	bool cyNeg = ReadBool();
	bool czNeg = ReadBool();
	uint16_t cx = ReadUInt16();
	uint16_t cy = ReadUInt16();
	uint16_t cz = ReadUInt16();

	// Calculate w from x,y,z
	x=cx/65535.0f;
	y=cy/65535.0f;
	z=cz/65535.0f;
	if (cxNeg) x=-x;
	if (cyNeg) y=-y;
	if (czNeg) z=-z;
	w = sqrt(1.0f - x*x - y*y - z*z);
	if (cwNeg) {
		w=-w;
	}
	return true;
}

bool BitStream::ReadOrthMatrix( 
					float &m00, float &m01, float &m02,
					float &m10, float &m11, float &m12,
					float &m20, float &m21, float &m22 )
{
	float qw,qx,qy,qz;
	if ( !ReadNormQuat(qw,qx,qy,qz ) ) {
		return false;
	}

	// Quat to orthogonal rotation matrix
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
	double sqw = (double)qw*(double)qw;
	double sqx = (double)qx*(double)qx;
	double sqy = (double)qy*(double)qy;
	double sqz = (double)qz*(double)qz;
	m00 =  (float)(sqx - sqy - sqz + sqw); // since sqw + sqx + sqy + sqz =1
	m11 = (float)(-sqx + sqy - sqz + sqw);
	m22 = (float)(-sqx - sqy + sqz + sqw);

	double tmp1 = (double)qx*(double)qy;
	double tmp2 = (double)qz*(double)qw;
	m10 = (float)(2.0 * (tmp1 + tmp2));
	m01 = (float)(2.0 * (tmp1 - tmp2));

	tmp1 = (double)qx*(double)qz;
	tmp2 = (double)qy*(double)qw;
	m20 =(float)(2.0 * (tmp1 - tmp2));
	m02 = (float)(2.0 * (tmp1 + tmp2));
	tmp1 = (double)qy*(double)qz;
	tmp2 = (double)qx*(double)qw;
	m21 = (float)(2.0 * (tmp1 + tmp2));
	m12 = (float)(2.0 * (tmp1 - tmp2));

	return true;
}

// Sets the read pointer back to the beginning of your data.
void BitStream::ResetReadPointer( void )
{
	m_readOffset = 0;
}

// Sets the write pointer back to the beginning of your data.
void BitStream::ResetWritePointer( void )
{
	m_writeOffset = 0;
}

// Write a 0
void BitStream::Write0( void )
{
	AddBitsAndReallocate( 1 );
	
	// New bytes need to be zeroed	
	if( MOD8( m_writeOffset ) == 0 ) {
		m_data[ DIV8( m_writeOffset ) ] = 0;
	}
		
	++m_writeOffset;
}

// Write a 1
void BitStream::Write1( void )
{
	AddBitsAndReallocate( 1 );
	
	int numberOfBitsMod8 = MOD8( m_writeOffset );
	
	if( 0 == numberOfBitsMod8 ) {
		m_data[ DIV8( m_writeOffset ) ] = 0x80;
	} else {
		// Set the bit to 1
		m_data[ DIV8( m_writeOffset ) ] |= ( 0x80 >> numberOfBitsMod8 );
	}
		
	++m_writeOffset;
}

// Returns true if the next data read is a 1, false if it is a 0
bool BitStream::ReadBit( void )
{
#pragma warning( disable : 4800 )
	return (bool) ( m_data[ DIV8( m_readOffset ) ] & ( 0x80 >> MOD8( m_readOffset++ ) ) );
#pragma warning( default : 4800 )
}

// Align the bitstream to the byte boundary and then write the specified number of bits.
// This is faster than WriteBits but wastes the bits to do the alignment and requires you to call
// SetReadToByteAlignment at the corresponding read position
void BitStream::WriteAlignedBytes( const unsigned char* input,
	const int numberOfBytesToWrite )
{
	if( numberOfBytesToWrite < 1 ) {
#ifdef _DEBUG
		assert(false);
#endif
		return;
	}
	
	AlignWriteToByteBoundary();
	// Allocate enough memory to hold everything
	AddBitsAndReallocate( MUL8( numberOfBytesToWrite ) );
	
	// Write the data
	memcpy( m_data + DIV8( m_writeOffset ), input, numberOfBytesToWrite );
	
	m_writeOffset += MUL8( numberOfBytesToWrite );
}

// Read bits, starting at the next aligned bits. Note that the modulus 8 starting offset of the
// sequence must be the same as was used with WriteBits. This will be a problem with packet coalescence
// unless you byte align the coalesced packets.
bool BitStream::ReadAlignedBytes( unsigned char* output,
	const int numberOfBytesToRead )
{
	if( numberOfBytesToRead < 1 ) {
#ifdef _DEBUG
		assert(false);
#endif
		return false;
	}
	
	// Byte align
	AlignReadToByteBoundary();
	
	if( m_readOffset + MUL8( numberOfBytesToRead ) > m_writeOffset ) {
		return false;
	}
		
	// Write the data
	memcpy( output, m_data + DIV8( m_readOffset ), numberOfBytesToRead );
	
	m_readOffset += MUL8( numberOfBytesToRead );
	
	return true;
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void BitStream::AlignWriteToByteBoundary( void )
{
	if( m_writeOffset ) {
		m_writeOffset += ( 8 - ( MOD8( m_writeOffset - 1 ) + 1 ) );
	}
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void BitStream::AlignReadToByteBoundary( void )
{
	if( m_readOffset ) {
		m_readOffset += ( 8 - ( MOD8( m_readOffset - 1 ) + 1 ) );
	}
}

// Write numberToWrite bits from the input source
void BitStream::WriteBits( const unsigned char *input,
	int numberOfBitsToWrite, const bool rightAlignedBits )
{
	if( numberOfBitsToWrite < 1 ) {
#ifdef _DEBUG
		assert(false);
#endif
		return;
	}
	
	AddBitsAndReallocate( numberOfBitsToWrite );
	uint32_t offset = 0;
	unsigned char dataByte;
	
	int numberOfBitsUsedMod8 = MOD8( m_writeOffset );
	
	// Faster to put the while at the top surprisingly enough
	while( numberOfBitsToWrite > 0 ) {

		unsigned char* dest = m_data + DIV8( m_writeOffset );
		dataByte = *( input + offset );
		// rightAlignedBits means in the case of a partial byte, the bits are aligned from the right (bit 0) rather than the left (as in the normal internal representation)
		if( numberOfBitsToWrite < 8 && rightAlignedBits ) { 
			// shift left to get the bits on the left, as in our internal representation
			dataByte <<= ( 8 - numberOfBitsToWrite );
		}	
		// Writing to a new byte each time
		if( 0 == numberOfBitsUsedMod8 ) {
			*dest = dataByte;
		} else {
			// Copy over the new data.
			// First half
			*dest |= ( dataByte >> numberOfBitsUsedMod8 );
			// If we didn't write it all out in the first half (8 - (numberOfBitsUsed%8) is the number we wrote in the first half)
			//if( ( 8 - numberOfBitsUsedMod8 ) < 8 && ( 8 - numberOfBitsUsedMod8 ) < numberOfBitsToWrite ) {
			if( ( 8 - numberOfBitsUsedMod8 ) < numberOfBitsToWrite ) {
				// Second half (overlaps byte boundary)
				*( dest + 1 ) = ( unsigned char ) ( dataByte << ( 8 - numberOfBitsUsedMod8 ) ); 
			}
		}
		
		if( numberOfBitsToWrite >= 8 ) {
			m_writeOffset += 8;
		} else {
			m_writeOffset += numberOfBitsToWrite;
		}
		
		numberOfBitsToWrite -= 8;
		
		++offset;
	}
}

// Set the stream to some initial data.  For internal use
void BitStream::SetData(const char* input, const int numberOfBits)
{
#ifdef _DEBUG
	// Make sure the stream is clear
	assert( 0 == m_writeOffset );
#endif
	
	if( numberOfBits < 1 ) {
		return;
	}
		
	AddBitsAndReallocate( numberOfBits );
	
	memcpy( m_data, input, BITS_TO_BYTES( numberOfBits ) );
	
	m_writeOffset = numberOfBits;
}

// Assume the input source points to a native type, compress and write it
void BitStream::WriteCompressed( const unsigned char* input,
	const int size, const bool unsignedData )
{
	if( size < 1 ) {
#ifdef _DEBUG
		assert(false);
#endif
		return;
	}

	int currentByte = DIV8( size - 1 );
	
	unsigned char byteMatch;
	
	if( unsignedData ) {
		byteMatch = 0;
	} else {
		byteMatch = 0xFF;
	}
	
	// Write upper bytes with a single 1
	// From high byte to low byte, if high byte is a byteMatch then write a 1 bit. Otherwise write a 0 bit and then write the remaining bytes
	while( currentByte > 0 ) {
		// If high byte is byteMatch (0 of 0xff) then it would have the same value shifted
		if(input[currentByte] == byteMatch) {
			bool b = true;
			WriteBool( b );
		} else {
			// Write the remainder of the data after writing 0
			bool b = false;
			WriteBool( b );
			
			WriteBits( input, MUL8( currentByte + 1 ), true );

			return;
		}
		
		--currentByte;
	}
	
	// If the upper half of the last byte is a 0 (positive) or 16 (negative) then write a 1 and the remaining 4 bits.  Otherwise write a 0 and the 8 bites.
	if( ( unsignedData && ( input[currentByte] & 0xF0 ) == 0x00 ) 
		|| ( unsignedData == false && ( input[currentByte] & 0xF0 ) == 0xF0 ) )
	{
		bool b = true;
		WriteBool( b );
		WriteBits( input + currentByte, 4, true );
	} else {
		bool b = false;
		WriteBool( b );
		WriteBits( input + currentByte, 8, true );
	}
}

// Read numberOfBitsToRead bits to the output source
// alignBitsToRight should be set to true to convert internal bitstream data to userdata
// It should be false if you used WriteBits with rightAlignedBits false
bool BitStream::ReadBits( unsigned char* output,
	int numberOfBitsToRead, const bool alignBitsToRight )
{
	if( numberOfBitsToRead < 1 ) {
#ifdef _DEBUG
		assert(false);
#endif
		return false;
	}
	
	if( m_readOffset + numberOfBitsToRead > m_writeOffset ) {
		return false;
	}
		
	uint32_t offset = 0;
	
	memset( output, 0, BITS_TO_BYTES( numberOfBitsToRead ) );
	
	int readOffsetMod8 = MOD8( m_readOffset );
	
	// Faster to put the while at the top surprisingly enough
	while( numberOfBitsToRead > 0 ) {
		unsigned char* dest = output + offset;
		unsigned char* src = m_data + DIV8( m_readOffset );
		// First half
		*dest |= *src << readOffsetMod8;
		// If we have a second half, we didn't read enough bytes in the first half
		if( readOffsetMod8 > 0 && numberOfBitsToRead > ( 8 - readOffsetMod8 ) ) {
			 // Second half (overlaps byte boundary)
			*dest |= *( src + 1 ) >> ( 8 - readOffsetMod8 );
		}
			
		numberOfBitsToRead -= 8;
		// Reading a partial byte for the last byte, shift right so the data is aligned on the right
		if( numberOfBitsToRead < 0 ) {
		
			if( alignBitsToRight ) {
				*dest >>= -numberOfBitsToRead;
			}

			m_readOffset += ( 8 + numberOfBitsToRead );
		} else {
			m_readOffset += 8;
		}
		++offset;
	}
	return true;
}

// Assume the input source points to a compressed native type. Decompress and read it
bool BitStream::ReadCompressed( unsigned char* output,
	const int size, const bool unsignedData )
{
	if( size < 1 ) {
		return false;
	}

	int currentByte = DIV8( size - 1 );
	
	
	unsigned char byteMatch, halfByteMatch;
	
	if( unsignedData ) {
		byteMatch = 0;
		halfByteMatch = 0;
	} else {
		byteMatch = 0xFF;
		halfByteMatch = 0xF0;
	}
	
	// Upper bytes are specified with a single 1 if they match byteMatch
	// From high byte to low byte, if high byte is a byteMatch then write a 1 bit. Otherwise write a 0 bit and then write the remaining bytes
	while( currentByte > 0 ) {
		// If we read a 1 then the data is byteMatch.
		bool b = ReadBool();
		// Check that bit
		if( b ) {
			output[currentByte] = byteMatch;
			currentByte--;
		} else {
			// Read the rest of the bytes
			if( ReadBits( output, MUL8( currentByte + 1 ) ) == false ) {
				return false;
			}		
			return true;
		}
	}
	
	// All but the first bytes are byteMatch.  If the upper half of the last byte is a 0 (positive) or 16 (negative) then what we read will be a 1 and the remaining 4 bits.
	// Otherwise we read a 0 and the 8 bytes
	// If this is hit the stream wasn't long enough to read from
	if( m_readOffset + 1 > m_writeOffset ) {
		return false;
	}
		
	bool b = ReadBool();
	// Check that bit
	if( b ) {
		if( ReadBits( output + currentByte, 4 ) == false ) {
			return false;
		}
		// We have to set the high 4 bits since these are set to 0 by ReadBits
		output[currentByte] |= halfByteMatch;
	} else {
		if( ReadBits( output + currentByte, 8 ) == false ) {
			return false;
		}
	}
	return true;
}

// Reallocates (if necessary) in preparation of writing numberOfBitsToWrite
void BitStream::AddBitsAndReallocate( const uint32_t writeBitSize )
{
	if( writeBitSize < 1 ) {
		return;
	}

	uint32_t newAllocBitSize = writeBitSize + m_writeOffset;
	// If we need to allocate 1 or more new bytes
	if( newAllocBitSize > 0 && DIV8( m_allocBitSize - 1 ) < DIV8( newAllocBitSize - 1 ) ) {
		if( m_copyData == false ) {
			// If this assert hits then we need to specify true for the third parameter in the constructor
			// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
			assert(false);
			return;
		} 

		// Less memory efficient but saves on news and deletes
		newAllocBitSize = newAllocBitSize * 2;
		// Use realloc and free so we are more efficient than delete and new for resizing
		uint32_t newAllocByteSize = BITS_TO_BYTES( newAllocBitSize );
		if( m_data == ( unsigned char* )m_stackData ) {
			 if( newAllocByteSize > BITSTREAM_STACK_ALLOCATION_SIZE ) {
				 m_data = ( unsigned char* ) malloc( newAllocByteSize );

				 // need to copy the stack data over to our new memory area too
				 memcpy( (void *)m_data, (void *)m_stackData, BITS_TO_BYTES( m_allocBitSize ) ); 
			 }
		} else {
			m_data = (unsigned char*) realloc( m_data, newAllocBitSize );
		}

#ifdef _DEBUG
		// Make sure realloc succeeded
		assert( m_data );
#endif
	}
	
	if( newAllocBitSize > m_allocBitSize ) {
		m_allocBitSize = newAllocBitSize;
	}
}

// Should hit if reads didn't match writes
void BitStream::AssertStreamEmpty( void )
{
	assert( m_readOffset == m_writeOffset );
}

void BitStream::PrintBits( void ) const
{
	if( m_writeOffset < 1 ) {
		return;
	}
	uint32_t byteSize = BITS_TO_BYTES( m_writeOffset );
	for( uint32_t counter = 0; counter < byteSize; ++counter ) {
		int stop;

		if( counter == DIV8( m_writeOffset - 1 ) ) {
			stop = 8 - ( MOD8( m_writeOffset - 1 ) + 1 );
		} else {
			stop = 0;
		}
			
		for( int counter2 = 7; counter2 >= stop; --counter2 ) {
			if( ( m_data[counter] >> counter2 ) & 1 ) {
				putchar( '1' );
			} else {
				putchar( '0' );
			}
		}
		putchar( ' ' );
	}	
	putchar( '\n' );
}


// Exposes the data for you to look at, like PrintBits does.
// Data will point to the stream.  Returns the length in bits of the stream.
int BitStream::CopyData( unsigned char** data ) const
{
	if( NULL == data ) {
		return 0;
	}

	if( m_writeOffset < 1 ) {
#ifdef _DEBUG
		assert(false);
#endif
		return 0;
	}

	uint32_t byteSize = BITS_TO_BYTES( m_writeOffset );
	*data = new unsigned char [byteSize];
	memcpy( *data, m_data, sizeof( unsigned char ) * byteSize );
	return m_writeOffset;
}

// Ignore data we don't intend to read
void BitStream::IgnoreBits( const int numberOfBits )
{
	m_readOffset += numberOfBits;
}

// Move the write pointer to a position on the array.  Dangerous if you don't know what you are doing!
void BitStream::SetWriteOffset( const uint32_t offset )
{
	m_writeOffset = offset;
}

// Returns the length in bits of the stream
uint32_t BitStream::GetWriteOffset( void ) const
{
	return m_writeOffset;
}

// Returns the length in bytes of the stream
uint32_t BitStream::GetNumberOfBytesUsed( void ) const
{
	return BITS_TO_BYTES( m_writeOffset );
}

// Move the read pointer to a position on the array.
void BitStream::SetReadOffset( const uint32_t offset )
{
	m_readOffset = offset;
}
// Returns the number of bits into the stream that we have read
uint32_t BitStream::GetReadOffset( void ) const
{
	return m_readOffset;
}

// Returns the number of bits left in the stream that haven't been read
uint32_t BitStream::GetNumberOfUnreadBits( void ) const
{
	if( m_writeOffset > m_readOffset ) {
		return m_writeOffset - m_readOffset;
	} else {
		return 0;
	}
}

// Exposes the internal data
const char* BitStream::GetData( void ) const
{
	return ( const char* )m_data;
}

// If we used the constructor version with copy data off, this makes sure it is set to on and the data pointed to is copied.
void BitStream::AssertCopyData( void )
{
	if( m_copyData == false ) {
		m_copyData = true;
		
		if( m_allocBitSize > 0 ) {
			uint32_t allocBitSize = BITS_TO_BYTES( m_allocBitSize );
			unsigned char* newdata = (unsigned char*) malloc( allocBitSize );

#ifdef _DEBUG		
			assert(m_data);
#endif
			memcpy( newdata, m_data, allocBitSize );
			m_data = newdata;
		} else {
			m_data = NULL;
		}
	}
}

} // end namespace ntwk

