/*
 * File:   CrashDump.h
 * Author: Jehu Shaw
 *
 * Created on 2010_7_9 21:27
 */
#ifndef CRASHDUMP_H
#define	CRASHDUMP_H

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined( _WIN64 )

#include "Common.h"
#include <Windows.h>
#include <DbgHelp.h>

namespace Frame
{
	class SHARED_DLL_DECL CrashDump
	{
	public:
		static CrashDump *Begin(LPTOP_LEVEL_EXCEPTION_FILTER CBFilter = NULL, MINIDUMP_TYPE minidumptype = MiniDumpNormal);
		static void End();

		LPTOP_LEVEL_EXCEPTION_FILTER GetPreviousFilter() const;

	private:
		CrashDump(LPTOP_LEVEL_EXCEPTION_FILTER CBFilter = NULL, MINIDUMP_TYPE minidumptype = MiniDumpNormal);
		~CrashDump();

		static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *lpTopLevelExceptionFilter);

	#if _MSC_VER >= 1400  // MSVC 2005/8
		// This function will be called by some CRT functions when they detect
		// that they were passed an invalid parameter.  Note that in _DEBUG builds,
		// the CRT may display an assertion dialog before calling this function,
		// and the function will not be called unless the assertion dialog is
		// dismissed by clicking "Ignore."
		static void HandleInvalidParameter(const wchar_t* expression,
			const wchar_t* function,
			const wchar_t* file,
			unsigned int line,
			uintptr_t reserved);
	#endif  // _MSC_VER >= 1400

		// This function will be called by the CRT when a pure virtual
		// function is called.
		static void HandlePureVirtualCall();

		static INT CreateMiniDump(LPEXCEPTION_POINTERS ExceptionInfo);
		static void WalkStack(LPEXCEPTION_POINTERS ExceptionInfo) ;

	private:
		static void terminator();

	private:
		static LPTOP_LEVEL_EXCEPTION_FILTER s_previousFilter;

		static LPTOP_LEVEL_EXCEPTION_FILTER s_CBFilter;

	#if _MSC_VER >= 1400  // MSVC 2005/8
		// Beginning in VC 8, the CRT provides an invalid parameter handler that will
		// be called when some CRT functions are passed invalid parameters.  In
		// earlier CRTs, the same conditions would cause unexpected behavior or
		// crashes.
		static _invalid_parameter_handler s_previous_iph;
	#endif  // _MSC_VER >= 1400

		// The CRT allows you to override the default handler for pure
		// virtual function calls.
		static _purecall_handler s_previous_pch;

		static CrashDump *s_pSelf;

		static MINIDUMP_TYPE s_minidumptype;
	};
}

#endif

#endif // CRASHDUMP_H
