/* 
 * File:   ObjectSet.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef OBJECTSET_H
#define OBJECTSET_H

#include "IObject.h"
#include "AutoPointer.h"
#include "PoolBase.h"
#include <set>

namespace util {

	template<class value_type>
	struct IObjectCompare
	{
		bool operator()(const value_type& pObject1
			, const value_type& pObject2)const
		{
			bool bInValid1 = pObject1.IsInvalid();
			bool bInValid2 = pObject2.IsInvalid();
			if (!bInValid1 && !bInValid2) {
				return pObject1->GetID() < pObject2->GetID();
			}
			else if (bInValid1 && !bInValid2) {
				return true;
			}
			return false;
		}
	};

	template<class key_type>
	class CObjectKey : public IObject<key_type>, public util::PoolBase<CObjectKey<key_type> > {
	public:
		CObjectKey(key_type objectId) : m_objectId(objectId) {}

		key_type GetID() const {
			return m_objectId;
		}
	private:
		key_type m_objectId;
	};

	template<class key_type>
	class CObjectSet {
	public:
		CObjectSet() {}
		~CObjectSet() {}

		typedef typename util::CAutoPointer<IObject<key_type> > value_type;

		typedef typename std::set<value_type, IObjectCompare<value_type> > set_type;

		typedef typename set_type::const_iterator const_iterator;

		typedef typename set_type::iterator iterator;

		bool Has(const value_type& pObject) const {
			if (pObject.IsInvalid()) {
				return false;
			}
			return m_objects.end() != m_objects.find(pObject);
		}

		bool Has(key_type key) const {
			CObjectKey<key_type> objectKey(key);
			value_type pObjectKey(&objectKey, false);
			return m_objects.end() != m_objects.find(pObjectKey);
		}

		value_type Find(key_type key) const {
			CObjectKey<key_type> objectKey(key);
			value_type pObjectKey(&objectKey, false);
			const_iterator it(m_objects.find(pObjectKey));
			if (m_objects.end() == it) {
				return value_type();
			}
			return *it;
		}

		bool LowerBound(key_type key, iterator& outIt) {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			outIt = m_objects.lower_bound(value_type(pObjectKey));
			return m_objects.end() == outIt || (*outIt)->GetID() != key;
		}

		bool UpperBound(key_type key, iterator& outIt) {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			outIt = m_objects.upper_bound(value_type(pObjectKey));
			return m_objects.end() == outIt || (*outIt)->GetID() != key;
		}


		bool Insert(value_type pObject) {
			if (pObject.IsInvalid()) {
				return false;
			}
			return m_objects.insert(pObject).second;
		}

		iterator Insert(const iterator& it, value_type pObject) {
			return m_objects.insert(it, pObject);
		}

		void Erase(key_type key) {
			CObjectKey<key_type> objectKey(key);
			value_type pObjectKey(&objectKey, false);
			m_objects.erase(pObjectKey);
		}

		void Erase(const value_type& value) {
			m_objects.erase(value);
		}

		value_type EraseEx(key_type key) {
			CObjectKey<key_type> objectKey(key);
			value_type pObjectKey(&objectKey, false);
			const_iterator it(m_objects.find(pObjectKey));
			if (m_objects.end() == it) {
				return value_type();
			}
			value_type oldObject(*it);
			m_objects.erase(it);
			return oldObject;
		}

		void Clear() {
			m_objects.clear();
		}

		bool Empty() const {
			return m_objects.empty();
		}

		unsigned Size() const {
			return (unsigned)m_objects.size();
		}

		const_iterator Begin() const {
			return m_objects.begin();
		}

		const_iterator End() const {
			return m_objects.end();
		}

	private:
		set_type m_objects;
	};

	template<class key_type>
	class CWeakObjectSet {
	public:
		CWeakObjectSet() {}
		~CWeakObjectSet() {}

		typedef typename util::CWeakPointer<IObject<key_type> > value_type;

		typedef typename std::set<value_type, IObjectCompare<value_type> > set_type;

		typedef typename set_type::const_iterator const_iterator;

		typedef typename set_type::iterator iterator;

		bool Has(const value_type& pObject) const {
			if (pObject.IsInvalid()) {
				return false;
			}
			return m_objects.end() != m_objects.find(pObject);
		}

		bool Has(key_type key) const {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(&objectKey, false);
			return m_objects.end() != m_objects.find(value_type(pObjectKey));
		}

		value_type Find(key_type key) const {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			const_iterator it(m_objects.find(value_type(pObjectKey)));
			if (m_objects.end() == it) {
				return value_type();
			}
			return *it;
		}

		bool LowerBound(key_type key, iterator& outIt) {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			outIt = m_objects.lower_bound(value_type(pObjectKey));
			return m_objects.end() == outIt || (*outIt)->GetID() != key;
		}

		bool UpperBound(key_type key, iterator& outIt)  {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			outIt = m_objects.upper_bound(value_type(pObjectKey));
			return m_objects.end() == outIt || (*outIt)->GetID() != key;
		}

		bool Insert(value_type pObject) {
			if (pObject.IsInvalid()) {
				return false;
			}
			return m_objects.insert(pObject).second;
		}

		void Insert(const iterator& it, value_type pObject) {
			m_objects.insert(it, pObject);
		}

		void Erase(key_type key) {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			m_objects.erase(value_type(pObjectKey));
		}

		void Erase(const value_type& value) {
			m_objects.erase(value);
		}

		value_type EraseEx(key_type key) {
			CObjectKey<key_type> objectKey(key);
			util::CAutoPointer<IObject<key_type> > pObjectKey(
				&objectKey, false);
			const_iterator it(m_objects.find(value_type(pObjectKey)));
			if (m_objects.end() == it) {
				return value_type();
			}
			value_type oldObject(*it);
			m_objects.erase(it);
			return oldObject;
		}

		void Clear() {
			m_objects.clear();
		}

		bool Empty() const {
			return m_objects.empty();
		}

		unsigned Size() const {
			return (unsigned)m_objects.size();
		}

		const_iterator Begin() const {
			return m_objects.begin();
		}

		const_iterator End() const {
			return m_objects.end();
		}

	private:
		set_type m_objects;
	};
} // namespace util

#endif /* OBJECTSET_H */
