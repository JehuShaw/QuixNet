
#if defined(_WIN32) || defined(_WIN64) 
#include "ProcessStatus.h"
#include "ExceptionManager.h"
#include <tchar.h>
#include <stdio.h>
#include <DbgHelp.h>


typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)

SHARED_DLL_DECL LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = NULL;
SHARED_DLL_DECL HANDLE hProcess = NULL;

SHARED_DLL_DECL char ExceptionBuf[MAX_OUTPUT_MESSAGE];
SHARED_DLL_DECL int  Line_B           = 0;
SHARED_DLL_DECL char File_B[128]      = {0};
SHARED_DLL_DECL int  Line_Net         = 0;
SHARED_DLL_DECL int  Line_E           = 0;
SHARED_DLL_DECL char File_E[128]      = {0};

using namespace util;

const char* GetExceptionString( DWORD dwCode )
{
#define EXCEPTION( x ) case EXCEPTION_##x: return (#x);

	switch ( dwCode )
	{
		EXCEPTION( ACCESS_VIOLATION )
		EXCEPTION( DATATYPE_MISALIGNMENT )
		EXCEPTION( BREAKPOINT )
		EXCEPTION( SINGLE_STEP )
		EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
		EXCEPTION( FLT_DENORMAL_OPERAND )
		EXCEPTION( FLT_DIVIDE_BY_ZERO )
		EXCEPTION( FLT_INEXACT_RESULT )
		EXCEPTION( FLT_INVALID_OPERATION )
		EXCEPTION( FLT_OVERFLOW )
		EXCEPTION( FLT_STACK_CHECK )
		EXCEPTION( FLT_UNDERFLOW )
		EXCEPTION( INT_DIVIDE_BY_ZERO )
		EXCEPTION( INT_OVERFLOW )
		EXCEPTION( PRIV_INSTRUCTION )
		EXCEPTION( IN_PAGE_ERROR )
		EXCEPTION( ILLEGAL_INSTRUCTION )
		EXCEPTION( NONCONTINUABLE_EXCEPTION )
		EXCEPTION( STACK_OVERFLOW )
		EXCEPTION( INVALID_DISPOSITION )
		EXCEPTION( GUARD_PAGE )
		EXCEPTION( INVALID_HANDLE )
	}

	// If not one of the "known" exceptions, try to get the string
	// from NTDLL.DLL's message table.

	static char szBuffer[512] = { 0 };

	FormatMessageA( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
		GetModuleHandle( _T("NTDLL.DLL") ),
		dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

	return szBuffer;
}

BOOL GetLogicalAddress( PVOID addr, char* szModule, DWORD len, DWORD& section, DWORD& offset )
{

	MEMORY_BASIC_INFORMATION mbi;

	if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if ( !GetModuleFileNameA( (HMODULE)hMod, szModule, len ) )
		return FALSE;

	// Point to the DOS header in memory
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

	// From the DOS header, find the NT (PE) header
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

	DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

	// Iterate through the section table, looking for the one that encompasses
	// the linear address.
	for (   unsigned i = 0;
		i < pNtHdr->FileHeader.NumberOfSections;
		i++, pSection++ )
	{
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd = sectionStart
			+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		// Is the address in this section???
		if ( (rva >= sectionStart) && (rva <= sectionEnd) )
		{
			// Yes, address is in the section.  Calculate section and offset,
			// and store in the "section" & "offset" params, which were
			// passed by reference.
			section = i+1;
			offset = rva - sectionStart;
			return TRUE;
		}
	}

	return FALSE;   // Should never get here!

}


void WriteStackDetails(	PCONTEXT pContext, bool bWriteVariables )  // true if local/params should be output
{
	sprintf_s( ExceptionBuf, "%s\r\nCall stack:\r\n", ExceptionBuf );
	sprintf_s( ExceptionBuf, "%sAddress   Frame     Function\r\n", ExceptionBuf );

	DWORD dwMachineType = 0;
	// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

	STACKFRAME sf;
	memset( &sf, 0, sizeof(sf) );

#ifdef _M_IX86
	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	sf.AddrPC.Offset       = pContext->Eip;
	sf.AddrPC.Mode         = AddrModeFlat;
	sf.AddrStack.Offset    = pContext->Esp;
	sf.AddrStack.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset    = pContext->Ebp;
	sf.AddrFrame.Mode      = AddrModeFlat;

	dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif

	int i = 3;
	while ( 1 )
	{
		i--;
		// Get the next stack frame
		if ( ! StackWalk(  dwMachineType, hProcess,	GetCurrentThread(),	&sf, pContext, 0, SymFunctionTableAccess, SymGetModuleBase, 0 ) )
			break;

		if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
			break;                      // the frame is OK.  Bail if not.

#if defined(_WIN64)
		sprintf_s(ExceptionBuf, "%s%08I64X  %08I64X  ", ExceptionBuf, sf.AddrPC.Offset, sf.AddrFrame.Offset);
#elif  defined(_WIN32)
		sprintf_s( ExceptionBuf, "%s%08X  %08X  ", ExceptionBuf, sf.AddrPC.Offset, sf.AddrFrame.Offset );
#endif
		/*
		// Get the name of the function for this stack frame entry
		BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + 1024 ];
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLen = 1024;

		DWORD64 symDisplacement = 0;    // Displacement of the input address,
		// relative to the start of the symbol

		if ( SymFromAddr(hProcess,sf.AddrPC.Offset,&symDisplacement,pSymbol))
		{
		sprintf_s( ExceptionBuf, "%s%hs+" XINT64FMT, ExceptionBuf, pSymbol->Name, symDisplacement );
		}
		else    // No symbol found.  Print out the logical address instead.
		{
		char szModule[MAX_OUTPUT_MESSAGE];
		memset( szModule, 0, MAX_OUTPUT_MESSAGE );
		DWORD section = 0, offset = 0;

		GetLogicalAddress(  (PVOID)sf.AddrPC.Offset, szModule, sizeof(szModule), section, offset );

		sprintf_s( ExceptionBuf, "%s%04X:%08X", ExceptionBuf, section, offset );
		}

		*/
		// Get the source line for this stack frame entry
		IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
		DWORD dwLineDisplacement;
		if ( SymGetLineFromAddr( hProcess, sf.AddrPC.Offset, &dwLineDisplacement, &lineInfo ) )
		{
			sprintf_s( ExceptionBuf, "%s  %s line %u", ExceptionBuf,lineInfo.FileName,lineInfo.LineNumber); 
		}


		sprintf_s( ExceptionBuf, "%s\r\n", ExceptionBuf );

		/*
		// Write out the variables, if desired
		if ( bWriteVariables )
		{
		// Use SymSetContext to get just the locals/params for this frame
		IMAGEHLP_STACK_FRAME imagehlpStackFrame;
		imagehlpStackFrame.InstructionOffset = sf.AddrPC.Offset;
		SymSetContext( m_hProcess, &imagehlpStackFrame, 0 );

		// Enumerate the locals/parameters
		SymEnumSymbols( m_hProcess, 0, 0, EnumerateSymbolsCallback, &sf );

		_tprintf( _T("\r\n") );
		}
		*/
	}

}

LONG WINAPI UnknowExceptionHandler( _EXCEPTION_POINTERS* pExceptionInfo ) throw () 
{
	//CreateMiniDump(pExceptionInfo);
	memset( ExceptionBuf, 0, MAX_OUTPUT_MESSAGE );

	SYSTEMTIME time;
	GetLocalTime( &time );
	sprintf_s( ExceptionBuf, "\nException Time: %d:%d:%d (%d-%d-%d)\n\n", time.wHour+8, time.wMinute, time.wSecond, time.wYear, time.wMonth, time.wDay );

	// 错误码及系统环境
	sprintf_s(ExceptionBuf, "%sError code: %d\r\n", ExceptionBuf, ::GetLastError());

	int cpu = 0;
	uint64_t mem = 0, vmem = 0, r = 0, w = 0;

	cpu = CpuUsage();
	mem = MemoryUsage();
	vmem = VirtualMemoryUsage();
	r = IoReadBytes();
	w = IoWriteBytes();

	mem /= 1024;	// kb
	vmem /= 1024;	// kb
	r /= 1024;		// kb
	w /= 1024;		// kb
	cpu = cpu <= 0? 0 : cpu;
	sprintf_s(ExceptionBuf, "%smem:%I64d vmem:%I64d read:%I64d write:%I64d cpu:%d\r\n", 
		ExceptionBuf, mem, vmem, r, w, cpu);
	//////////////////////////

	DWORD nThreadId  = ::GetCurrentThreadId();
	DWORD nProcessId = ::GetCurrentProcessId();
	hProcess = GetCurrentProcess();
	sprintf_s( ExceptionBuf, "%sProcess ID=%d, Thread ID=%d\n", ExceptionBuf, nProcessId, nThreadId );

	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;
	sprintf_s( ExceptionBuf, "%sException code: %08X %s\r\n", ExceptionBuf, pExceptionRecord->ExceptionCode, GetExceptionString(pExceptionRecord->ExceptionCode) );

	char szFaultingModule[MAX_OUTPUT_MESSAGE];
	DWORD section, offset;
	GetLogicalAddress(  pExceptionRecord->ExceptionAddress, szFaultingModule, sizeof( szFaultingModule ), section, offset );
#if defined(_WIN64)
	sprintf_s(ExceptionBuf, "%sFault address:  %08I64X %02X:%08X\r\nFile: %s\r\n", ExceptionBuf, (uintptr_t)pExceptionRecord->ExceptionAddress, section, offset, szFaultingModule);
#elif  defined(_WIN32)
	sprintf_s( ExceptionBuf, "%sFault address:  %08X %02X:%08X\r\nFile: %s\r\n", ExceptionBuf, (uintptr_t)pExceptionRecord->ExceptionAddress, section, offset, szFaultingModule );
#endif

	PCONTEXT pCtx = pExceptionInfo->ContextRecord;
	
	printf_s( ExceptionBuf, "%s\r\nRegisters:\r\n", ExceptionBuf );
#if defined(_WIN64)
	sprintf_s(ExceptionBuf, "%sRAX:%08I64X\tRBX:%08I64X\r\nRCX:%08I64X\tRDX:%08I64X\r\nRSI:%08I64X\tRDI:%08I64X\r\n", ExceptionBuf, pCtx->Rax, pCtx->Rbx, pCtx->Rcx, pCtx->Rdx, pCtx->Rsi, pCtx->Rdi);
	sprintf_s(ExceptionBuf, "%sCS:RIP:%04X:%08I64X\r\n", ExceptionBuf, pCtx->SegCs, pCtx->Rip);
	sprintf_s(ExceptionBuf, "%sSS:RSP:%04X:%08I64X  RBP:%08I64X\r\n", ExceptionBuf, pCtx->SegSs, pCtx->Rsp, pCtx->Rbp);
#elif defined(_WIN32)
	sprintf_s( ExceptionBuf, "%sEAX:%08X\tEBX:%08X\r\nECX:%08X\tEDX:%08X\r\nESI:%08X\tEDI:%08X\r\n", ExceptionBuf, pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx,pCtx->Esi, pCtx->Edi );
	sprintf_s( ExceptionBuf, "%sCS:EIP:%04X:%08X\r\n", ExceptionBuf,pCtx->SegCs, pCtx->Eip );
	sprintf_s( ExceptionBuf, "%sSS:ESP:%04X:%08X  EBP:%08X\r\n", ExceptionBuf,pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
#endif
	sprintf_s( ExceptionBuf, "%sDS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", ExceptionBuf,pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
	sprintf_s( ExceptionBuf, "%sFlags:%08X\r\n", ExceptionBuf,pCtx->EFlags );
	sprintf_s( ExceptionBuf, "%sBeginFile:%s,BeginLine:%d \r\n",ExceptionBuf,File_B,Line_B);
	sprintf_s( ExceptionBuf, "%sEndFile:%s,EndLine:%d \r\n",ExceptionBuf,File_E,Line_E);
	sprintf_s( ExceptionBuf, "%sLine:%d \r\n",ExceptionBuf,Line_Net);

	SymSetOptions( SYMOPT_DEFERRED_LOADS );
	// Initialize DbgHelp
	//if ( !SymInitialize( GetCurrentProcess(), 0, TRUE ) )
	//{
	//	FILE* fp = fopen("../../Log/Error.log" , "at+");
	//	if (!fp)
	//		fp = fopen("../../Log/Error.log" , "wb+");
	//	fwrite("\n________________________________________\n\n" , 1 , strlen("\n________________________________________\n\n") , fp);
	//	fwrite( ExceptionBuf, sizeof(char), strlen(ExceptionBuf), fp );
	//	fclose( fp );
	//	//::MessageBoxA( NULL, ExceptionBuf, "程序遇到异常必须终止", MB_OK );

	//	if ( g_previousFilter ) 
	//	 	return g_previousFilter( pExceptionInfo ); 
	//	else 
	//	 	return EXCEPTION_CONTINUE_SEARCH; 
	//}


	CONTEXT trashableContext = *pCtx;
	WriteStackDetails( &trashableContext, false );

	/*
	#ifdef _M_IX86  // X86 Only!


	sprintf_s( buf, "%s========================\r\n", buf );
	sprintf_s( buf, "%sLocal Variables And Parameters\r\n", buf );

	trashableContext = *pCtx;
	WriteStackDetails( &trashableContext, true );

	sprintf_s( buf, "%s========================\r\n", buf );
	sprintf_s( buf, "%sGlobal Variables\r\n", buf );

	SymEnumSymbols( GetCurrentProcess(), (DWORD64)GetModuleHandle(szFaultingModule), 0, EnumerateSymbolsCallback, 0 );

	#endif      // X86 Only!

	SymCleanup( GetCurrentProcess() );
	*/


	FILE* fp = fopen("log/Error.log" , "at+");
	if (!fp)
		fp = fopen("log/Error.log" , "wb+");
	fwrite("\n________________________________________\n\n" , 1 , strlen("\n________________________________________\n\n") , fp);
	fwrite( ExceptionBuf, sizeof(char), strlen(ExceptionBuf), fp );
	fclose( fp );

	//::MessageBoxA( NULL, ExceptionBuf, "程序遇到异常必须终止", MB_OK );
	//ExitProcess(0);

// 	if ( g_previousFilter ) 
// 		return g_previousFilter( pExceptionInfo ); 
// 	else 
		return EXCEPTION_EXECUTE_HANDLER; 
}
#pragma warning(pop)

#endif
