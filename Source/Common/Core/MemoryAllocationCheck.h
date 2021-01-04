// MemoryAllocationCheck.h

#ifndef _CORE_MEMORYALLOCATIONCHECK_H_
#define _CORE_MEMORYALLOCATIONCHECK_H_

#ifdef _DEBUG

#include <exception>
#include <new>
#include <cstdlib>

namespace Core
{
	class MemoryAllocationStatistics
	{
		size_t m_CountAllocation;

	public:

		MemoryAllocationStatistics()
			: m_CountAllocation(0)
		{
		}

		void Reset()
		{
			m_CountAllocation = 0;
		}

		void OnMemoryAllocation()
		{
			m_CountAllocation++;
		}

		size_t GetCountAllocations() const
		{
			return m_CountAllocation;
		}

		static MemoryAllocationStatistics& GetInstance()
		{
			static MemoryAllocationStatistics instance;
			return instance;
		}
	};
}

inline void* operator new (size_t size)
{
	Core::MemoryAllocationStatistics::GetInstance().OnMemoryAllocation();

	void* p = malloc(size); 
	if (p == nullptr)
	{
		throw std::bad_alloc();
	}
	return p;
}

#endif

#endif