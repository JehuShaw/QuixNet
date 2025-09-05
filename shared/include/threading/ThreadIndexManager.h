/*
 * File:   ThreadIndexManager.h
 * Author: Jehu Shaw
 *
 * Created on 2019年06月23日, 上午10:32
 */

#ifndef THREADINDEXMANAGER_H
#define	THREADINDEXMANAGER_H

#include "Singleton.h"
#include "XqxTableIndexS.h"

namespace thd {

	class SHARED_DLL_DECL CThreadIndexManager 
		: public util::Singleton<CThreadIndexManager> {
	public:
		CThreadIndexManager(){}

        uint64_t Get() {
			uint64_t& thdIndex = GetThreadIndex();
			if(XQXTABLE_INDEX_NIL == thdIndex) {
				thdIndex = s_thdIndexTable.Add();
			}
			return thdIndex;
		}

		bool Remove() {
			uint64_t thdIndex = GetThreadIndex();
			if(XQXTABLE_INDEX_NIL != thdIndex) {
				return s_thdIndexTable.Remove(thdIndex);
			}
			return false;
		}

	private:
		static uint64_t& GetThreadIndex()  {
#if COMPILER == COMPILER_MICROSOFT
			__declspec(thread) static uint64_t s_thdIndex = XQXTABLE_INDEX_NIL;
#elif COMPILER == COMPILER_GNU
			static __thread uint64_t s_thdIndex = XQXTABLE_INDEX_NIL;
#elif COMPILER == COMPILER_BORLAND
			static uint64_t __thread s_thdIndex = XQXTABLE_INDEX_NIL;
#else
#error "not support";
#endif
			return s_thdIndex;
		}

		typedef util::CXqxTableIndexS index_table_t;

		static index_table_t s_thdIndexTable;

	private:
		CThreadIndexManager(const CThreadIndexManager& orig) {}
		CThreadIndexManager& operator=(const CThreadIndexManager& right) { return *this; }
	};
}

#endif  // THREADINDEXMANAGER_H
