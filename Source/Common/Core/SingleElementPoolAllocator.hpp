// Core/SingleElementPoolAllocator.hpp

#pragma once

#define EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY 0

#include <Core/DataStructures/Pool.hpp>
#include <Core/AlignedAllocator.hpp>
#include <Core/Platform.h>

#include <cassert>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <typeindex>

namespace Core
{
	namespace Detail
	{
		class _SingleElementPoolAllocatorImplBase
		{
		public:
			virtual ~_SingleElementPoolAllocatorImplBase() {}
		};

		template <typename T>
		class _SingleElementPoolAllocatorImpl : public _SingleElementPoolAllocatorImplBase
		{
			struct PlaceHolderType
			{
				uint8_t data[sizeof(T)];
			};
			static_assert(sizeof(PlaceHolderType) == sizeof(T), "Type size error.");

			Core::ResourcePool<PlaceHolderType, unsigned> m_Pool;

		public:

#if EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY
			std::set<T*> m_AllocatedPointers;
#endif

			~_SingleElementPoolAllocatorImpl() override
			{
			}

			T* Request()
			{
				auto p = reinterpret_cast<T*>(m_Pool.RequestUnconstructed());

#if EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY
				assert(m_AllocatedPointers.find(p) == m_AllocatedPointers.end());
				m_AllocatedPointers.insert(p);
#endif
				return p;
			}

			void Release(T* p)
			{
#if EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY
				assert(m_AllocatedPointers.find(p) != m_AllocatedPointers.end());
				m_AllocatedPointers.erase(p);
#endif

				m_Pool.ReleaseNoDestruction(reinterpret_cast<PlaceHolderType*>(p));
			}

#if EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY
			void CheckContainment(T* p)
			{
				assert(m_AllocatedPointers.find(p) != m_AllocatedPointers.end());
			}
#endif
		};

		class _SingleElementPoolAllocatorBase
		{
		protected:
			using CacheType = std::map<std::type_index, std::unique_ptr<_SingleElementPoolAllocatorImplBase>>;
			static CacheType m_ImplCache;
			static std::mutex m_Mutex;
		};
	}

	template <typename T>
	class SingleElementPoolAllocator : public Detail::_SingleElementPoolAllocatorBase
	{
		Detail::_SingleElementPoolAllocatorImpl<T>* m_Impl;

		friend class SingleElementPoolAllocator;

		void InitializeImpl()
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			auto typeIndex = std::type_index(typeid(T));

			auto iIt = m_ImplCache.find(typeIndex);
			if (iIt == m_ImplCache.end())
			{
				iIt = m_ImplCache.insert(
					std::make_pair(typeIndex, std::make_unique<Detail::_SingleElementPoolAllocatorImpl<T>>())).first;
			}
			m_Impl = static_cast<Detail::_SingleElementPoolAllocatorImpl<T>*>(iIt->second.get());
		}

	public:
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		template <class U> struct rebind { typedef SingleElementPoolAllocator<U> other; };

		SingleElementPoolAllocator() CORE_NOEXCEPT
		{
			InitializeImpl();
		}

		~SingleElementPoolAllocator()
		{
		}

		template <class U>
		SingleElementPoolAllocator(const SingleElementPoolAllocator<U>&) CORE_NOEXCEPT
		{
			InitializeImpl();
			// Note that the parameter allocator might not be empty. That's not a problem, because we don't need to
			// copy any objects here.
		}

		size_type max_size() const
		{
			// This is not according to the standard, which requires to return the value n here, for which
			// allocate(n, 0) will succeed. This is 1, but STL's MSVC implementation uses the max_size to
			// find out whether new nodes can be created one by one.
			return (std::numeric_limits<size_type>::max() - (size_type)Core::Alignment::Normal) / sizeof(T);
		}

		const_pointer address(const_reference x) const CORE_NOEXCEPT
		{
			return std::addressof(x);
		}

		pointer allocate(size_type n, std::allocator_traits<std::allocator<void>>::const_pointer = 0)
		{
			if (n > 1) throw std::bad_alloc();

			return m_Impl->Request();
		}

		void deallocate(pointer p, size_type n)
		{
			if (n > 1) throw std::bad_alloc();

			m_Impl->Release(p);
		}

		template <class... Args>
		void construct(pointer p, Args&& ... args)
		{
#if EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY
			m_Impl->CheckContainment(p);
#endif

			::new(reinterpret_cast<void*>(p)) T(std::forward<Args>(args)...);
		}

		void destroy(pointer p)
		{
#if EXAMINE_SINGLE_ELEMENT_POOL_ALLOCATOR_CONSISTENCY
			m_Impl->CheckContainment(p);
#endif

			p->~T();
		}
	};

	template <typename TKey, typename TPredicate = typename std::less<TKey>>
	using FastStdSet = typename std::set<TKey, TPredicate,
		SingleElementPoolAllocator<TKey>>;

	template <typename TKey, typename TData, typename TPredicate = typename std::less<TKey>>
	using FastStdMap = typename std::map<TKey, TData, TPredicate,
		SingleElementPoolAllocator<std::pair<const TKey, TData>>>;

	template <typename TKey, typename TData, typename TPredicate = typename std::less<TKey>>
	using FastStdMultiMap = typename std::multimap<TKey, TData, TPredicate,
		SingleElementPoolAllocator<std::pair<const TKey, TData>>>;
}