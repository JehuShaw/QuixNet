/* 
 * File:   SmallBuffer.h
 * Author: Jehu Shaw
 *
 * Created on 2011_7_2 9:47
 */

#ifndef SMALLBUFFER_H
#define	SMALLBUFFER_H

#include <stddef.h>

namespace ntwk
{
#define SMALL_BUF_SIZE 512

    class SmallBuffer { 
    public:
        /**
         * 默认构造函�?         
		 */
        SmallBuffer() { 
			maxUsed = 0;
			buf = smallBuf;
        }

        SmallBuffer(const SmallBuffer& orig) { 
            if (orig.maxUsed < SMALL_BUF_SIZE) { 
                buf = smallBuf;
            } else { 
                buf = new char[orig.maxUsed+1];
            }
            maxUsed = orig.maxUsed;
        }
        /**
         * 构造函�?         * @param size 预分配大�?         
		 */
        SmallBuffer(size_t size) { 
			if (size < SMALL_BUF_SIZE) { 
				buf = smallBuf;
			} else { 
				buf = new char[size+1];
			}
			maxUsed = size;
        }
        /**
         * 虚构函数
         */
        ~SmallBuffer() { 
            if (buf != smallBuf) { 
                delete[] buf;
				buf = NULL;
            }
        }
        /**
         * 改变buffer大小
         * @param size
         */
        void Put(size_t size) { 

			if(size > maxUsed) {
				if (buf != smallBuf) { 
					delete[] buf;
				}
				if (size < SMALL_BUF_SIZE) { 
					buf = smallBuf;
				} else { 
					buf = new char[size+1];
				}
				maxUsed = size;
			}
        }
        /**
         * 强制类型转换，获取buffer指针
         * @return 
         */
        operator char*() { 
            return buf; 
        }

		operator const char*() const { 
			return buf; 
		}

		char& operator[] (const int index) {
			return buf[index];
		}

		const char& operator[] (const int index) const {
			return buf[index];
		}

    protected:
        char   smallBuf[SMALL_BUF_SIZE];
		char*  buf;
		size_t maxUsed;
    };
}
#endif	/* SMALLBUFFER_H */

