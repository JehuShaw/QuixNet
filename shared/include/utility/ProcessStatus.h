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

	/// ��ȡ��ǰ���̵�cpu usage
	SHARED_DLL_DECL int CpuUsage();
	/// ��ȡ��ǰ�����ڴ�ʹ����
	SHARED_DLL_DECL int64_t MemoryUsage();
	/// ��ȡ��ǰ���������ڴ�ʹ����
	SHARED_DLL_DECL int64_t VirtualMemoryUsage();
	/// ��ȡ��ǰ�����ܹ���IO���ֽ���
	SHARED_DLL_DECL int64_t IoReadBytes();
	/// ��ȡ��ǰ�����ܹ�дIO���ֽ���
	SHARED_DLL_DECL int64_t IoWriteBytes();

#ifdef  __cplusplus
}
#endif

	/// ��ȡ��ǰ���̵�cpu usage
	SHARED_DLL_DECL std::string GetCPUUsageStr();
	/// ��ȡ��ǰ�����ڴ�ʹ����
	SHARED_DLL_DECL std::string GetMemoryUsageStr();
	/// ��ȡ��ǰ���������ڴ�ʹ����
	SHARED_DLL_DECL std::string GetVirtualMemoryUsageStr();
	/// ��ȡ��ǰ�����ܹ���IO���ֽ���
	SHARED_DLL_DECL std::string GetIOReadKBStr();
	/// ��ȡ��ǰ�����ܹ�дIO���ֽ���
	SHARED_DLL_DECL std::string GetIOWriteKBStr();

}

#endif/*PROCESS_STAT_H*/
