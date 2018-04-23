/*
 * File:   ICacheMemory.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __ICACHEMEMORY_H__
#define __ICACHEMEMORY_H__

#include "AutoPointer.h"
#include "Common.h"
#include "MCResult.h"


namespace util {
	class CTransferStream;
}

class ICacheMemory
{
public:

	ICacheMemory() : m_bLock(false) {}
	virtual ~ICacheMemory() {}

    virtual MCResult AddToDB(
		bool bResetFlag = false) = 0;

    virtual MCResult LoadFromDB(
		bool bDBCas = true,
		const int32_t* pInFlag = NULL,
		int32_t* pOutFlag = NULL) = 0;

    virtual MCResult UpdateToDB(
		bool bDBCas = true,
		bool bResetFlag = false) = 0;

    virtual MCResult DeleteFromDB() = 0;

    virtual const std::string& GetKey() const = 0;

    virtual void SetValue(util::CTransferStream& inValue) = 0;

protected:
	friend class CCacheMemoryManager;

	virtual int Index() const = 0;

	virtual void Index(int nIndex) = 0;

    virtual uint64_t LastActivityTime() const = 0;

    virtual void LastActivityTime(uint64_t n64ActivTime) = 0;

	virtual uint8_t ChangeType() const = 0;

	virtual void ChangeValue(util::CTransferStream& inValue) = 0;

private:
	volatile bool m_bLock;

};

#endif /* __ICACHEMEMORY_H__ */
