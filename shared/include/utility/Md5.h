#ifndef MD5_H
#define MD5_H

#include <string>
#include <cstring>
#include "Common.h"

namespace util {

	SHARED_DLL_DECL std::string md5(std::string dat);
	SHARED_DLL_DECL std::string md5(const void* dat, size_t len);
	SHARED_DLL_DECL std::string md5file(const char* filename);
	SHARED_DLL_DECL std::string md5file(std::FILE* file);
	SHARED_DLL_DECL std::string md5sum6(std::string dat);
	SHARED_DLL_DECL std::string md5sum6(const void* dat, size_t len);
}

#endif // end of MD5_H
