/*
 * File:   ICacheMemory.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef ICACHEMEMORY_H
#define ICACHEMEMORY_H

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

    virtual MCResult AddToDB() = 0;

    virtual MCResult LoadFromDB(
		bool bDBCas = true) = 0;

    virtual MCResult UpdateToDB(
		bool bDBCas = true) = 0;

    virtual MCResult DeleteFromDB() = 0;

	virtual MCResult LoadCasFromDB() = 0;

    virtual const std::string& GetKey() const = 0;

    virtual void SetValue(util::CTransferStream& inValue) = 0;

	virtual uint64_t GetCas() const = 0;

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

#endif /* ICACHEMEMORY_H */
