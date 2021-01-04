// CollectionExtensions.hpp

#ifndef _CORE_COLLECTION_EXTENSIONS_H_
#define _CORE_COLLECTION_EXTENSIONS_H_

#include <set>
#include <map>
#include <queue>
#include <memory>
#include <algorithm>

namespace Core
{
	template <typename T>
	struct PointerComparator : public std::binary_function<T, T, bool>
	{
		bool operator()(const T* lhs, const T* rhs) const
		{
			return *lhs < *rhs;
		}
	};

	template <typename T>
	struct SharedPointerComparator : public std::binary_function<T, T, bool>
	{
		bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const
		{
			return *lhs.get() < *rhs.get();
		}
	};

	template <typename T>
	struct WeakPointerComparator : public std::binary_function<T, T, bool>
	{
		bool operator()(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs) const
		{
			return *lhs.lock().get() < *rhs.lock().get();
		}
	};

	template <typename T> using PointerSet = std::set<T*, PointerComparator<T>>;
	template <typename T> using SharedPointerSet = std::set<std::shared_ptr<T>, SharedPointerComparator<T>>;
	template <typename T> using WeakPointerSet = std::set<std::weak_ptr<T>, WeakPointerComparator<T>>;
	template <typename K, typename D> using PointerMap = std::map<K*, D, PointerComparator<K>>;
	template <typename K, typename D> using SharedPointerMap = std::map<std::shared_ptr<K>, D, SharedPointerComparator<K>>;

	template <typename K, typename D>
	bool ContainsData(const std::map<K, D>& m, const D& d)
	{
		for (auto& it : m)
		{
			if (it.second == d)
			{
				return true;
			}
		}
		return false;
	}

	template<typename Target, typename Source>
	std::unique_ptr<Target> Static_unique_ptr_cast(std::unique_ptr<Source>&& p)
	{
		auto d = static_cast<Target*>(p.release());
		return std::unique_ptr<Target>(d);
	}

	template<typename Target, typename Source>
	std::unique_ptr<Target> Dynamic_unique_ptr_cast(std::unique_ptr<Source>&& p)
	{
		if (Target* result = dynamic_cast<Target*>(p.get()))
		{
			p.release();
			return std::unique_ptr<Target>(result);
		}
		return std::unique_ptr<Target>(nullptr);
	}

	template <class T, class S, class C>
	S& Container(std::priority_queue<T, S, C>& q)
	{
		struct HackedQueue : private std::priority_queue<T, S, C>
		{
			static S& Container(std::priority_queue<T, S, C>& q)
			{
				return q.*&HackedQueue::c;
			}
		};
		return HackedQueue::Container(q);
	}
}

#endif