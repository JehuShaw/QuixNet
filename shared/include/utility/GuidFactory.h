/* 
 * File:   GuidFactory.h
 * Author: Jehu Shaw
 *
 * Created on 2011_7_26, 11:09
 */

#ifndef GUIDFACTORY_H
#define	GUIDFACTORY_H

#include "Common.h"
#include "Singleton.h"
#include "AtomicLock.h"

namespace util {
/**
 * Create 64bits guid,  Unix timestamp,  since 1970.1.1 to 2149.1.1
 */
class SHARED_DLL_DECL CGuidFactory
	: public util::Singleton<CGuidFactory>
{
public:
	CGuidFactory();
    /**
     * create 64 bits guid 
     * @return 
     */
    uint64_t CreateGuid();
    /**
     * Set code field
     * @param value
     */
	inline void SetCodeInt16(uint16_t value){
		atomic_xchg16(&m_code.u16Code, value);
	}
    /**
     * Set code field high 8bits
     * @param value
     */
	inline void SetCodeInt8H(uint8_t value) {
		atomic_xchg8(&m_code.u8CodeH, value);
	}
    /**
     * Set code field low 8bits
     * @param value
     */
	inline void SetCodeInt8L(uint8_t value) {
		atomic_xchg8(&m_code.u8CodeL, value);
	}

	/**
     * Get code field 16bit 
     * @param value
     */
	inline uint16_t GetCodeInt16() const {
		return (uint16_t)m_code.u16Code;
	}
    /**
     * Get code field high 8bits
     * @param value
     */
	inline uint8_t GetCodeInt8H() const {
		return (uint8_t)m_code.u8CodeH;
	}
    /**
     * Get code field low 8bits
     * @param value
     */
	inline uint8_t GetCodeInt8L() const {
		return (uint8_t)m_code.u8CodeL;
	}

private:
	CGuidFactory(const CGuidFactory &): m_uCount(0) {
		atomic_xchg16(&m_code.u16Code, 0);
	}

	CGuidFactory& operator = (const CGuidFactory &) { return *this; }

private:
	volatile union {
		uint16_t u16Code;
		struct {
			uint8_t u8CodeL;
			uint8_t u8CodeH;
		};
	} m_code;
	volatile uint16_t m_uCount;
};

}

#endif	/* GUIDFACTORY_H */

