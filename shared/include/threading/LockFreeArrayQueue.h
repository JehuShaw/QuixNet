#ifndef LOCKFREEARRAYQUEUE_H
#define LOCKFREEARRAYQUEUE_H

#include <cstddef>
#include "AtomicLock.h"


namespace thd {

	template <typename T>
	class CLockFreeArrayQueue
	{
	public:
		explicit CLockFreeArrayQueue(size_t capacity)
		{
//#if COMPILER == COMPILER_MICROSOFT
//			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
//#endif
			m_capacityMask = capacity - 1;
			for (size_t i = 1; i <= sizeof(void*) * 4; i <<= 1) {
				m_capacityMask |= m_capacityMask >> i;
			}
			m_capacity = m_capacityMask + 1;

			m_queue = (SNode*)new char[sizeof(SNode) * m_capacity];
			for (size_t i = 0; i < m_capacity; ++i)
			{
				atomic_xchg(&m_queue[i].tail, i);
				atomic_xchg(&m_queue[i].head, -1);
			}

			atomic_xchg(&m_tail, 0);
			atomic_xchg(&m_head, 0);
		}


		~CLockFreeArrayQueue()
		{
			for (size_t i = m_head; i != m_tail; ++i) {
				(&m_queue[i & m_capacityMask].data)->~T();
			}

			delete[](char*)m_queue;
		}

		size_t Capacity() const
		{
			return m_capacity;
		}

		size_t Size() const
		{
			uint32_t head = m_head;
			return m_tail - head;
		}

		bool TryPush(const T& data)
		{
			SNode* node;
			uint32_t tail = m_tail;
			do {
				node = &m_queue[tail & m_capacityMask];
				if (node->tail != tail) {
					return false;
				}
				if (atomic_cmpxchg(&m_tail, tail, tail + 1) == tail) {
					break;
				}
			} while (true);

			new (&node->data)T(data);
			atomic_xchg(&node->head, tail);
			return true;
		}

		void Push(const T& data)
		{
			for (int i = 0; !TryPush(data); ++i) {
				cpu_relax(i);
			}
		}

		bool Pop(T& result)
		{
			SNode* node;
			uint32_t head = m_head;
			do {
				node = &m_queue[head & m_capacityMask];
				if (node->head != head) {
					return false;
				}
				if (atomic_cmpxchg(&m_head, head, head + 1)) {
					break;
				}
			} while (true);
			result = node->data;
			(&node->data)->~T();
			atomic_xchg(&node->tail, head + m_capacity);
			return true;
		}

	private:
		struct SNode
		{
			T data;
			volatile uint32_t tail;
			volatile uint32_t head;
		};

	private:
		SNode* m_queue;
		size_t m_capacityMask;
		size_t m_capacity;
		volatile uint32_t m_tail;
		volatile uint32_t m_head;
	};
} // namespace thd

#endif /* LOCKFREEARRAYQUEUE_H */