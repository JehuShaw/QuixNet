/*
 * File:   PoolBase.h
 * Author: Jehu Shaw
 *
 */
#ifndef PROCESS_STAT_H
#define PROCESS_STAT_H

#include "Common.h"
#include <string>

namespace util {

#ifdef __cplusplus
extern "C" {
#endif

	/// 获取当前进程的cpu usage
	SHARED_DLL_DECL int CpuUsage();
	/// 获取当前进程内存使用量
	SHARED_DLL_DECL int64_t MemoryUsage();
	/// 获取当前进程虚拟内存使用量
	SHARED_DLL_DECL int64_t VirtualMemoryUsage();
	/// 获取当前进程总共读IO的字节数
	SHARED_DLL_DECL int64_t IoReadBytes();
	/// 获取当前进程总共写IO的字节数
	SHARED_DLL_DECL int64_t IoWriteBytes();

#ifdef  __cplusplus
}
#endif

	/// 获取当前进程的cpu usage
	SHARED_DLL_DECL std::string GetCPUUsageStr();
	/// 获取当前进程内存使用量
	SHARED_DLL_DECL std::string GetMemoryUsageStr();
	/// 获取当前进程虚拟内存使用量
	SHARED_DLL_DECL std::string GetVirtualMemoryUsageStr();
	/// 获取当前进程总共读IO的字节数
	SHARED_DLL_DECL std::string GetIOReadKBStr();
	/// 获取当前进程总共写IO的字节数
	SHARED_DLL_DECL std::string GetIOWriteKBStr();

}

#endif/*PROCESS_STAT_H*/
