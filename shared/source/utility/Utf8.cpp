#include "Utf8.h"   
#include "stdlib.h"    
#include <stdio.h>   
#include <wchar.h>   
#include <exception>   
#include <assert.h>   
#include <string.h>     
#if defined(WIN32)      
#include <windows.h>   
#pragma warning(disable : 4244)        
#endif   
   
   
namespace util {

#define STRING_BUF_SIZE 512

    template<class T>
    class StringBuffer { 
    public:
        StringBuffer() { 
            maxUsed = 0;
            buf = smallBuf;
        }
        StringBuffer(size_t size) { 
            if (size < STRING_BUF_SIZE) { 
                buf = smallBuf;
            } else { 
                buf = new T[size+1];
            }
            maxUsed = size;
        }

        ~StringBuffer() { 
            if (buf != smallBuf) { 
                delete[] buf;
                buf = NULL;
            }
        }

        void Put(size_t size) {

            if(size > maxUsed) {
                if (buf != smallBuf) { 
                    delete[] buf;
                }
                if (size < STRING_BUF_SIZE) { 
                    buf = smallBuf;
                } else { 
                    buf = new T[size+1];
                }
                maxUsed = size;
            }

        }

        operator T*() { 
            return buf; 
        }

        operator const T*() const { 
            return buf; 
        }

        T& operator[] (const int index) {
            return buf[index];
        }

        const T& operator[] (const int index) const {
            return buf[index];
        }

        T& operator[] (const size_t index) {
            return buf[index];
        }

        const T& operator[] (const size_t index) const {
            return buf[index];
        }

    protected:
        T   smallBuf[STRING_BUF_SIZE];
        T*  buf;
        size_t maxUsed;
    };

    class UTF8ConvertError : public std::exception{   
    public:   
        virtual castr what() const throw(){   
            return "UTF8ConvertError";   
        }   
    };  

    class XCharError : public std::exception{   
    public:   
        XCharError(xchar ch){   
            m_ch = ch;   
            ::sprintf(m_msg,"XCharError:%i",ch);   
        }   
        virtual castr what() const throw(){   
            return m_msg;   
        }   
    private:   
        char m_msg[100];   
        xchar m_ch;   
    };  

    class UTF8FormatError : public std::exception{   
    public:   
        UTF8FormatError(unsigned long pos,unsigned long  len,uint8_t byte,unsigned long  index){   
            m_len = len;   
            m_pos = pos;   
            m_byte= byte;   
            m_index=index;   
        }   
        UTF8FormatError(unsigned long index){   
            m_len = 0;   
            m_pos = 0;   
            m_byte= 0;   
            m_index=index;   
        }   
    public:   
        virtual const char* what() const throw(){   
            return "UTF8FormatError";   
        }   
    private:   
        unsigned long  m_len;   
        unsigned long  m_pos;   
        uint8_t   m_byte;   
        unsigned long  m_index;   
    };   
       
    unsigned int XCharToUTF8(xchar ch,astr utf8){   
        unsigned int encodedBytes;   
        if(ch <= 127){   
            *utf8 = (char)ch;   
            encodedBytes = 1;   
        }else{   
            uchar *chars = (ustr)utf8;   
            uchar *outPtr = chars;   
            // Figure out how many bytes we need   
               
            if (ch < 0x80)   
                encodedBytes = 1;   
            else if (ch < 0x800)   
                encodedBytes = 2;   
            else if (ch < 0x10000)   
                encodedBytes = 3;   
            else if (ch < 0x200000)   
                encodedBytes = 4;   
            else if (ch < 0x4000000)   
                encodedBytes = 5;   
            else if (ch <= 0x7FFFFFFF)   
                encodedBytes = 6;   
            else{   
                   
                throw XCharError(ch);   
            }   
   
            //   
            //  And spit out the bytes. We spit them out in reverse order   
            //  here, so bump up the output pointer and work down as we go.   
            //   
            outPtr += encodedBytes;   
            switch(encodedBytes){   
                case 6 : *--outPtr = uint8_t((ch | 0x80UL) & 0xBFUL);   
                        ch >>= 6;   
                case 5 : *--outPtr = uint8_t((ch | 0x80UL) & 0xBFUL);   
                        ch >>= 6;   
                case 4 : *--outPtr = uint8_t((ch | 0x80UL) & 0xBFUL);   
                        ch >>= 6;   
                case 3 : *--outPtr = uint8_t((ch | 0x80UL) & 0xBFUL);   
                        ch >>= 6;   
                case 2 : *--outPtr = uint8_t((ch | 0x80UL) & 0xBFUL);   
                        ch >>= 6;   
                case 1 : *--outPtr = uint8_t(ch | gFirstByteMark[encodedBytes]);   
            }   
        }   
        return encodedBytes;   
    }   
    
    unsigned int UTF8ToXChar(custr utf8, xchar &ch){   
        const unsigned char *srcPtr = (const unsigned char*)utf8;   
        if (*srcPtr <= 127){   
            return *srcPtr;   
        }   
        unsigned int trailingBytes = gUTFBytes[*srcPtr];   
   
        xchar tmpVal = *srcPtr++;   
        tmpVal <<= 6;   
        for(unsigned int i=1; i<trailingBytes; i++){   
            if((*srcPtr & 0xC0) == 0x80){   
                tmpVal += *srcPtr++;    
                tmpVal <<= 6;   
            }else throw UTF8FormatError(i,trailingBytes,*srcPtr,0xffffffff);   
        }   
        if((*srcPtr & 0xC0) == 0x80){   
            tmpVal += *srcPtr++;   
        }else throw UTF8FormatError(trailingBytes,trailingBytes,*srcPtr,0xffffffff);   
           
        tmpVal -= gUTFOffsets[trailingBytes];   
   
        //   
        //  If it will fit into a single char, then put it in. Otherwise   
        //  encode it as a surrogate pair. If its not valid, use the   
        //  replacement char.   
        //   
        if (tmpVal & 0xFFFF0000){   
            // Store the leading surrogate char   
            tmpVal -= (xchar)0x10000;   
        }   
        ch = tmpVal;   
        return trailingBytes+1;   
    } 

    xchar UTF8Value(custr str){   
        unsigned char *srcPtr = (unsigned char*)str;   
        if (*srcPtr <= 127){   
            return *srcPtr;   
        }   
        unsigned int trailingBytes = gUTFBytes[*srcPtr];   
   
        xchar tmpVal = *srcPtr++;   
        tmpVal <<= 6;   
        for(unsigned int i=1; i<trailingBytes; i++){   
            if((*srcPtr & 0xC0) == 0x80){   
                tmpVal += *srcPtr++;    
                tmpVal <<= 6;   
            }else throw UTF8FormatError(i,trailingBytes,*srcPtr,0xffffffff);   
        }   
        if((*srcPtr & 0xC0) == 0x80){   
            tmpVal += *srcPtr++;   
        }else throw UTF8FormatError(trailingBytes,trailingBytes,*srcPtr,0xffffffff);   
           
        tmpVal -= gUTFOffsets[trailingBytes];   
   
        //   
        //  If it will fit into a single char, then put it in. Otherwise   
        //  encode it as a surrogate pair. If its not valid, use the   
        //  replacement char.   
        //   
        if (tmpVal & 0xFFFF0000){   
            // Store the leading surrogate char   
            tmpVal -= (xchar)0x10000;   
        }   
        return tmpVal;   
    }   
   
    int UTF8StrCmp(castr str1, castr str2, size_t count) {   
        if(count == -1)   
            return ::strcmp((char*)str1, (char*)str2);      
        else{   
            size_t len1 = strlen((char*)str1);
            size_t len2 = strlen((char*)str2);
            size_t len = (len1 < len2) ? len1 : len2;
            len = (len < count) ? len : count;   
   
            int ret = memcmp(str1, str2, len);   
            if(ret == 0){   
                if(len1 > len2) ret = 1;   
                else if(len1 < len2) ret = -1;   
            }   
            return ret;   
        }   
    }

    void UTF8StrLen(castr str,unsigned int &size,unsigned int &rawSize,unsigned int count){   
        assert(str != NULL);   
        rawSize = 0;   
        size = 0;   
        custr p=(custr)str;   
        for(;*p!=0 && rawSize<count;p++){   
            if(*p < 0x80 || *p >= 0xE0) size++;   
            rawSize++;   
        }   
    }   
   
   
    // ---------------------------------------------------------------------------   
    //  Local static data   
    //   
    //  gUTFBytes   
    //      A list of counts of trailing bytes for each initial byte in the input.   
    //   
    //  gUTFByteIndicator   
    //      For a UTF8 sequence of n bytes, n>=2, the first byte of the   
    //      sequence must contain n 1's followed by precisely 1 0 with the   
    //      rest of the byte containing arbitrary bits.  This array stores   
    //      the required bit pattern for validity checking.   
    //  gUTFByteIndicatorTest   
    //      When bitwise and'd with the observed value, if the observed   
    //      value is correct then a result matching gUTFByteIndicator will   
    //      be produced.   
    //   
    //  gUTFOffsets   
    //      A list of values to offset each result char type, according to how   
    //      many source bytes when into making it.   
    //   
    //  gFirstByteMark   
    //      A list of values to mask onto the first byte of an encoded sequence,   
    //      indexed by the number of bytes used to create the sequence.   
    // ---------------------------------------------------------------------------   
    SHARED_DLL_DECL cuchar gUTFBytes[256] =   
    {   
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   
        ,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   
        ,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1   
        ,   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2   
        ,   3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5   
    };   
   
    static cuchar gUTFByteIndicator[6] =   
    {   
        (cuchar)0x00, (cuchar)0xC0, (cuchar)0xE0, (cuchar)0xF0, (cuchar)0xF8, (cuchar)0xFC   
    };   
    static cuchar gUTFByteIndicatorTest[6] =   
    {   
        (cuchar)0x80, (cuchar)0xE0, (cuchar)0xF0, (cuchar)0xF8, (cuchar)0xFC, (cuchar)0xFE   
    };   
   
    SHARED_DLL_DECL const unsigned long gUTFOffsets[6] =   
    {   
        0, 0x3080, 0xE2080, 0x3C82080, 0xFA082080, 0x82082080   
    };   
   
    SHARED_DLL_DECL cuchar gFirstByteMark[7] =   
    {   
        (cuchar)0x00, (cuchar)0x00, (cuchar)0xC0, (cuchar)0xE0, (cuchar)0xF0, (cuchar)0xF8, (cuchar)0xFC   
    };   
   
    size_t ANSIToUNICODE(castr srcData,size_t srcCount, wstr destData,size_t destCount){   
        int ret;   
#if defined(_MSC_VER)   
        ret = ::MultiByteToWideChar(CP_ACP,0,srcData,(int)srcCount,destData,(int)destCount);   
        if(ret == 0) ret = -1;   
        return ret;   
#else   
        ret = ::mbstowcs(destData, srcData,destCount);   
#endif   
        return (unsigned int)ret;   
    }   

    size_t UNICODEToANSI(cwstr srcData,size_t srcCount,astr destData,size_t destCount){   
        int ret;   
#if defined(_MSC_VER)   
        ret = ::WideCharToMultiByte(CP_ACP,0,srcData,(int)srcCount,
            destData,(int)destCount, NULL, NULL);   
#else   
        ret = ::wcstombs(destData,srcData,destCount);   
#endif   
        return (unsigned int)ret;   
    }   
   
    // ---------------------------------------------------------------------------   
    //  XMLUTF8Transcoder: Implementation of the transcoder API   
    // ---------------------------------------------------------------------------   
    size_t ANSIToUTF8(castr srcData,size_t srcCount,ustr destData,size_t destCount){   
        StringBuffer<wchar_t> wstr(srcCount*2);   
        size_t len = 0;   
        int ret = 0;   
        try{   
#if defined(WIN32)   
        ret = ::MultiByteToWideChar(CP_ACP,0,srcData,(int)srcCount,
            (wchar_t*)wstr,(int)destCount);
        if(ret == 0) ret = -1;
#else   
        ret = ::mbstowcs((wchar_t*)wstr, srcData,srcCount);   
#endif   
            if(ret < 0){   
                throw UTF8ConvertError();   
            }   
            len = UNICODEToUTF8((wchar_t*)wstr,ret,destData,destCount);   
        }catch(...){    
            throw;   
        }     
        return len;   
    }   

    size_t UTF8ToANSI(custr srcData,size_t srcCount, astr destData,size_t destCount){  
        StringBuffer<wchar_t> wstr((destCount+1)*2);
        int ret = 0;   
        try{   
            size_t len = UTF8ToUNICODE(srcData,srcCount,(wchar_t*)wstr,destCount);   
            wstr[len] = 0;   
#if defined(_MSC_VER)   
        ret = ::WideCharToMultiByte(CP_ACP,0,(wchar_t*)wstr,(int)len,destData,
            (int)len*2, NULL, NULL );   
#else   
        ret = ::wcstombs(destData,(wchar_t*)wstr,len*2);   
#endif    
            if(ret < 0){
                throw UTF8ConvertError(); 
                return ret;
            }   
        }catch(...){    
            throw;   
        }   
        return ret;   
    }   

    size_t UTF8ToUNICODE(custr srcData,size_t srcCount, wstr destData, size_t destCount){   
        // Watch for pathological scenario. Shouldn't happen, but...   
        if (!srcCount || !destCount)   
            return 0;   
   
        //   
        //unsigned char charSizes[1024];   
        unsigned int  bytesEaten;   
   
        //   
        //  Get pointers to our start and end points of the input and output   
        //  buffers.   
        //   
        custr  srcPtr = srcData;   
        custr  srcEnd = srcPtr + srcCount;   
        wstr   outPtr = destData;   
        wstr   outEnd = outPtr + destCount;   
        //ustr   sizePtr = charSizes;   
   
   
   
        //   
        //  We now loop until we either run out of input data, or room to store   
        //  output chars.   
        //   
        while ((srcPtr < srcEnd) && (outPtr < outEnd))   
        {   
            // Special-case ASCII, which is a leading byte value of <= 127   
            if (*srcPtr <= 127)   
            {   
                *outPtr++ = wchar_t(*srcPtr++);   
                //*sizePtr++ = 1;   
                continue;   
            }   
   
            // See how many trailing src bytes this sequence is going to require   
            const unsigned int trailingBytes = gUTFBytes[*srcPtr];   
   
            //   
            //  If there are not enough source bytes to do this one, then we   
            //  are done. Note that we done >= here because we are implicitly   
            //  counting the 1 byte we get no matter what.   
            //   
            //  If we break out here, then there is nothing to undo since we   
            //  haven't updated any pointers yet.   
            //   
            if (srcPtr + trailingBytes >= srcEnd)   
                break;   
   
            // Looks ok, so lets build up the value   
            // or at least let's try to do so--remembering that   
            // we cannot assume the encoding to be valid:   
   
            // first, test first byte   
            if((cuchar)(gUTFByteIndicatorTest[trailingBytes] & *srcPtr) != gUTFByteIndicator[trailingBytes])    
                throw UTF8FormatError(0,trailingBytes,*srcPtr,srcPtr-srcData);   
   
            unsigned long tmpVal = *srcPtr++;   
            tmpVal <<= 6;   
            for(unsigned int i=1; i<trailingBytes; i++)    
            {   
                if((*srcPtr & 0xC0) == 0x80)    
                {   
                    tmpVal += *srcPtr++;    
                    tmpVal <<= 6;   
                }else throw UTF8FormatError(i,trailingBytes,*srcPtr,srcPtr-srcData);   
            }   
            if((*srcPtr & 0xC0) == 0x80){   
                tmpVal += *srcPtr++;   
            }else throw UTF8FormatError(trailingBytes,trailingBytes,*srcPtr,srcPtr-srcData);   
   
            // since trailingBytes comes from an array, this logic is redundant   
            //  default :   
            //      ThrowXML(TranscodingException, XMLExcepts::Trans_BadSrcSeq);   
            //}   
            tmpVal -= gUTFOffsets[trailingBytes];   
   
            //   
            //  If it will fit into a single char, then put it in. Otherwise   
            //  encode it as a surrogate pair. If its not valid, use the   
            //  replacement char.   
            //   
            if (!(tmpVal & 0xFFFF0000)) {   
                //*sizePtr++ = trailingBytes + 1;   
                *outPtr++ = wchar_t(tmpVal);   
            }else if (tmpVal > 0x10FFFF){   
                //   
                //  If we've gotten more than 32 chars so far, then just break   
                //  out for now and lets process those. When we come back in   
                //  here again, we'll get no chars and throw an exception. This   
                //  way, the error will have a line and col number closer to   
                //  the real problem area.   
                //   
                if ((outPtr - destData) > 32)   
                    break;   
   
                throw UTF8FormatError(srcPtr-srcData);   
            }else{   
                //   
                //  If we have enough room to store the leading and trailing   
                //  chars, then lets do it. Else, pretend this one never   
                //  happened, and leave it for the next time. Since we don't   
                //  update the bytes read until the bottom of the loop, by   
                //  breaking out here its like it never happened.   
                //   
                if (outPtr + 1 >= outEnd)   
                    break;   
   
                // Store the leading surrogate char   
                tmpVal -= 0x10000;   
                //*sizePtr++ = trailingBytes + 1;   
                *outPtr++ = wchar_t((tmpVal >> 10) + 0xD800);   
   
                //   
                //  And then the treailing char. This one accounts for no   
                //  bytes eaten from the source, so set the char size for this   
                //  one to be zero.   
                //   
                //*sizePtr++ = 0;   
                *outPtr++ = wchar_t(tmpVal & 0x3FF) + 0xDC00;   
            }   
        }   
   
        // Update the bytes eaten   
        bytesEaten = (unsigned char*)srcPtr - (unsigned char*)srcData;   
   
        // Return the characters read   
        return outPtr - destData;   
    }   
   
    size_t UNICODEToUTF8(cwstr srcData,size_t srcCount,ustr destData,size_t destCount){   
        // Watch for pathological scenario. Shouldn't happen, but...   
        if (!srcCount || !destCount)   
            return 0;   
   
        //   
         unsigned int   charsEaten;   
   
        //   
        //  Get pointers to our start and end points of the input and output   
        //  buffers.   
        //   
        cwstr    srcPtr = srcData;   
        cwstr    srcEnd = srcPtr + srcCount;   
        ustr     outPtr = destData;   
        ustr     outEnd = destData + destCount;   
   
        while (srcPtr < srcEnd)   
        {   
            //   
            //  Tentatively get the next char out. We have to get it into a   
            //  32 bit value, because it could be a surrogate pair.   
            //   
            unsigned long curVal = *srcPtr;   
   
            //   
            //  If its a leading surrogate, then lets see if we have the trailing   
            //  available. If not, then give up now and leave it for next time.   
            //   
            unsigned int srcUsed = 1;   
            if ((curVal >= 0xD800) && (curVal <= 0xDBFF))   
            {   
                if (srcPtr + 1 >= srcEnd)   
                    break;   
   
                // Create the composite surrogate pair   
                curVal = ((curVal - 0xD800) << 10)   
                        + ((*(srcPtr + 1) - 0xDC00) + 0x10000);   
   
                // And indicate that we ate another one   
                srcUsed++;   
            }   
   
            // Figure out how many bytes we need   
            unsigned int encodedBytes;   
            if (curVal < 0x80)   
                encodedBytes = 1;   
            else if (curVal < 0x800)   
                encodedBytes = 2;   
            else if (curVal < 0x10000)   
                encodedBytes = 3;   
            else if (curVal < 0x200000)   
                encodedBytes = 4;   
            else if (curVal < 0x4000000)   
                encodedBytes = 5;   
            else if (curVal <= 0x7FFFFFFF)   
                encodedBytes = 6;   
            else   
            {   
                assert(0);   
                // Else, use the replacement character   
                //*outPtr++ = chSpace;   
                //srcPtr += srcUsed;   
                continue;   
            }   
   
            //   
            //  If we cannot fully get this char into the output buffer,   
            //  then leave it for the next time.   
            //   
            if (outPtr + encodedBytes > outEnd)   
                break;   
   
            // We can do it, so update the source index   
            srcPtr += srcUsed;   
   
            //   
            //  And spit out the bytes. We spit them out in reverse order   
            //  here, so bump up the output pointer and work down as we go.   
            //   
            outPtr += encodedBytes;   
            switch(encodedBytes)   
            {   
                case 6 : *--outPtr = uint8_t((curVal | 0x80UL) & 0xBFUL);   
                        curVal >>= 6;   
                case 5 : *--outPtr = uint8_t((curVal | 0x80UL) & 0xBFUL);   
                        curVal >>= 6;   
                case 4 : *--outPtr = uint8_t((curVal | 0x80UL) & 0xBFUL);   
                        curVal >>= 6;   
                case 3 : *--outPtr = uint8_t((curVal | 0x80UL) & 0xBFUL);   
                        curVal >>= 6;   
                case 2 : *--outPtr = uint8_t((curVal | 0x80UL) & 0xBFUL);   
                        curVal >>= 6;   
                case 1 : *--outPtr = uint8_t   
                        (   
                            curVal | gFirstByteMark[encodedBytes]   
                        );   
            }   
   
            // Add the encoded bytes back in again to indicate we've eaten them   
            outPtr += encodedBytes;   
        }   
   
        // Fill in the chars we ate   
        charsEaten = (srcPtr - srcData);   
   
        // And return the bytes we filled in   
        return (outPtr - destData);   
    }   
   
    std::string UTF8ToANSI(const std::string &srcData){ 
        StringBuffer<char> destData(srcData.size()+1);
        size_t len = UTF8ToANSI((uchar*)srcData.c_str(),srcData.size()
            ,(char*)destData,srcData.size()+1);   
        destData[len] = 0;   
        return (char*)destData;   
    }

    std::string ANSIToUTF8(const std::string &srcData){   
        StringBuffer<uchar> destData(srcData.size()*3+1);   
        size_t len = ANSIToUTF8(srcData.c_str(),srcData.size(),
            (uchar*)destData,srcData.size()*3+1);   
        destData[len] = 0;   
        return (char*)(uchar*)destData;   
    }   
}   
   

