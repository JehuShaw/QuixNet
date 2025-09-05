/*
 * File:   GlobalIDFactory.h
 * Author: Jehu Shaw
 *
 * Created on 2011_7_26, 11:09
 */

#ifndef GLOBALIDFACTORY_H
#define	GLOBALIDFACTORY_H

#include "Singleton.h"
#include "GuidFactory.h"
#include "Common.h"

namespace util {
	/**
	 * Generates a unique ID in the global process
	 */
	class CGlobalIDFactory : public Singleton<CGlobalIDFactory>
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
		CGuidFactory<ID_TYPE_GLOBAL> factory;
	};
}

#endif	/* GLOBALIDFACTORY_H */
