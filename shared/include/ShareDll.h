/*
 * File:   ShareDll.h
 * Author: Jehu Shaw
 *
 */

#ifndef _SHAREDLL_H_
#define _SHAREDLL_H_

/*
service system exports/imports
*/

#ifdef SHARE_DLL
	#ifdef SHARED_EXPORTS
		#define SHARED_DLL_DECL __declspec(dllexport)
		#define SERVICE_DLL_DECL __declspec(dllimport)
	#else
		#define SHARED_DLL_DECL __declspec(dllimport)
		#define SERVICE_DLL_DECL __declspec(dllexport)
	#endif
#else
	#define SHARED_DLL_DECL
	#define SERVICE_DLL_DECL
#endif


#endif /*_SHAREDLL_H_*/
