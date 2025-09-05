/*
 * File:   ThreadBase.h
 * Author: Jehu Shaw
 *
 */

#ifndef THREAD_BASE_H
#define THREAD_BASE_H

namespace thd {

class SHARED_DLL_DECL ThreadBase
{
public:
	ThreadBase() {}
	virtual ~ThreadBase() {}

	virtual bool OnRun() = 0;

	virtual void OnShutdown() = 0;
};

}

#endif // THREAD_BASE_H

