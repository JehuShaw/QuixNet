/*
 * File:   FunEnvHelper.h
 * Author: Jehu Shaw
 *
 */

#ifndef __FUNENVHELPER_H_
#define __FUNENVHELPER_H_

#include <set>
#include <stdexcept>

#define BeginFunEnv()\
class CFunEnv {\
private:\
	class CFieldBase {\
	public:\
		virtual void doLock() {throw std::runtime_error("Method doLock() not implemented.");};\
	};\
	static std::set<CFieldBase*>& GetLockSet() {\
		static std::set<CFieldBase*> lockSet;\
		return lockSet;\
	}

#define SetFunEnv(fieldName, lockType, globalValue)\
private:\
	class C##fieldName : public CFieldBase, public lockType {\
	public:\
		C##fieldName(): lockType(globalValue) {\
			GetLockSet().insert(this);\
		}\
		virtual void doLock() {\
			GetData();\
		}\
	};\
public:\
	C##fieldName fieldName;

#define EndFunEnv(objName)\
public:\
	CFunEnv() {\
		std::set<CFieldBase*>& lockSet = GetLockSet();\
		std::set<CFieldBase*>::iterator it(lockSet.begin());\
		for(; it != lockSet.end(); ++it) {\
			(*it)->doLock();\
		}\
	}\
};\
CFunEnv objName;

#endif // __FUNENVHELPER_H_

/* end of header file */
