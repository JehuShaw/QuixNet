
#ifndef ARRAYLIST_H
#define ARRAYLIST_H 

#include <assert.h>
#include <string.h>

static const unsigned int ARRAYLIST_MAX_INDEX = 4294967295U;

namespace util
{
	/// \brief Array based implementation of a list.
	template <class ValueType>
	class CArrayList
	{	
	public:
		/// Default constructor
		CArrayList();

		/// Destructor
		~CArrayList();
		
		/// Copy constructor
		/// \param[in]  orig The list to duplicate 
		CArrayList(const CArrayList& orig);
		
		/// Assign one list to another
		CArrayList& operator= (const CArrayList& right);
		
		/// Access an element by its index in the array 
		/// \param[in]  index The index into the array. 
		/// \return The element at index \a index. 
		ValueType& operator[] (const unsigned int index) const;
		
		/// Insert an element at position \a position in the list 
		/// \param[in] input The new element. 
		/// \param[in] index The index of the new element. 		
		void Insert(const ValueType input, const unsigned int index);
		
		/// Insert at the end of the list.
		/// \param[in] input The new element. 
		void Insert(const ValueType input);
		
		/// Replace the value at \a index by \a input.  If the size of
		/// the list is less than @em index, it increase the capacity of
		/// the list and fill slot with @em filler.
		/// \param[in] input The element to replace at index @em index. 
		/// \param[in] filler The element use to fill new allocated capacity. 
		/// \param[in] index The index of input in the list. 		
		void Replace(const ValueType input, const ValueType filler, const unsigned int index);
		
		/// Replace the last element of the list by \a input .
		/// \param[in] input The element used to replace the last element. 
		void Replace(const ValueType input);
		
		/// Delete the element at index \a index. 
		/// \param[in] index The index of the element to delete 
		void RemoveAtIndex(const unsigned int index);
		
		/// Delete the element at the end of the list 
		void Del(const unsigned num = 1);
		
		/// Returns the index of the specified item or ARRAYLIST_MAX_INDEX if not found
		/// \param[in] input The element to check for 
		/// \return The index of @em input in the list. 
		/// \retval ARRAYLIST_MAX_INDEX The object is not in the list
		/// \retval [Integer] The index of the element in the list
		unsigned int GetIndexOf(const ValueType input);
		
		/// \return The number of elements in the list
		unsigned int Size(void) const;
		
		/// Clear the list		
		void Clear(bool dontDelete = false);
		
		/// Frees over allocated members, to use the minimum memory necessary
		/// \attention 
		/// This is a slow operation		
		void Compress(void);
		
	private:
		/// An array of user values
		ValueType* m_listArray;
		
		/// Number of elements in the list 		
		unsigned int m_listSize;
		
		/// Size of \a array 		
		unsigned int m_allocSize;
	};

	template <class ValueType>
		CArrayList<ValueType>::CArrayList()
			: m_listArray(NULL)
			, m_listSize(0)
			, m_allocSize(0)
	{
	}

	template <class ValueType>
		CArrayList<ValueType>::~CArrayList()
	{
		if(m_allocSize > 0) {
			delete [] m_listArray;
			m_listArray = NULL;
			m_listSize = 0;
			m_allocSize = 0;
		}
	}


	template <class ValueType>
		CArrayList<ValueType>::CArrayList(const CArrayList& orig)
	{
		if(0 == orig.m_listSize)
		{
			m_listSize = 0;
			m_allocSize = 0;
		}
		else
		{
			m_listArray = new ValueType[orig.m_listSize];

			// Don't call constructors, assignment operators, etc.
			memcpy(m_listArray, orig.m_listArray, orig.m_listSize * sizeof(ValueType));

			m_listSize = m_allocSize = orig.m_listSize;
		}
	}

	template <class ValueType>
		CArrayList<ValueType>& CArrayList<ValueType>::operator= (const CArrayList& right)
	{
		if(&right != this)
		{
			Clear();

			if (0 == right.m_listSize)
			{
				m_listSize = 0;
				m_allocSize = 0;
			}
			else
			{
				m_listArray = new ValueType[right.m_listSize];

				// Don't call constructors, assignment operators, etc.
				memcpy(m_listArray, right.m_listArray, right.m_listSize * sizeof(ValueType));

				m_listSize = m_allocSize = right.m_listSize;
			}
		}
		return *this;
	}


	template <class ValueType>
		inline ValueType& CArrayList<ValueType>::operator[] (const unsigned int index) const
	{
#ifdef _DEBUG
		assert(index < m_listSize);
#endif
		return m_listArray[index];
	}

	template <class ValueType>
		void CArrayList<ValueType>::Insert(const ValueType input, const unsigned int index)
	{
#ifdef _DEBUG
		assert(index <= m_listSize);
#endif

		// Reallocate list if necessary
		if(m_listSize == m_allocSize)
		{
			if(0 == m_allocSize) {
				m_allocSize = 16;
			} else {
				m_allocSize *= 2;
			}

			ValueType * newArray = new ValueType[m_allocSize];

			// Don't call constructors, assignment operators, etc.
			memcpy(newArray, m_listArray, m_listSize * sizeof(ValueType));

			// set old array to point to the newly allocated and twice as large array
			delete[] m_listArray;

			m_listArray = newArray;
		}

		// Move the elements in the list to make room
		// Don't call constructors, assignment operators, etc.
		memmove(m_listArray+index+1, m_listArray+index, (m_listSize - index) * sizeof(ValueType));

		// Insert the new item at the correct spot
		m_listArray[index] = input;

		++m_listSize;
	}


	template <class ValueType>
		void CArrayList<ValueType>::Insert(const ValueType input)
	{
		// Reallocate list if necessary
		if(m_listSize == m_allocSize)
		{
			if(0 == m_allocSize) {
				m_allocSize = 16;
			} else {
				m_allocSize *= 2;
			}

			ValueType * newArray = new ValueType[m_allocSize];

			// Don't call constructors, assignment operators, etc.
			memcpy(newArray, m_listArray, m_listSize * sizeof(ValueType));

			// set old array to point to the newly allocated and twice as large array
			delete[] m_listArray;

			m_listArray = newArray;
		}

		// Insert the new item at the correct spot
		m_listArray[m_listSize] = input;

		++m_listSize;
	}

	template <class ValueType>
		inline void CArrayList<ValueType>::Replace(const ValueType input, const ValueType filler, const unsigned int index)
	{
		if(m_listSize > 0 && index < m_listSize)
		{
			m_listArray[index] = input;
		}
		else
		{
			if(index >= m_allocSize)
			{	
				m_allocSize = index + 1;

				ValueType * newArray = new ValueType[m_allocSize];

				// Don't call constructors, assignment operators, etc.
				memcpy(newArray, m_listArray, m_listSize * sizeof(ValueType));

				// set old array to point to the newly allocated array
				delete[] m_listArray;

				m_listArray = newArray;
			}

			// Fill in holes with filler
			while(m_listSize < index) {
				m_listArray[m_listSize++] = filler;
			}

			// Fill in the last element with the new item
			m_listArray[m_listSize++] = input;

#ifdef _DEBUG
			assert(m_listSize == index + 1);
#endif
		}
	}

	template <class ValueType>
		inline void CArrayList<ValueType>::Replace(const ValueType input)
	{
		if(m_listSize > 0) {
			m_listArray[m_listSize - 1] = input;
		}
	}

	template <class ValueType>
		void CArrayList<ValueType>::RemoveAtIndex(const unsigned int index)
	{
#ifdef _DEBUG
		assert(index < m_listSize);
#endif

		if(index < m_listSize)
		{
			// Compress the array
			memmove(m_listArray+index, m_listArray+index+1, (m_listSize-1-index) * sizeof(ValueType));

			Del();
		}
	}

	template <class ValueType>
		inline void CArrayList<ValueType>::Del(const unsigned num)
	{
		// Delete the last elements on the list.  No compression needed
#ifdef _DEBUG
		assert(m_listSize >= num);
#endif
		m_listSize -= num;
	}

	template <class ValueType>
		unsigned int CArrayList<ValueType>::GetIndexOf(const ValueType input)
	{
		for(unsigned int i = 0; i < m_listSize; ++i) {
			if(m_listArray[i] == input) {
				return i;
			}
		}
		return ARRAYLIST_MAX_INDEX;
	}

	template <class ValueType>
		inline unsigned int CArrayList<ValueType>::Size(void) const
	{
		return m_listSize;
	}

	template <class ValueType>
		void CArrayList<ValueType>::Clear(bool dontDelete/* = false*/)
	{
		if(0 == m_allocSize) {
			return;
		}

		if(m_allocSize > 512 && !dontDelete)
		{
			delete [] m_listArray;
			m_listArray = NULL;
			m_allocSize = 0;	
		}
		m_listSize = 0;
	}

	template <class ValueType>
		void CArrayList<ValueType>::Compress(void)
	{
		if(0 == m_allocSize) {
			return;
		}

		ValueType * newArray = new ValueType [m_allocSize];

		// Don't call constructors, assignment operators, etc.
		memcpy(newArray, m_listArray, m_listSize * sizeof(ValueType));

		// set old array to point to the newly allocated array
		delete[] m_listArray;

		m_listArray = newArray;
	}
	
} // End namespace

#endif // ARRAYLIST_H
