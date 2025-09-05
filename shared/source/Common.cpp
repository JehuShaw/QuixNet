#include "Common.h"


uint64_t AddressToInteger(const char * str)
{
	const char* address = strstr(str, "://");
	if(NULL == address) {
		address = str;
	} else {
		address += 3;
	}

	uint64_t res = 0;
	char* p = NULL;
	int nCount = 0;
	while(nCount < 5) {
		p = (char*)strchr(address, '.');
		if(NULL == p) {
			break;
		}

		res <<= 8;
		res |= (uint8_t)strtoul(address, NULL, 10);
		address = p + 1;
		++nCount;
	}
	if(nCount > 2) {
		p = (char*)strchr(address, ':');
		if(NULL != p) {
			res <<= 8;
			res |= (uint8_t)strtoul(address, NULL, 10);
			address = p + 1;

			res <<= 16;
			res |= (uint16_t)strtoul(address, NULL, 10);
		} else {
			res = 0;
		}
	} else {
		res = 0;
	}
	return res;
}

bool ReviseAddress(std::string& outStr)
{
	const char* pSrc = outStr.c_str();
	char protocal[128] = { '\0' };
	const char* address = strstr(pSrc, "://");
	if(NULL == address) {
		address = pSrc;
	} else {
		address += 3;
		memcpy(protocal, pSrc, address - pSrc);
		protocal[address - pSrc] = '\0';
	}

	char hostname[256] = { '\0' };
	uint16_t port = 0;
	char* p(NULL);
	if ((p = (char*)strchr(address, ':')) == NULL
		|| unsigned(p - address) >= sizeof(hostname)
		|| (port = (uint16_t)strtoul(p+1, NULL, 10)) == 0)
	{
		return false;
	}
	memcpy(hostname, address, p - address);
	hostname[p - address] = '\0';

	struct hostent* hp;
#if defined(HAVE_GETHOSTBYNAME_R) && !defined(NO_PTHREADS)
	struct hostent ent;  // entry in hosts table
	char buf[GETHOSTBYNAME_BUF_SIZE];
	int h_err;
#if defined(__sun)
	if ((hp = gethostbyname_r(hostname, &ent, buf, sizeof buf, &h_err)) == NULL
#else
	if (gethostbyname_r(hostname, &ent, buf, sizeof buf, &hp, &h_err) != 0
		|| hp == NULL
#endif
		)
#else
	if ((hp = gethostbyname(hostname)) == NULL)
#endif
	{
		return false;
	}

	if(hp->h_length > 0) {
		char szNumber[64] = { '\0' };
		outStr = protocal;
		ultostr(szNumber, (unsigned char)hp->h_addr[0], 10, 0);
		outStr += szNumber;
		for(int i = 1; i < hp->h_length; ++i) {
			outStr += '.';
			ultostr(szNumber, (unsigned char)hp->h_addr[i], 10, 0);
			outStr += szNumber;
		}
		outStr += ':';
		ultostr(szNumber, port, 10, 0);
		outStr += szNumber;
		return true;
	}
	return false;
}
