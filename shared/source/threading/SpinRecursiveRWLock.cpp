
#include "SpinRecursiveRWLock.h"

namespace thd {

	SHARED_DLL_DECL CSpinRecursiveRWLock::thread_table_t CSpinRecursiveRWLock::s_thdContext;
	SHARED_DLL_DECL CSpinRecursiveRWLock::instc_table_t CSpinRecursiveRWLock::s_instcIdxs;

}
