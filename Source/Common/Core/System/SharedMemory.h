// Core/System/SharedMemory.h

#ifndef _CORE_SHARED_MEMORY_H_
#define _CORE_SHARED_MEMORY_H_

#include <memory>

namespace Core
{
	namespace detail
	{
		struct SM_Data;
	}

	enum class SharedMemoryHandlingRole : unsigned
	{
		Create, Use
	};

	enum class SharedMemoryAccessMode
	{
		ReadOnly, ReadWrite
	};

	class SharedMemory
	{
		std::unique_ptr<detail::SM_Data> m_Data;

	public:

		SharedMemory();
		~SharedMemory();

		void Initialize(SharedMemoryHandlingRole role,
			SharedMemoryAccessMode accessMode,
			const char* name,
			size_t size = 0);

		void Write(const unsigned char* buffer, size_t size);
		void Read(unsigned char* buffer, size_t size);

		void* GetPointer();
	};
}

#endif