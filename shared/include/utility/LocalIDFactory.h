/* 
 * File:   LocalIDFactory.h
 * Author: Jehu Shaw
 *
 * Created on 2011_7_26, 11:09
 */

#ifndef LOCALIDFACTORY_H
#define	LOCALIDFACTORY_H

#include "Singleton.h"
#include "GuidFactory.h"
#include "Common.h"

namespace util {
	/**
	 * Generates a unique ID in the local process
	 */
	class CLocalIDFactory : public Singleton<CLocalIDFactory>
	{
	public:
		/**
		 * Generate 64 bits id
		 * @return
		 */
		INLINE uint64_t GenerateID()
		{
			return factory.GenerateID();
		}

	private:
		CGuidFactory<ID_TYPE_LOCAL> factory;
	};
}

#endif	/* LOCALIDFACTORY_H */

