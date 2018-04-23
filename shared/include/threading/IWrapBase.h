/*
 * File:   IWrapBase.h
 * Author: Jehu Shaw
 *
 */

#ifndef __IWRAPBASE_H_
#define __IWRAPBASE_H_

namespace thd {

	class wrap_error : public std::runtime_error {
	public:
		explicit wrap_error(const std::string& s):std::runtime_error(s) {

		}

		virtual ~wrap_error() throw() { }
	};

	// If the instance is used,than delete it throw used_error .
	class wrap_used_error : public wrap_error{
	public:
		explicit wrap_used_error(const std::string& s):wrap_error(s) {

		}

		virtual ~wrap_used_error() throw() { }
	};

	// If the lock is timeout,than throw wrap_timeout_error .
	class wrap_timeout_error : public wrap_error {
	public:
		explicit wrap_timeout_error(const std::string& s):wrap_error(s) {

		}

		virtual ~wrap_timeout_error() throw() { }
	};

	// If read only but want get write handle,than throw wrap_timeout_error .
	class wrap_readonly_error : public wrap_error {
	public:
		explicit wrap_readonly_error(const std::string& s):wrap_error(s) {

		}

		virtual ~wrap_readonly_error() throw() { }
	};

	/** An abstract base class for synchronization primitives.
	*/
	class IWrapBase
	{
	public:
		virtual ~IWrapBase() {}
	};

	template<class UseType>
	class IWrapData : virtual public IWrapBase
	{
	protected:	
		virtual UseType* GetDataPtr() = 0;

		template<class DataType> friend class CScopedWrapLock;
		template<class DataType> friend class CScopedWrapRWLock;
	};

}; // namespace thd

#endif // __IWRAPBASE_H_

/* end of header file */
