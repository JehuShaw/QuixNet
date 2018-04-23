/********************************************************************
            Copyright (c) 2010, 欢乐连线工作室
                   All rights reserved
         
    创建日期：  2010年1月25日 14时39分
    文件名称：  ExceptionManager.h
    说    明：  异常管理
    
    当前版本：  1.00
    作    者：  Conserlin
    概    述：  创建  

*********************************************************************/
#pragma once

#ifdef WIN32

#include "Common.h"
#include <Windows.h>

#define MAX_OUTPUT_MESSAGE		10000

extern SHARED_DLL_DECL LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter;
extern SHARED_DLL_DECL HANDLE hProcess;
extern SHARED_DLL_DECL char ExceptionBuf[MAX_OUTPUT_MESSAGE];
extern SHARED_DLL_DECL int  Line_B;
extern SHARED_DLL_DECL char File_B[128];
extern SHARED_DLL_DECL int  Line_Net;
extern SHARED_DLL_DECL int  Line_E;
extern SHARED_DLL_DECL char File_E[128];

SHARED_DLL_DECL char* GetExceptionString(DWORD dwCode);
SHARED_DLL_DECL BOOL GetLogicalAddress(PVOID addr, char* szModule, DWORD len, DWORD& section, DWORD& offset);
SHARED_DLL_DECL void WriteStackDetails(PCONTEXT pContext, bool bWriteVariables);
SHARED_DLL_DECL LONG WINAPI UnknowExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo) throw ();
//void __cdecl CrashTerminator();
//void __cdecl CrashInvalidParameter(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved );

#endif
