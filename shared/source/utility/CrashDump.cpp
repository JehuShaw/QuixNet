#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined(_WIN64)

#include "CrashDump.h"
#include <signal.h>
#include <stdio.h>
#include <tchar.h>
#include <eh.h>


#pragma comment(lib, "dbghelp.lib")
#pragma message( "Automatically linking with dbghelp.lib......" )

namespace Frame
{
	SHARED_DLL_DECL CrashDump *CrashDump::s_pSelf = NULL;
	LPTOP_LEVEL_EXCEPTION_FILTER CrashDump::s_previousFilter = NULL;
	LPTOP_LEVEL_EXCEPTION_FILTER CrashDump::s_CBFilter = NULL;

	_invalid_parameter_handler CrashDump::s_previous_iph = NULL;
	_purecall_handler CrashDump::s_previous_pch	= NULL;

	MINIDUMP_TYPE CrashDump::s_minidumptype = MiniDumpNormal;


	//inline void signal_handler(int)
	//{
	//	terminator();
	//}
	//

	CrashDump * CrashDump::Begin(LPTOP_LEVEL_EXCEPTION_FILTER CBFilter/* = NULL*/, MINIDUMP_TYPE minidumptype/* = MiniDumpNormal*/) {
		if (NULL == s_pSelf) {
			s_pSelf = new CrashDump(CBFilter, minidumptype);
		}
		return s_pSelf;
	}

	void CrashDump::End() {
		delete s_pSelf;
		s_pSelf = NULL;
	}

	CrashDump::CrashDump(LPTOP_LEVEL_EXCEPTION_FILTER CBFilter/* = NULL*/, MINIDUMP_TYPE minidumptype/* = MiniDumpNormal*/) {
		//SetErrorMode(SEM_NOGPFAULTERRORBOX)的意思是即使程序出现未处理异常，也不弹出错误信息，而是安静的离开。 

		//	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);就可以满足你的要求，另外，下面的仅供参考 

		//	_set_new_mode(1);//设置new底层通过malloc分配内存 
		//_set_se_translator(SeTranslator);//设置结构化异常到标准C＋＋异常的转换 
		//_set_error_mode(_OUT_TO_STDERR);//设置C++的错误信息输出方式 
		//SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);//设置错误处理模式 
		//SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)UnknowExceptionHandler);//设置未处理的结构化异常处理例程 
		//set_terminate(   term_func   );//设置C++终止程序处理例程 
		//set_unexpected(Unexpected);//设置未处理C++异常处理例程 
		//_set_purecall_handler(PurecallHandler);//设置纯虚函数调用处理例程 
		//_set_new_handler(handle_program_memory_depletion);//设置new失败的处理例程，只有当_set_new_mode(1)时才有效。

		if (NULL == s_previousFilter) {		
			//// 使其不再出现错误运算窗口
			//SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX) ; 
			//signal(SIGABRT, signal_handler);
			//_set_abort_behavior(0, _WRITE_ABORT_MSG|_CALL_REPORTFAULT);

			//set_terminate( &terminator );
			//set_unexpected( &terminator );
			s_CBFilter = CBFilter;
			s_minidumptype = minidumptype;

			s_previousFilter = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&CrashDump::UnhandledExceptionFilter);
#if _MSC_VER >= 1400  // MSVC 2005/8
			s_previous_iph = _set_invalid_parameter_handler(&CrashDump::HandleInvalidParameter);
#endif  // _MSC_VER >= 1400
			s_previous_pch = _set_purecall_handler(&CrashDump::HandlePureVirtualCall);

			// 设置C终止程序处理例程
			::set_terminate(&CrashDump::terminator); 
			// 设置未处理C异常处理例程 
			::set_unexpected(&CrashDump::terminator);

			// DisableSetUnhandledExceptionFilter
			void *addr = (void*)GetProcAddress(LoadLibrary(_T("kernel32.dll")),
				"SetUnhandledExceptionFilter");
			if (addr) 
			{
				unsigned char code[16];
				int size = 0;
				code[size++] = 0x33;
				code[size++] = 0xC0;
				code[size++] = 0xC2;
				code[size++] = 0x04;
				code[size++] = 0x00;

				DWORD dwOldFlag, dwTempFlag;
				VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
				WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
				VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
			}
		}

		if (!SymInitialize( GetCurrentProcess(), 0, TRUE )) {
			printf("crash dump SymInitialize err! \n");
		}
	}

	CrashDump::~CrashDump() {
		SymCleanup(GetCurrentProcess());
		
		SetUnhandledExceptionFilter(s_previousFilter) ;
		s_previousFilter = NULL;
		s_CBFilter = NULL;


#if _MSC_VER >= 1400  // MSVC 2005/8
		_set_invalid_parameter_handler(s_previous_iph);
		s_previous_iph = NULL;
#endif  // _MSC_VER >= 1400
		_set_purecall_handler(s_previous_pch);
		s_previous_pch = NULL;
	}



	LONG WINAPI CrashDump::UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *lpTopLevelExceptionFilter) {
		CreateMiniDump(lpTopLevelExceptionFilter); 
		//EXCEPTION_EXECUTE_HANDLER = 1 
		//	EXCEPTION_CONTINUE_EXECUTION = -1 
		//	这两个返回值都应该由调用UnhandledExceptionFilter后返回。 
		//	EXCEPTION_EXECUTE_HANDLER表示进程结束 
		//	EXCEPTION_CONTINUE_EXECUTION表示处理异常之后继续执行 
		//	EXCEPTION_CONTINUE_SEARCH = 0 
		//	进行系统通常的异常处理（错误消息对话框） 

		if (NULL != s_CBFilter) {
			return s_CBFilter(lpTopLevelExceptionFilter);
		}

		WalkStack(lpTopLevelExceptionFilter);
		return EXCEPTION_CONTINUE_SEARCH; 
	}

	INT CrashDump::CreateMiniDump(LPEXCEPTION_POINTERS ExceptionInfo) {

		LONG ret = EXCEPTION_EXECUTE_HANDLER; 
		if (ExceptionInfo == NULL) {
			// Generate exception to get proper context in dump
			__try {
				OutputDebugString(_T("raising exception\r\n"));
				RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
			} __except(CreateMiniDump(GetExceptionInformation()),
				EXCEPTION_CONTINUE_EXECUTION) 
			{
			}
			return EXCEPTION_EXECUTE_HANDLER;
		} 

		//BOOL bMiniDumpSuccessful; 
		//TCHAR szPath[MAX_PATH]; 
		TCHAR szFileName[MAX_PATH]; 
		//DWORD dwBufferSize = MAX_PATH; 
		HANDLE hDumpFile; 
		SYSTEMTIME stLocalTime; 
		//MINIDUMP_EXCEPTION_INFORMATION ExpParam;

		GetLocalTime( &stLocalTime ); 

		CreateDirectory( TEXT("Dump"), NULL );

		_stprintf_s( szFileName, TEXT("Dump/%04u%02u%02u%02u%02u%02u.dmp"), 
			stLocalTime.wYear, 
			stLocalTime.wMonth, 
			stLocalTime.wDay, 
			stLocalTime.wHour, 
			stLocalTime.wMinute, 
			stLocalTime.wSecond
			);

		// Try to create file for minidump.
		hDumpFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		// Write a minidump.

		if(hDumpFile) {
			MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

			DumpExceptionInfo.ThreadId			= GetCurrentThreadId();
			DumpExceptionInfo.ExceptionPointers	= ExceptionInfo;
			DumpExceptionInfo.ClientPointers	= TRUE;

			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(), 
				hDumpFile, 
				//MiniDumpNormal, 
				s_minidumptype,
				ExceptionInfo ? &DumpExceptionInfo : NULL,
				//&DumpExceptionInfo,
				NULL, 
				NULL 
				);
			CloseHandle(hDumpFile);
		}

		return ret;
	}

	void CrashDump::WalkStack(LPEXCEPTION_POINTERS ExceptionInfo) {
		// Walk the stack.
		const unsigned int MAX_SYMBOL_NAME_LENGTH = 1024;
		// Variables for stack walking.
		HANDLE	Process		= GetCurrentProcess();
		HANDLE	Thread		= GetCurrentThread();
		DWORD64	Offset64	= 0;
		DWORD	Offset		= 0;
		DWORD	SymOptions	= SymGetOptions();
		SYMBOL_INFO*		Symbol		= (SYMBOL_INFO*) malloc( sizeof(SYMBOL_INFO) + MAX_SYMBOL_NAME_LENGTH );
		IMAGEHLP_LINE64		Line;
		STACKFRAME64		StackFrame;

		// Initialize stack frame.
		memset(&StackFrame, 0, sizeof(StackFrame));
#if defined(_WIN64)
		DWORD64 nOffset = ExceptionInfo->ContextRecord->Rip;
#else
		DWORD nOffset = ExceptionInfo->ContextRecord->Eip;
#endif
		StackFrame.AddrPC.Offset		= nOffset;
		StackFrame.AddrPC.Mode			= AddrModeFlat;
		StackFrame.AddrFrame.Offset		= nOffset;
		StackFrame.AddrFrame.Mode		= AddrModeFlat;
		StackFrame.AddrStack.Offset		= nOffset;
		StackFrame.AddrStack.Mode		= AddrModeFlat;
		StackFrame.AddrBStore.Mode		= AddrModeFlat;
		StackFrame.AddrReturn.Mode		= AddrModeFlat;

		// Set symbol options.
		SymOptions						|= SYMOPT_LOAD_LINES;
		SymOptions						|= SYMOPT_UNDNAME;
		SymOptions						|= SYMOPT_EXACT_SYMBOLS;
		SymSetOptions( SymOptions );

		// Initialize symbol.
		memset( Symbol, 0, sizeof(SYMBOL_INFO) + MAX_SYMBOL_NAME_LENGTH );
		Symbol->SizeOfStruct			= sizeof(SYMBOL_INFO);
		Symbol->MaxNameLen				= MAX_SYMBOL_NAME_LENGTH;

		// Initialize line number info.
		memset( &Line, 0, sizeof(Line) );
		Line.SizeOfStruct				= sizeof(Line);

		// Load symbols.
		SymInitialize( GetCurrentProcess(), ".", 1 );

		CreateDirectory( TEXT("Dump"), NULL );

		char szFileName[MAX_PATH]; 
		SYSTEMTIME stLocalTime; 
		GetLocalTime( &stLocalTime ); 

		sprintf_s( szFileName, ("Dump/%04u%02u%02u%02u%02u%02u.stk"), 
			stLocalTime.wYear, 
			stLocalTime.wMonth, 
			stLocalTime.wDay, 
			stLocalTime.wHour, 
			stLocalTime.wMinute, 
			stLocalTime.wSecond
			);

		FILE *pFileOut(NULL);
        if(fopen_s(&pFileOut, (const char*)&szFileName, "wt") != 0) {
			free(Symbol);
            return;
        }
		//ofstream ofs("test.txt");
		//ostringstream ossErr;
		fwrite("WalkStack:\n",strlen("WalkStack:\n"), 1, pFileOut);
		char	szErrorHist[4096] = ("");	
		bool bOuted = false;
		while( 1 ) {
			if( !StackWalk64( IMAGE_FILE_MACHINE_I386, Process, Thread, &StackFrame, ExceptionInfo->ContextRecord, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL ) )
				break;

			// Warning - ANSI_TO_TCHAR uses alloca which might not be safe during an exception handler - INVESTIGATE!
			if( SymFromAddr( Process, StackFrame.AddrPC.Offset, &Offset64, Symbol ) && SymGetLineFromAddr64( Process, StackFrame.AddrPC.Offset, &Offset, &Line ) ) {
				bOuted = true;
				sprintf_s(szErrorHist, ("{function:%s \n\tfile:%s\t [Line %i]}\n"), (Symbol->Name), Line.FileName, Line.LineNumber);
				//strncat( szErrorHist, sprintf(("%s [Line %i] \n "),(Symbol->Name), Line.LineNumber), strlen(szErrorHist) );
				//ossErr << szErrorHist;
				//ofs << szErrorHist;
				fwrite(szErrorHist,strlen(szErrorHist), 1, pFileOut);
				//debugf(TEXT("%45s   Line %4i of %s"), ANSI_TO_TCHAR(Symbol->Name), Line.LineNumber, ANSI_TO_TCHAR(Line.FileName) );
			}
		}
		
		fclose(pFileOut);

		SymCleanup( Process );
		free( Symbol );
	}

	void CrashDump::HandleInvalidParameter( const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved ) {
		terminator();
		////MDRawAssertionInfo assertion;
		////memset(&assertion, 0, sizeof(assertion));
		////_snwprintf_s(reinterpret_cast<wchar_t*>(assertion.expression),
		////	sizeof(assertion.expression) / sizeof(assertion.expression[0]),
		////	_TRUNCATE, L"%s", expression);
		////_snwprintf_s(reinterpret_cast<wchar_t*>(assertion.function),
		////	sizeof(assertion.function) / sizeof(assertion.function[0]),
		////	_TRUNCATE, L"%s", function);
		////_snwprintf_s(reinterpret_cast<wchar_t*>(assertion.file),
		////	sizeof(assertion.file) / sizeof(assertion.file[0]),
		////	_TRUNCATE, L"%s", file);
		////assertion.line = line;
		////assertion.type = MD_ASSERTION_INFO_TYPE_INVALID_PARAMETER;

		//// Make up an exception record for the current thread and CPU context
		//// to make it possible for the crash processor to classify these
		//// as do regular crashes, and to make it humane for developers to
		//// analyze them.
		//EXCEPTION_RECORD exception_record = {};
		//CONTEXT exception_context = {};
		//EXCEPTION_POINTERS exception_ptrs = { &exception_record, &exception_context };
		//RtlCaptureContext(&exception_context);
		//exception_record.ExceptionCode = STATUS_NONCONTINUABLE_EXCEPTION;

		//// We store pointers to the the expression and function strings,
		//// and the line as exception parameters to make them easy to
		//// access by the developer on the far side.
		//exception_record.NumberParameters = 3;
		//exception_record.ExceptionInformation[0] =
		//	reinterpret_cast<ULONG_PTR>(expression);
		//exception_record.ExceptionInformation[1] =
		//	reinterpret_cast<ULONG_PTR>(file);
		//exception_record.ExceptionInformation[2] = line;

		//UnhandledExceptionFilter(&exception_ptrs);
	}

	void CrashDump::HandlePureVirtualCall()
	{
		terminator();
		////MDRawAssertionInfo assertion;
		////memset(&assertion, 0, sizeof(assertion));
		////assertion.type = MD_ASSERTION_INFO_TYPE_PURE_VIRTUAL_CALL;

		//// Make up an exception record for the current thread and CPU context
		//// to make it possible for the crash processor to classify these
		//// as do regular crashes, and to make it humane for developers to
		//// analyze them.
		//EXCEPTION_RECORD exception_record = {};
		//CONTEXT exception_context = {};
		//EXCEPTION_POINTERS exception_ptrs = { &exception_record, &exception_context };
		//RtlCaptureContext(&exception_context);
		//exception_record.ExceptionCode = STATUS_NONCONTINUABLE_EXCEPTION;

		//// We store pointers to the the expression and function strings,
		//// and the line as exception parameters to make them easy to
		//// access by the developer on the far side.
		////exception_record.NumberParameters = 3;
		////exception_record.ExceptionInformation[0] =
		////	reinterpret_cast<ULONG_PTR>(expression);
		////exception_record.ExceptionInformation[1] =
		////	reinterpret_cast<ULONG_PTR>(&assertion.file);
		////exception_record.ExceptionInformation[2] = assertion.line;
		//UnhandledExceptionFilter(&exception_ptrs);
	}

	void CrashDump::terminator() {
		// int* hsz = NULL; *hsz = 0;
		abort();
	}

	LPTOP_LEVEL_EXCEPTION_FILTER CrashDump::GetPreviousFilter() const {
		return s_previousFilter;
	}
}

#endif
