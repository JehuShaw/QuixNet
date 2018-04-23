//md5.hÎÄ¼þ
/******************************************************************************
*  Copyright (C) 2000 by Robert Hubley.                                      *
*  All rights reserved.                                                      *
*                                                                            *
*  This software is provided ``AS IS'' and any express or implied            *
*  warranties, including, but not limited to, the implied warranties of      *
*  merchantability and fitness for a particular purpose, are disclaimed.     *
*  In no event shall the authors be liable for any direct, indirect,         *
*  incidental, special, exemplary, or consequential damages (including, but  *
*  not limited to, procurement of substitute goods or services; loss of use, *
*  data, or profits; or business interruption) however caused and on any     *
*  theory of liability, whether in contract, strict liability, or tort       *
*  (including negligence or otherwise) arising in any way out of the use of  *
*  this software, even if advised of the possibility of such damage.         *
*                                                                            *
******************************************************************************
MD5.H - header file for MD5C.C
Port to Win32 DLL by Robert Hubley 1/5/2000
Original Copyright:
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.
License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.
License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.
RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.
These notices must be retained in any copies of any part of this
documentation and/or software.
*/
/******************************************************************************
*  Copyright (C) 2000 by Robert Hubley.                                      *
*  All rights reserved.                                                      *
*                                                                            *
*  This software is provided ``AS IS'' and any express or implied            *
*  warranties, including, but not limited to, the implied warranties of      *
*  merchantability and fitness for a particular purpose, are disclaimed.     *
*  In no event shall the authors be liable for any direct, indirect,         *
*  incidental, special, exemplary, or consequential damages (including, but  *
*  not limited to, procurement of substitute goods or services; loss of use, *
*  data, or profits; or business interruption) however caused and on any     *
*  theory of liability, whether in contract, strict liability, or tort       *
*  (including negligence or otherwise) arising in any way out of the use of  *
*  this software, even if advised of the possibility of such damage.         *
*                                                                            *
******************************************************************************/
	/*use by c function,digest[16] must convert ,you can use the code:
	std::string finallHash = "";
		for ( int i = 0 ; i < 16 ; i++ )
		{
			char tmp[3];
			_itoa(digest[i], tmp , 16);

			if (strlen(tmp) == 1)
			{
				tmp[1] = tmp[0];
				tmp[0] = '0';
				tmp[2] = '\0';
			}
			finallHash += tmp;
		}
	return finallHash; */

#ifndef _LGY_MD5_H
#define _LGY_MD5_H

#include "Common.h"
#include <string>

namespace util {

/* MD5 Class. */
class SHARED_DLL_DECL MD5_CTX {
public:
	MD5_CTX();
	virtual ~MD5_CTX();
	//use by c function
	static void MD5Update ( unsigned char *input, unsigned int inputLen);
	static void MD5Final (unsigned char digest[16]);										
private:
	static unsigned long int state[4];     /* state (ABCD) */
	static unsigned long int count[2];     /* number of bits, 
										   modulo 2^64 (lsb first) */
	static unsigned char buffer[64];       /* input buffer */
	static unsigned char PADDING[64];  /* What? */
private:
	static void MD5Init ();
	static void MD5Transform (unsigned long int state[4], unsigned char block[64]);
	static void MD5_memcpy (unsigned char* output, unsigned char* input,unsigned int len);
	static void Encode (unsigned char *output, unsigned long int *input,unsigned int len);
	static void Decode (unsigned long int *output, unsigned char *input, unsigned int len);
	static void MD5_memset (unsigned char* output,int value,unsigned int len);
	static void MD5_Clear();
public:
	//direct use following function
	static std::string MakePassMD5(std::string m_Input);
	static void MakePassMD5(unsigned char* OutputData, unsigned char* InputData, unsigned int InputDataSize);
	static void MakePassMD5(unsigned char* OutputData, const char* szFileName);

};

}

#endif
