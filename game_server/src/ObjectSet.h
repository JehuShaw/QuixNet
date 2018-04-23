/* 
 * File:   ObjectSet.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __OBJECTSET_H__
#define __OBJECTSET_H__

#include "IObject.h"
#include "AutoPointer.h"
#include "PoolBase.h"
#include <set>


template<class key_type>
struct IObjectCompare
{
    bool operator()(const util::CAutoPointer<IObject<key_type> >& pObject1
        , const util::CAutoPointer<IObject<key_type> >& pObject2)const
    {
		bool bInValid1 = pObject1.IsInvalid();
		bool bInValid2 = pObject2.IsInvalid();
		if(!bInValid1 && !bInValid2) {
			return pObject1->GetID() < pObject2->GetID();
		} else if(bInValid1 && !bInValid2) {
			return true;
		}
		return false;
    }
};

template<class key_type>
class CObjectKey : public IObject<key_type>, public util::PoolBase<CObjectKey<key_type> > {
public:
    CObjectKey(key_type objectId) :m_objectId(objectId)  {}
    
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

    typedef typename std::set<value_type, IObjectCompare<key_type> > set_type;

    typedef typename set_type::const_iterator const_iterator;

    bool Has(const value_type& pObject) const {
        if(pObject.IsInvalid()) {
            return false;
        }
        return m_objects.end() != m_objects.find(pObject);
    }

    bool Has(const key_type &key) const {
        util::CAutoPointer<IObject<key_type> > pObjectKey(
            new CObjectKey<key_type>(key));
        return m_objects.end() != m_objects.find(pObjectKey);
    }

    value_type Find(const key_type &key) {

        value_type pObjectKey(new CObjectKey<key_type>(key));
        const_iterator it = m_objects.find(pObjectKey);
        if(m_objects.end() == it) {
            return value_type();
        }
        return *it;
    }

    bool Insert(const value_type& pObject) {
        if(pObject.IsInvalid()) {
            return false;
        }
        return m_objects.insert(pObject).second;
    }

    value_type InsertEx(const value_type& pObject) 
    {
        if(pObject.IsInvalid()) {
            return value_type();
        }
        set_type::_Pairib pairIB = m_objects.insert(pObject);
        if(pairIB.second) {
            return value_type();
        }
        return *pairIB.first;
    }

    void Erase(const key_type& key) {
        value_type pObjectKey(new CObjectKey<key_type>(key));
        m_objects.erase(pObjectKey);
    }

	void Erase(const value_type& value) {
		m_objects.erase(value);
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

#endif /* __OBJECTSET_H__ */
