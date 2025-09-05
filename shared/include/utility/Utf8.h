
#ifndef UTF8_H
#define UTF8_H

#include "Common.h"
#include <string>
 
namespace util { 

    typedef const char cuchar;
    typedef unsigned char uchar;
    typedef wchar_t xchar;
    typedef const char* castr;
    typedef char* astr;
    typedef const unsigned char* custr;
    typedef unsigned char* ustr;
    typedef const wchar_t* cwstr;
    typedef wchar_t* wstr;
    
	SHARED_DLL_DECL size_t ANSIToUTF8(castr srcData,size_t srcCount,ustr destData,size_t destCount); 
	SHARED_DLL_DECL size_t UTF8ToANSI(custr srcData,size_t srcCount, astr destData,size_t destCount); 
	SHARED_DLL_DECL size_t ANSIToUNICODE(castr srcData,size_t srcCount, wstr destData,size_t destCount); 
	SHARED_DLL_DECL size_t UNICODEToANSI(cwstr srcData,size_t srcCount,astr destData,size_t destCount); 
	SHARED_DLL_DECL size_t UNICODEToUTF8(cwstr srcData,size_t srcCount,ustr destData,size_t destCount); 
	SHARED_DLL_DECL size_t UTF8ToUNICODE(custr srcData,size_t srcCount, wstr destData,size_t destCount); 
 
 
    SHARED_DLL_DECL std::string UTF8ToANSI(const std::string &srcData); 
    SHARED_DLL_DECL std::string ANSIToUTF8(const std::string &srcData); 
 
	//把一个xchar转换成UTF-8编码 
	//parameters: 
	//	ch   任意的字符 
	//	utf8 不小于6字节的缓冲区,存储ch转换的结果 
	//return: 
	//	ch 转换成utf8后占用的字节数 
	SHARED_DLL_DECL unsigned int XCharToUTF8(xchar ch, astr utf8); 
	//把一个utf8字符转换成任意字符xchar 
	//parameters: 
	//	utf8  存储一个字符的UTF-8格式 
	//	ch	  用于存储utf8转换的结果 
	//return: 
	//	utf8 表示的一个字符占用的字节数 
	SHARED_DLL_DECL unsigned int UTF8ToXChar(custr utf8, xchar &ch); 
	 
 
	//UTF-8字符串操作函数 
	//默认情况下此函数以标准库的strcmp函数实现，我们也可以设置系统参数 
	//使得strcmp以更为人性的方式比较字符串。比如以逻辑的字符值或者是按照 
	//汉字拼音比较。 
	SHARED_DLL_DECL int	UTF8StrCmp(castr str1, castr str2, unsigned int count= (unsigned int)-1); 
	//返回值 
	//size    str的逻辑长度,字符数 
	//rawSize str的物理长度,以0结尾的缓冲区长度 
	SHARED_DLL_DECL void UTF8StrLen(castr str,unsigned int &size,unsigned int &rawSize,unsigned int count= (unsigned int)-1); 
	SHARED_DLL_DECL xchar UTF8Value(custr str); 
 
	extern SHARED_DLL_DECL cuchar gUTFBytes[256];
	extern SHARED_DLL_DECL cuchar gFirstByteMark[7];
	extern SHARED_DLL_DECL const unsigned long gUTFOffsets[6]; 
 
 
	///////////////////////////////////////// 
	//utf8字符跌代器 
	class SHARED_DLL_DECL UTF8ConstIterator{ 
	public: 
		typedef xchar		value_type; 
		typedef value_type	reference; 
		typedef UTF8ConstIterator MyType; 
	public: 
		UTF8ConstIterator(castr it):m_it((ustr)it){ 
            //如果it刚好在一个utf8字符的中间，则向前边推算出这个字符 
			for(;*m_it >= 0x80 && *m_it < 0xE0 && (ustr)it - m_it < 6;--m_it); 
		} 
		reference operator*() const{ 
			return *m_it <= 127 ? *m_it : UTF8Value(m_it); 
		} 
		operator castr () const{ 
			return (castr)m_it; 
		} 
        int GetCharSize() const{ 
            return gUTFBytes[*m_it]+1; 
        } 
		long operator - (const UTF8ConstIterator &other) const{ 
			//long size; 
			//for(utf8_const_iterator it=*this;it!=other;it++) 
			//return (size_t)(m_it-it.m_it); 
			return 0; 
		} 
 
        MyType operator + (int offset) const{ 
            if(offset < 0) 
                return operator-(-offset); 
 
            const uchar *str = m_it; 
            for(int i=0;i<offset;i++) 
                str += gUTFBytes[*str]+1; 
            return MyType((castr)str); 
        } 
        MyType operator - (int offset) const{ 
            if(offset < 0) 
                return operator+(-offset); 
 
            const uchar *str = m_it; 
            for(int i=0;i<offset;i++) 
                for(--str;*str >= 0x80 && *str < 0xE0;--str); 
            return MyType((castr)str); 
        } 
		MyType& operator ++(){ 
			m_it += gUTFBytes[*m_it]+1; 
			return *this; 
		} 
		MyType operator ++(int){ 
			MyType tmp = *this; 
			m_it += gUTFBytes[*m_it]+1; 
			return tmp; 
		} 
		MyType& operator --(){ 
			//如果是在1000 0000(0x80)和1100 0000(0xE0)之间的值就忽略掉 
			for(--m_it;*m_it >= 0x80 && *m_it < 0xE0;--m_it); 
			return *this; 
		} 
		MyType operator --(int){ 
			MyType tmp = *this; 
			for(--m_it;*m_it >= 0x80 && *m_it < 0xE0;--m_it); 
			return tmp; 
		} 
		bool operator == (const MyType &right){ 
			return m_it==right.m_it; 
		} 
		bool operator != (const MyType &right){ 
			return !(*this == right); 
		} 
		bool operator<(const MyType &right) const{ 
			return m_it<right.m_it; 
		} 
		bool operator>(const MyType& right) const{ 
			return (right < *this); 
		} 
		bool operator<=(const MyType& right) const{ 
			return (!(right < *this)); 
		} 
		bool operator>=(const MyType& right) const{ 
			return (!(*this < right)); 
		} 
	protected: 
		ustr m_it; 
	};

	class SHARED_DLL_DECL UTF8Iterator : public UTF8ConstIterator{ 
	public: 
		typedef UTF8Iterator MyType; 
	public: 
		UTF8Iterator(castr it):UTF8ConstIterator(it){ 
		} 
		MyType& operator ++(){ 
			m_it += gUTFBytes[*m_it]+1; 
			return *this; 
		} 
		MyType operator ++(int){ 
			MyType temp = *this; 
			m_it += gUTFBytes[*m_it]+1; 
			return temp; 
		} 
		MyType& operator --(){ 
			for(--m_it;*m_it >= 0x80 && *m_it < 0xE0;--m_it); 
			return *this; 
		} 
		MyType operator --(int){ 
			MyType tmp = *this; 
			for(--m_it;*m_it >= 0x80 && *m_it < 0xE0;--m_it); 
			return tmp; 
		} 
	}; 
 
 
	//////////////////////////////////// 
	//反向跌代器 
	template<class BaseType> 
	class SHARED_DLL_DECL UTF8ReverseBidirectionalIterator{ 
	public: 
		typedef typename BaseType::value_type    value_type; 
		typedef typename BaseType::reference 	reference; 
		typedef UTF8ReverseBidirectionalIterator MyType; 
	public: 
		UTF8ReverseBidirectionalIterator(const BaseType &it):m_it(it){} 
		BaseType Base() const{ 
			return (m_it); 
		} 
 
		reference operator*() const{ 
			return *m_it; 
		} 
		MyType& operator ++(){ 
			--m_it; 
			return *this; 
		} 
		MyType operator ++(int){ 
			MyType tmp = *this; 
			--m_it; 
			return tmp; 
		} 
		MyType& operator --(){ 
			++m_it; 
			return *this; 
		} 
		MyType operator --(int){ 
			MyType tmp = *this; 
			++m_it; 
			return tmp; 
		} 
		bool operator == (const MyType &right){ 
			return m_it==right.m_it; 
		} 
		bool operator != (const MyType &right){ 
			return !(*this == right); 
		} 
		bool operator<(const MyType &right) const{ 
			return m_it<right.m_it; 
		} 
		bool operator>(const MyType& right) const{ 
			return (right < *this); 
		} 
		bool operator<=(const MyType& right) const{ 
			return (!(right < *this)); 
		} 
		bool operator>=(const MyType& right) const{ 
			return (!(*this < right)); 
		} 
	private: 
		BaseType m_it; 
	}; 
	typedef UTF8ReverseBidirectionalIterator<UTF8Iterator> utf8_reverse_iterator; 
	typedef UTF8ReverseBidirectionalIterator<UTF8ConstIterator> utf8_const_reverse_iterator; 
} 
 
#endif // UTF8_H 
