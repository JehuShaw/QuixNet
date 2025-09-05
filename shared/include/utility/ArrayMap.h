
#ifndef ARRAYMAP_H
#define ARRAYMAP_H

#include "ArrayList.h"

namespace util
{
	template <class KeyType, class ValueType>
	int defaultArrayMapComparison(const KeyType& a, const ValueType& b)
	{
		if(a < b) return -1; if (a == b) return 0; return 1;
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)=defaultArrayMapComparison<KeyType, ValueType> >
	class CArrayMap
	{
	public:
		CArrayMap();
		CArrayMap(const CArrayMap& original_copy);
		CArrayMap& operator= (const CArrayMap& original_copy);

		~CArrayMap();

		/// comparisonFunction must take a KeyType and a ValueType and return <0, ==0, or >0
		/// If the data type has comparison operators already defined then you can just use defaultComparison
		bool HasData(const KeyType key) const;

		unsigned int GetIndexFromKey(const KeyType key, bool* objectExists) const;

		ValueType GetElementFromKey(const KeyType key) const;

		unsigned int Insert(const KeyType key, const ValueType data);

		unsigned int Remove(const KeyType key);

		ValueType& operator[] (const unsigned int index) const;

		void RemoveAtIndex(const unsigned int index);

		void InsertAtIndex(const ValueType data, const unsigned int index);

		void InsertAtEnd(const ValueType data);

		void Del(const unsigned int num = 1);

		void Clear(void);

		unsigned int Size(void) const;

	protected:
		CArrayList<ValueType> orderedList;
	};

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	CArrayMap<KeyType, ValueType, comparisonFunction>::CArrayMap()
	{
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	CArrayMap<KeyType, ValueType, comparisonFunction>::CArrayMap(const CArrayMap& orig)
	{
		orderedList = orig.orderedList;
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	CArrayMap<KeyType, ValueType, comparisonFunction>& CArrayMap<KeyType, ValueType, comparisonFunction>::operator= (const CArrayMap& right)
	{
		orderedList = right.orderedList;
		return *this;
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	CArrayMap<KeyType, ValueType, comparisonFunction>::~CArrayMap()
	{
		Clear();
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	bool CArrayMap<KeyType, ValueType, comparisonFunction>::HasData(const KeyType key) const
	{
		bool objectExists = false;
		GetIndexFromKey(key, &objectExists);
		return objectExists;
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	ValueType CArrayMap<KeyType, ValueType, comparisonFunction>::GetElementFromKey(const KeyType key) const
	{
		bool objectExists = false;
		unsigned int index = GetIndexFromKey(key, &objectExists);
		if(!objectExists) {
			return ValueType();
		}
		return orderedList[index];
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	unsigned int CArrayMap<KeyType, ValueType, comparisonFunction>::GetIndexFromKey(const KeyType key, bool* objectExists) const
	{
		unsigned int index, upperBound, lowerBound;
		int res;

		if(orderedList.Size() == 0)
		{
			*objectExists = false;
			return 0;
		}

		upperBound = orderedList.Size() - 1;
		lowerBound = 0;
		index = orderedList.Size()/2;

#ifdef _MSC_VER
	#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
		while(true)
		{
			res = comparisonFunction(key, orderedList[index]);
			if(0 == res)
			{
				*objectExists = true;
				return index;
			}
			else if(res < 0)
			{
				if(0 == index)
				{
					*objectExists = false;
					return 0;
				} else {
					upperBound = index - 1;
				}
			}
			else// if (res>0)
			{
				if(index >= ARRAYLIST_MAX_INDEX - 1) {
					*objectExists = false;
					return ARRAYLIST_MAX_INDEX - 1;
				} else {
					lowerBound = index + 1;
				}
			}

			if(lowerBound > upperBound)
			{
				*objectExists = false;
				return lowerBound; // No match
			}

			index = lowerBound + (upperBound - lowerBound) / 2;
		}
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	unsigned int CArrayMap<KeyType, ValueType, comparisonFunction>::Insert(const KeyType key, const ValueType data)
	{
		bool objectExists;
		unsigned int index = GetIndexFromKey(key, &objectExists);

		// Don't allow duplicate insertion.
		if(objectExists) {
			return ARRAYLIST_MAX_INDEX;
		}

		if(index >= orderedList.Size())
		{
			orderedList.Insert(data);
			return orderedList.Size() - 1;
		}
		else
		{
			orderedList.Insert(data, index);
			return index;
		}		
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	unsigned int CArrayMap<KeyType, ValueType, comparisonFunction>::Remove(const KeyType key)
	{
		bool objectExists;
		unsigned int index = GetIndexFromKey(key, &objectExists);

		// Can't find the element to remove if this assert hits
		assert(objectExists == true);
		if(objectExists == false) {
			return ARRAYLIST_MAX_INDEX;
		}

		orderedList.RemoveAtIndex(index);
		return index;
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	void CArrayMap<KeyType, ValueType, comparisonFunction>::RemoveAtIndex(const unsigned int index)
	{
		orderedList.RemoveAtIndex(index);
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
		void CArrayMap<KeyType, ValueType, comparisonFunction>::InsertAtIndex(const ValueType data, const unsigned int index)
	{
		orderedList.Insert(data, index);
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
		void CArrayMap<KeyType, ValueType, comparisonFunction>::InsertAtEnd(const ValueType data)
	{
		orderedList.Insert(data);
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
		void CArrayMap<KeyType, ValueType, comparisonFunction>::Del(const unsigned int num)
	{
		orderedList.Del(num);
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	void CArrayMap<KeyType, ValueType, comparisonFunction>::Clear(void)
	{
		orderedList.Clear();
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	ValueType& CArrayMap<KeyType, ValueType, comparisonFunction>::operator[](const unsigned int index) const
	{
		return orderedList[index];
	}

	template <class KeyType, class ValueType, int (*comparisonFunction)(const KeyType&, const ValueType&)>
	unsigned int CArrayMap<KeyType, ValueType, comparisonFunction>::Size(void) const
	{
		return orderedList.Size();
	}
}

#endif // ARRAYMAP_H
