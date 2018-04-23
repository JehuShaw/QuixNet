
/*
 *This class implemented BitMap with the memory management using char* type
 *it provide three public functions:
 *
 *getBit(unsigned long index) get the bit value in position index
 *
 *setBit(unsigned long index, bool value) set the bit value in position index
 *
 *printBitSet() print the bits
 *
 */
#ifndef _BIT_SET_H_
#define _BIT_SET_H_

#include <stdint.h>
#include "Common.h"

namespace util {

	class SHARED_DLL_DECL BitSignSet{
		public:
			bool GetBit(unsigned long index);
			void SetBit(unsigned long index, bool value);
			inline unsigned long FindFirst0() {
				return FindFirst0(0);
			}
			inline unsigned long FindFirst1() {
				return FindFirst1(0);
			}
			unsigned long FindFirst0(unsigned long offset);
			unsigned long FindFirst1(unsigned long offset);
			void PrintBitSet(std::string& outStr);

			inline unsigned long BitSize() {
				return m_uBitSize;
			}

			inline unsigned long ByteSize() {
				return BitToByteSize(m_uBitSize);
			}

			void OrBitSet(const BitSignSet& right);

			void ResetBitSet();
		/*
		 * the construction function
		 */
		public:
			BitSignSet();
			BitSignSet(unsigned long uBitSize);
			BitSignSet(const BitSignSet& orig);
			~BitSignSet();

			BitSignSet& operator = (const BitSignSet& right);
	
		private:
			static unsigned long BitToByteSize(unsigned long value);

			static inline uint8_t Set1(uint8_t data, int number){
				return data | 0x1 << number;
			}

			static inline uint8_t Set0(uint8_t data, int number){
				return data & ~(0x1 << number);
			}

			static inline bool GetByteBit(uint8_t data, int number){
				return (data >> number) & 0x1;
			}

			static inline void PrintChar(std::string& outStr, uint8_t data) {

				char buf[9] = {'\0'};
				int i = 7;
				while(i >= 0){
					buf[i--]=((data & 0x1) == 0 ? '0' : '1');
					data >>= 1;
				} 
				buf[8] = '\0';
				outStr += buf;
			}

			static inline void PrintChar(std::string& outStr, uint8_t data, int len) {

				if(len > 8 || len < 1) {
					return;
				}
				data >>= 8 - len;
				//just print the first len bit	
				char buf[9] = {'\0'};
				int i = len - 1;
				while(i >= 0){
					buf[i--] = ((data & 0x1) == 0 ? '0' : '1');
					data >>= 1;
				}   
				buf[len] = '\0';
				outStr += buf;
			}

	private:
		unsigned long m_uBitSize;
		uint32_t m_stackBuf;
		uint8_t* m_buf;
	};

}

#endif /* _BIT_SET_H_ */
