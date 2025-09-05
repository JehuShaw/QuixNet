/*
 * File:   EnvLockHelper.h
 * Author: Jehu Shaw
 *
 */

#ifndef ENVLOCKHELPER_H
#define ENVLOCKHELPER_H

#include <set>
#include <stdexcept>

namespace thd {
	class IEnvLockBase;
}

class CFieldBase {
public:
	virtual void doLock() { throw std::runtime_error("Method doLock() not implemented."); }
};

#define BeginEnvLock()\
std::set<CFieldBase*> setEvnLock;\
std::set<CFieldBase*>* pEnvLockSet = &setEvnLock;

#define SetEnvLock(fieldName, lockType, globalValue)\
class C##fieldName : public CFieldBase, public lockType {\
public:\
	C##fieldName(std::set<CFieldBase*>* pLockSet, thd::IEnvLockBase& vl) : lockType(vl) {\
		SetEnvFlag();\
		if(NULL != pLockSet) {\
			pLockSet->insert(this); \
		} else {\
			assert(false);\
		}\
	}\
	~C##fieldName() {\
		RemoveEnvFlag();\
	}\
	virtual void doLock() {\
		GetData();\
	}\
};\
C##fieldName fieldName(pEnvLockSet, globalValue)

#define SetEnvLockConst(fieldName, lockType, globalValue)\
class C##fieldName : public CFieldBase, public lockType {\
public:\
	C##fieldName(std::set<CFieldBase*>* pLockSet, thd::IEnvLockBase& vl) : lockType(vl) {\
		SetEnvFlag();\
		if(NULL != pLockSet) {\
			pLockSet->insert(this); \
		} else {\
			assert(false);\
		}\
	}\
	~C##fieldName() {\
		RemoveEnvFlag();\
	}\
	virtual void doLock() {\
		GetData();\
	}\
};\
const C##fieldName fieldName(pEnvLockSet, globalValue)

#define EndEnvLock()\
if(NULL != pEnvLockSet) {\
	std::set<CFieldBase*>::iterator it(pEnvLockSet->begin()); \
	for (; it != pEnvLockSet->end(); ++it) {\
		(*it)->doLock(); \
	}\
	pEnvLockSet = NULL;\
}

#endif // ENVLOCKHELPER_H

/* end of header file */
