/*
 * File:   IEnvLockBase.h
 * Author: Jehu Shaw
 *
 */

#ifndef IENVLOCKBASE_H
#define IENVLOCKBASE_H

#ifndef IGNORE_THREAD_SAFE_CHECK
namespace util {
	template<class T> class CXqxTable1StoreS;
}
#endif

namespace thd {

	class env_lock_error : public std::runtime_error {
	public:
		explicit env_lock_error(const std::string& s):std::runtime_error(s) {

		}

		virtual ~env_lock_error() throw() { }
	};

	// If the instance is used,than delete it throw env_lock_used_error .
	class env_lock_used_error : public env_lock_error{
	public:
		explicit env_lock_used_error(const std::string& s):env_lock_error(s) {

		}

		virtual ~env_lock_used_error() throw() { }
	};

	// If the lock is timeout,than throw env_lock_timeout_error .
	class env_lock_timeout_error : public env_lock_error {
	public:
		explicit env_lock_timeout_error(const std::string& s):env_lock_error(s) {

		}

		virtual ~env_lock_timeout_error() throw() { }
	};

	// If read only but want get write handle,than throw env_lock_readonly_error .
	class env_lock_readonly_error : public env_lock_error {
	public:
		explicit env_lock_readonly_error(const std::string& s):env_lock_error(s) {

		}

		virtual ~env_lock_readonly_error() throw() { }
	};

	/** An abstract base class for synchronization primitives.
	*/
	class IEnvLockBase
	{
	public:
		virtual ~IEnvLockBase() {}
	};

	template<class UseType>
	class IEnvLockData : virtual public IEnvLockBase
	{
	protected:	
		virtual UseType* GetDataPtr() = 0;

#ifndef IGNORE_THREAD_SAFE_CHECK
		virtual util::CXqxTable1StoreS<bool>& GetEnvInfo() = 0;
#endif
		template<class DataType> friend class CScopedEnvLock;
		template<class DataType> friend class CScopedEnvRWLock;
	};

}; // namespace thd

#endif // IENVLOCKBASE_H

/* end of header file */
