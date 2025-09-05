

#ifndef LOCK_FREE_NODE_H
#define LOCK_FREE_NODE_H

namespace thd {

	template<typename Ty>
	class CLockFreeNode {
	public:
		CLockFreeNode() : value(), pNext(NULL) {}

		CLockFreeNode(const Ty& v) : value(v), pNext(NULL) {}

		CLockFreeNode(const Ty&& v) : value(v), pNext(NULL) {}

		virtual ~CLockFreeNode() {}

		Ty& GetValue()
		{
			return value;
		}

		const Ty& GetValue() const
		{
			return value;
		}

	private:
		template<typename T> friend class CLockFreeQueue;

		Ty value;
		CLockFreeNode<Ty>* volatile pNext;
	};

}

#endif // LOCK_FREE_NODE_H
