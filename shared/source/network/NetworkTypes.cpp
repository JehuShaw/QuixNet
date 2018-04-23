
#include "NetworkTypes.h"
#include <string.h>
#include <stdio.h>


#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char* _itoa( int value, char* result, int base ) {
    // check that the base if valid
    if (base < 2 || base > 16) { *result = 0; return result; }
    char* out = result;
    int quotient = value;

    int absQModB;

    do {
        // KevinJ - get rid of this dependency
        //*out = "0123456789abcdef"[ std::abs( quotient % base ) ];
        absQModB=quotient % base;
        if (absQModB < 0)
            absQModB=-absQModB;
        *out = "0123456789abcdef"[ absQModB ];
        ++out;
        quotient /= base;
    } while ( quotient );

    // Only apply negative sign for base 10
    if ( value < 0 && base == 10) *out++ = '-';

    // KevinJ - get rid of this dependency
    // std::reverse( result, out );
    *out = 0;

    // KevinJ - My own reverse code
    char *start = result;
    char temp;
    out--;
    while (start < out)
    {
        temp=*start;
        *start=*out;
        *out=temp;
        start++;
        out--;
    }
    return result;
}
#endif

namespace ntwk {

//bool SocketID::operator==( const SocketID& right ) const
//{
//    return binaryAddress == right.binaryAddress && port == right.port;
//}

//bool SocketID::operator!=( const SocketID& right ) const
//{
//    return binaryAddress != right.binaryAddress || port != right.port;
//}
//
//bool SocketID::operator>( const SocketID& right ) const
//{
//    return ( ( binaryAddress > right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port > right.port ) ) );
//}
//
//bool SocketID::operator<( const SocketID& right ) const
//{
//    return ( ( binaryAddress < right.binaryAddress ) || ( ( binaryAddress == right.binaryAddress ) && ( port < right.port ) ) );
//}
char *SocketID::ToString(bool writePort) const
{
    static char str[22];
    in_addr in;
    in.s_addr = binaryAddress;
    strcpy(str, inet_ntoa( in ));
    if (writePort)
    {
        strcat(str, ":");
        _itoa(port, str+strlen(str), 10);

    }
    return (char*) str;
}
void SocketID::SetBinaryAddress(const char *str)
{
    binaryAddress=inet_addr(str);
}

} // end namespace ntwk