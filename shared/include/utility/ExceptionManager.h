/*
 * File:   ExceptionManager.h
 * Author: Jehu Shaw
 *
 * Created on 2010_1_25 14:39
 */
#ifndef EXCEPTIONMANAGER_H
#define	EXCEPTIONMANAGER_H

#if defined(_WIN32) || defined(_WIN64) 

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

SHARED_DLL_DECL const char* GetExceptionString(DWORD dwCode);
SHARED_DLL_DECL BOOL GetLogicalAddress(PVOID addr, char* szModule, DWORD len, DWORD& section, DWORD& offset);
SHARED_DLL_DECL void WriteStackDetails(PCONTEXT pContext, bool bWriteVariables);
SHARED_DLL_DECL LONG WINAPI UnknowExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo) throw ();
//void __cdecl CrashTerminator();
//void __cdecl CrashInvalidParameter(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved );

#endif

#endif // EXCEPTIONMANAGER_H