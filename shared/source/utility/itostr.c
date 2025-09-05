#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "ShareDll.h"
//////////////////////////////////////////////////////////////////////////
static char *__ultostr(char *buf, unsigned long uval, int base, int uppercase)
{
	int digit;

	if ((base < 2) || (base > 36)) {
		return 0;
	}

	*buf = '\0';

	do {
		digit = uval % base;
		uval /= base;

		/* note: slightly slower but generates less code */
		*--buf = '0' + digit;
		if (digit > 9) {
			*buf = (uppercase ? 'A' : 'a') + digit - 10;
		}
	} while (uval);

	return buf;
}

static char *__ltostr(char *buf, long val, int base, int uppercase)
{
	unsigned long uval;
	char *pos;
    int negative;

	negative = 0;
    if (val < 0) {
		negative = 1;
		uval = ((unsigned long)(-(1+val))) + 1;
    } else {
		uval = val;
	}


    pos = __ultostr(buf, uval, base, uppercase);

    if (pos && negative) {
		*--pos = '-';
    }

    return pos;
}

static char *__ulltostr(char *buf, unsigned long long uval, int base, int uppercase)
{
	int digit;

	if ((base < 2) || (base > 36)) {
		return 0;
	}

	*buf = '\0';

	do {
		digit = uval % base;
		uval /= base;

		/* note: slightly slower but generates less code */
		*--buf = '0' + digit;
		if (digit > 9) {
			*buf = (uppercase ? 'A' : 'a') + digit - 10;
		}
	} while (uval);

	return buf;
}

static char *__lltostr(char *buf, long long val, int base, int uppercase)
{
	unsigned long long uval;
	char *pos;
	int negative;

	negative = 0;
	if (val < 0) {
		negative = 1;
		uval = ((unsigned long long)(-(1+val))) + 1;
	} else {
		uval = val;
	}


	pos = __ulltostr(buf, uval, base, uppercase);

	if (pos && negative) {
		*--pos = '-';
	}

	return pos;
}
//////////////////////////////////////////////////////////////////////////
SHARED_DLL_DECL int ltostr(char* dst, long val, int base, int uppercase) {
	char buf[32] = {'\0'};
	char* pEnd = buf + sizeof(buf) - 1;
	char* pStart = __ltostr(pEnd, val, base, uppercase);
	int len = (int)((intptr_t)pEnd - (intptr_t)pStart);
	memcpy(dst, pStart, len + 1);
#if _DEBUG
	assert(len >= 0);
#endif 
	return len;
}

SHARED_DLL_DECL int ultostr(char* dst, unsigned long uval, int base, int uppercase) {
	char buf[32] = {'\0'};
	char* pEnd = buf + sizeof(buf) - 1;
	char* pStart = __ultostr(pEnd, uval, base, uppercase);
	int len = (int)((intptr_t)pEnd - (intptr_t)pStart);
	memcpy(dst, pStart, len + 1);
#if _DEBUG
	assert(len >= 0);
#endif 
	return len;
}

SHARED_DLL_DECL int lltostr(char* dst, long long val, int base, int uppercase) {
	char buf[64] = {'\0'};
	char* pEnd = buf + sizeof(buf) - 1;
	char* pStart = __lltostr(pEnd, val, base, uppercase);
	int len = (int)((intptr_t)pEnd - (intptr_t)pStart);
	memcpy(dst, pStart, len + 1);
#if _DEBUG
	assert(len >= 0);
#endif 
	return len;
}

SHARED_DLL_DECL int ulltostr(char* dst, unsigned long long uval, int base, int uppercase) {
	char buf[64] = {'\0'};
	char* pEnd = buf + sizeof(buf) - 1;
	char* pStart = __ulltostr(pEnd, uval, base, uppercase);
	int len = (int)((intptr_t)pEnd - (intptr_t)pStart);
	memcpy(dst, pStart, len + 1);
#if _DEBUG
	assert(len >= 0);
#endif 
	return len;
}