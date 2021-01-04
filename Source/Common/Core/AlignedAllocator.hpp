// AlignedAllocator.hpp

#ifndef _CORE_ALLIGNEDALLOCATOR_HPP_INCLUDED_
#define _CORE_ALLIGNEDALLOCATOR_HPP_INCLUDED_

#include <Core/Platform.h>
#include <Core/Utility.hpp>

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <vector>

namespace Core
{
	enum class Alignment : size_t
	{
		Normal = sizeof(void*),
		_16Byte = 16,
		_32Byte = 32
	};

	namespace detail
	{
		inline void* allocate_aligned_memory(size_t align, size_t size)
		{
			assert(align >= sizeof(void*));
			assert(Core::IsPowerOfTwo(align));

			if (size == 0)
			{
				return nullptr;
			}

#ifdef IS_WINDOWS
			return _aligned_malloc(size, align);
#else
			return aligned_alloc(align, size);
#endif
		}

		inline void deallocate_aligned_memory(void *ptr) CORE_NOEXCEPT
		{
#ifdef IS_WINDOWS
			_aligned_free(ptr);
#else
			free(ptr);
#endif
		}
	}

	template <typename T, Alignment Align = Alignment::_32Byte>
	class AlignedAllocator;

	template <Alignment Align>
	class AlignedAllocator<void, Align>
	{
	public:

		typedef void*             pointer;
		typedef const void*       const_pointer;
		typedef void              value_type;

		template <class U> struct rebind { typedef AlignedAllocator<U, Align> other; };
	};

	template <typename T, Alignment Align>
	class AlignedAllocator
	{
	public:
		typedef T         value_type;
		typedef T*        pointer;
		typedef const T*  const_pointer;
		typedef T&        reference;
		typedef const T&  const_reference;
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;

		typedef std::true_type propagate_on_container_move_assignment;

		template <class U>
		struct rebind { typedef AlignedAllocator<U, Align> other; };

	public:

		AlignedAllocator() CORE_NOEXCEPT
		{
		}

		template <class U>
		AlignedAllocator(const AlignedAllocator<U, Align>&) CORE_NOEXCEPT
		{
		}

		size_type max_size() const CORE_NOEXCEPT
		{ 
			return (size_type(~0) - size_type(Align)) / sizeof(T);
		}

		pointer address(reference x) const CORE_NOEXCEPT
		{
			return std::addressof(x);
		}

		const_pointer address(const_reference x) const CORE_NOEXCEPT
		{
			return std::addressof(x);
		}

		pointer allocate(size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0)
		{
			const size_type alignment = static_cast<size_type>(Align);
			void* ptr = detail::allocate_aligned_memory(alignment, n * sizeof(T));
			if (ptr == nullptr)
			{
				throw std::bad_alloc();
			}

			return reinterpret_cast<pointer>(ptr);
		}

		void deallocate(pointer p, size_type) CORE_NOEXCEPT
		{
			return detail::deallocate_aligned_memory(p);
		}

		template <class U, class ...Args>
		void construct(U* p, Args&&... args)
		{
			::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
		}

		void destroy(pointer p)
		{
			p->~T();
		}

		template <typename T>
		void destroy(T* p)
		{
			p->~T();
		}
	};

	template <typename T, Alignment Align>
	class AlignedAllocator<const T, Align>
	{
	public:
		typedef T         value_type;
		typedef const T*  pointer;
		typedef const T*  const_pointer;
		typedef const T&  reference;
		typedef const T&  const_reference;
		typedef size_t    size_type;
		typedef ptrdiff_t difference_type;

		typedef std::true_type propagate_on_container_move_assignment;

		template <class U>
		struct rebind { typedef AlignedAllocator<U, Align> other; };

	public:
		AlignedAllocator() CORE_NOEXCEPT
		{
		}

		template <class U>
		AlignedAllocator(const AlignedAllocator<U, Align>&) CORE_NOEXCEPT
		{
		}

		size_type max_size() const CORE_NOEXCEPT
		{
			return (size_type(~0) - size_type(Align)) / sizeof(T);
		}

		const_pointer address(const_reference x) const CORE_NOEXCEPT
		{
			return std::addressof(x);
		}

		pointer allocate(size_type n, typename AlignedAllocator<void, Align>::const_pointer = 0)
		{
			const size_type alignment = static_cast<size_type>(Align);
			void* ptr = detail::allocate_aligned_memory(alignment, n * sizeof(T));
			if (ptr == nullptr) {
				throw std::bad_alloc();
			}

			return reinterpret_cast<pointer>(ptr);
		}

		void deallocate(pointer p, size_type) CORE_NOEXCEPT
		{
			return detail::deallocate_aligned_memory(p);
		}

		template <class U, class ...Args>
		void construct(U* p, Args&&... args)
		{
			::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
		}

		void destroy(pointer p)
		{
			p->~T();
		}
	};

	template <typename T, Alignment TAlign, typename U, Alignment UAlign>
	inline bool operator== (const AlignedAllocator<T, TAlign>&, const AlignedAllocator<U, UAlign>&) CORE_NOEXCEPT
	{
		return TAlign == UAlign;
	}

	template <typename T, Alignment TAlign, typename U, Alignment UAlign>
	inline bool operator!= (const AlignedAllocator<T, TAlign>&, const AlignedAllocator<U, UAlign>&) CORE_NOEXCEPT
	{
		return TAlign != UAlign;
	}

	template <typename T, Alignment Align>
	using AlignedVector = std::vector<T, AlignedAllocator<T, Align>>;
}


#endif