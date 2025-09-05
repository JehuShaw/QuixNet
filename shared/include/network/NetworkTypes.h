/*
 * File:   NetworkTypes.h
 * Author: Jehu Shaw
 *
 * Created on 2010_8_24 22:51
 */

#ifndef NETWORKTYPES_H
#define	NETWORKTYPES_H

#include "Common.h"
#include "SmallBuffer.h"

namespace ntwk
{

// leave 4 byte for len, so 4294967296 - 4 = 4294967292
#define MAX_PACKET_SIZE 4294967292
#define SOCKETID_INDEX_NULL -1
#define INVALID_FD -1

    /// Corresponds to a network address
    struct SHARED_DLL_DECL SocketID
    {
		int fd;
        /// index SocketID instance in array
        int index;
        ///The peer address from inet_addr.
        unsigned int binaryAddress;
        ///The port number
        unsigned short port;

        SocketID() : fd(INVALID_FD), index(SOCKETID_INDEX_NULL), binaryAddress(0), port(0){}

        // Return the RemoteClient as a string in the format <IP>:<Port>
        // Note - returns a static string.  Not thread-safe or safe for multiple calls per line.
        char *ToString(bool writePort=true) const;

        // Sets the binary address part from a string.  Doesn't set the port
        void SetBinaryAddress(const char *str);

		bool IsEmpty() const {
			return SOCKETID_INDEX_NULL == index && INVALID_FD == fd;
		}

        //bool operator==( const SocketID& right ) const;
        //bool operator!=( const SocketID& right ) const;
        //bool operator > ( const SocketID& right ) const;
        //bool operator < ( const SocketID& right ) const;

        bool operator==( const SocketID& right ) const
        {
            return index == right.index && fd == right.fd;
        }

        bool operator!=( const SocketID& right ) const
        {
            return index != right.index || fd != right.fd;
        }

        bool operator > ( const SocketID& right ) const
        {
            if(index > right.index) {
				return true;
			}else if(index == right.index) {
				if(fd > right.fd) {
					return true;
				}
			}
			return false;
        }

        bool operator < ( const SocketID& right ) const
        {
			if(index < right.index) {
				return true;
			}else if(index == right.index) {
				if(fd < right.fd) {
					return true;
				}
			}
			return false;
        }

    };

    /// This represents a user message from another system.
    struct Packet
    {
        /// The length of the data in bytes
        unsigned int length;

        /// The data from the sender
        SmallBuffer data;
    };

}
#endif	/* NETWORKTYPES_H */

