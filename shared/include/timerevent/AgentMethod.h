/* 
 * File:   AgentMethod.h
 * Author: Jehu Shaw
 *
 * Created on 2010_11_12, 9:39
 */

#ifndef AGENTMETHOD_H
#define	AGENTMETHOD_H

#include <assert.h>
#include <string>
#include "WeakPointer.h"

namespace evt
{
	class ArgumentBase {
	public:
		virtual ~ArgumentBase() {}

		/**
		 * reset body data;
		 */
		virtual void Reset() {};
	};

    class MethodRIP1Base {
    public:
        virtual int Invoke(const util::CWeakPointer<ArgumentBase>& arg) = 0;
		virtual bool Equal(const MethodRIP1Base& base) const = 0;
		virtual ~MethodRIP1Base() {}
    };

    // For member function callback
    template<typename T>
    class MemberMethodRIP1 : public MethodRIP1Base, public util::PoolBase<MemberMethodRIP1<T> > {
     public:
        typedef int (T::* MethodType)(const util::CWeakPointer<ArgumentBase>& arg);

        MemberMethodRIP1(util::CWeakPointer<T> p, MethodType m) : m_pObject(p), m_pMethod(m) {
			assert(!m_pObject.IsInvalid());
			assert(NULL != m_pMethod);
		}

        MemberMethodRIP1(const MemberMethodRIP1& orig) : m_pObject(orig.m_pObject), m_pMethod(orig.m_pMethod) {
			assert(!m_pObject.IsInvalid());
			assert(NULL != m_pMethod);
		}


        MemberMethodRIP1& operator= (const MemberMethodRIP1& orig){
            m_pObject = orig.m_pObject;
            m_pMethod = orig.m_pMethod;
			assert(!m_pObject.IsInvalid());
			assert(NULL != m_pMethod);
            return *this;
        }

		bool Equal(const MethodRIP1Base& pBase) const {
			const MemberMethodRIP1* pRight = dynamic_cast<const MemberMethodRIP1*>(&pBase);
			if(NULL == pRight) {
				return false;
			}
			return m_pObject == pRight->m_pObject && m_pMethod == pRight->m_pMethod;
		}

        int Invoke(const util::CWeakPointer<ArgumentBase>& arg){
			util::CAutoPointer<T> pObject(m_pObject.GetStrong());
			if(pObject.IsInvalid()) {
				assert(false);
				return FALSE;
			}
			T* ptr = pObject.operator->();
			return (ptr->*m_pMethod)(arg);
        }

    private:
        util::CWeakPointer<T> m_pObject;
        MethodType m_pMethod;
    };

    class GlobalMethodRIP1 : public MethodRIP1Base, public util::PoolBase<GlobalMethodRIP1> {
    public:
        typedef int (* MethodType)(const util::CWeakPointer<ArgumentBase>& arg);

        GlobalMethodRIP1(MethodType m):m_pMethod(m){ assert(NULL != m_pMethod); }

        GlobalMethodRIP1(const GlobalMethodRIP1& orig) : m_pMethod(orig.m_pMethod) {
			assert(NULL != m_pMethod);
        }

        GlobalMethodRIP1& operator= (const GlobalMethodRIP1& orig){
            m_pMethod = orig.m_pMethod;
			assert(NULL != m_pMethod);
            return *this;
        }

		bool Equal(const MethodRIP1Base& base) const {
			const GlobalMethodRIP1* pRight = dynamic_cast<const GlobalMethodRIP1*>(&base);
			if(NULL == pRight) {
				return false;
			}
			return m_pMethod == pRight->m_pMethod;
		}

        int Invoke(const util::CWeakPointer<ArgumentBase>& arg){
            return (*m_pMethod)(arg);
        }

    private:
        MethodType m_pMethod;
    };

////////////////////////////// return std::string ////////////////////////////////////////////
	class MethodRSBase {
	public:
		virtual std::string Invoke() = 0;
		virtual bool Equal(const MethodRSBase& base) const = 0;
		virtual ~MethodRSBase() {}
	};

	// For member function callback
	template<typename T>
	class MemberMethodRS : public MethodRSBase, public util::PoolBase<MemberMethodRS<T> > {
	public:
		typedef std::string (T::* MethodType)();

		MemberMethodRS(util::CWeakPointer<T> p, MethodType m) : m_pObject(p), m_pMethod(m) {
			assert(!m_pObject.IsInvalid());
			assert(NULL != m_pMethod);
		}

		MemberMethodRS(const MemberMethodRS& orig) : m_pObject(orig.m_pObject), m_pMethod(orig.m_pMethod) {
			assert(!m_pObject.IsInvalid());
			assert(NULL != m_pMethod);
		}


		MemberMethodRS& operator= (const MemberMethodRS& orig){
			m_pObject = orig.m_pObject;
			m_pMethod = orig.m_pMethod;
			assert(!m_pObject.IsInvalid());
			assert(NULL != m_pMethod);
			return *this;
		}

		bool Equal(const MethodRSBase& base) const {
			const MemberMethodRS* pRight = dynamic_cast<const MemberMethodRS*>(&base);
			if(NULL == pRight) {
				return false;
			}
			return m_pObject == pRight->m_pObject && m_pMethod == pRight->m_pMethod;
		}

		std::string Invoke() {
			util::CAutoPointer<T> pObject(m_pObject.GetStrong());
			if(pObject.IsInvalid()) {
				assert(false);
				return std::string();
			}
			T* ptr = pObject.operator->();
			return (ptr->*m_pMethod)();
		}

	private:
		util::CWeakPointer<T> m_pObject;
		MethodType m_pMethod;
	};

	class GlobalMethodRS : public MethodRSBase, public util::PoolBase<GlobalMethodRS> {
	public:
		typedef std::string (* MethodType)();

		GlobalMethodRS(MethodType m):m_pMethod(m){ assert(NULL != m_pMethod); }

		GlobalMethodRS(const GlobalMethodRS& orig) : m_pMethod(orig.m_pMethod) {
			assert(NULL != m_pMethod);
		}

		GlobalMethodRS& operator= (const GlobalMethodRS& orig){
			m_pMethod = orig.m_pMethod;
			assert(NULL != m_pMethod);
			return *this;
		}

		bool Equal(const MethodRSBase& base) const {
			const GlobalMethodRS* pRight = dynamic_cast<const GlobalMethodRS*>(&base);
			if(NULL == pRight) {
				return false;
			}
			return m_pMethod == pRight->m_pMethod;
		}

		std::string Invoke(){
			return (*m_pMethod)();
		}

	private:
		MethodType m_pMethod;
	};
}
#endif	/* AGENTMETHOD_H */

