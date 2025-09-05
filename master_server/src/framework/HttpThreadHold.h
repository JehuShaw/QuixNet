/* 
 * File:   HttpThreadHold.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef HTTPTHREADHOLD_H
#define	HTTPTHREADHOLD_H

#include "NodeDefines.h"
#include "ThreadBase.h"


class CHttpThreadHold
	: public thd::ThreadBase
{
public:
	CHttpThreadHold() : m_isStarted(true) {
	}

	~CHttpThreadHold() {
		Dispose();
	}

	void Dispose() {
		atomic_xchg8(&m_isStarted, false);
	}

	virtual bool OnRun();

	virtual void OnShutdown() {
		atomic_xchg8(&m_isStarted, false);
	}

private:
    volatile bool m_isStarted;
};

#endif	/* HTTPTHREADHOLD_H */

