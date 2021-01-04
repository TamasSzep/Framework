// Core/System/SharedMemory.cpp

#include <Core/System/SharedMemory.h>

#include <Core/Platform.h>
#include <Core/System/Filesystem.h>
#include <Core/Windows.h>

#ifdef IS_WINDOWS
#include <filesystem>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <cstdint>
#include <cassert>
#include <string>
#include <cstring>
#include <sstream>

namespace Core
{
	namespace detail
	{
		// Aligning to 256 byte.
		struct SM_Header
		{
			uint64_t TotalSize;
			uint64_t HeaderSize;
			uint8_t Padding[240];
		};

		struct SM_Data
		{
			SharedMemoryHandlingRole Role;
			SharedMemoryAccessMode AccessMode;
			std::string Name;
			uint64_t Size;
			bool IsOpened;
			void* MappedPtr;

#ifdef IS_WINDOWS
			HANDLE		Handle;
#else
			int			FileDesc;
#endif
		};

		uint64_t _GetTotalSize(uint64_t userSize) { return userSize + sizeof(SM_Header); }
		
		std::string _GetFullName(const char* name)
		{
#ifdef IS_WINDOWS
			return std::string("Local\\") + name;
#else
			return std::string("/") + name;
#endif
		}

#ifdef IS_WINDOWS
#else
		void InitializeFileMapping(SM_Data* pData, SharedMemoryAccessMode accessMode,
			bool isCreator)
		{
			auto fullName = _GetFullName(pData->Name.c_str());
			auto oFlag =
				(accessMode == SharedMemoryAccessMode::ReadOnly ? O_RDONLY : O_RDWR)
				| (isCreator ? (O_CREAT | O_EXCL) : 0);
			auto mode = (accessMode == SharedMemoryAccessMode::ReadOnly
				? S_IRUSR
				: S_IRUSR | S_IWUSR);
			pData->FileDesc = shm_open(fullName.c_str(), oFlag, mode);
			if (pData->FileDesc < 0)
			{
				throw std::runtime_error("Failed to create file mapping.");
			}
		}

		void Unlink(SM_Data* pData)
		{
			shm_unlink(_GetFullName(pData->Name.c_str()).c_str());
		}
#endif

		void _CreateFileMapping(SM_Data* pData, SharedMemoryAccessMode accessMode)
		{	
			auto totalSize = _GetTotalSize(pData->Size);

#ifdef IS_WINDOWS
			auto fullName = _GetFullName(pData->Name.c_str());
			auto tsHigh = (uint32_t)(totalSize >> 32);
			auto tsLow = (uint32_t)(totalSize & 0xffffffff);
			pData->Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr,
				(accessMode == SharedMemoryAccessMode::ReadOnly
					? PAGE_READONLY
					: PAGE_READWRITE),
				tsHigh, tsLow, fullName.c_str());
			if (pData->Handle == nullptr)
			{
				std::stringstream ss;
				ss << "Failed to create file mapping: " << GetLastError();
				throw std::runtime_error(ss.str());
			}
#else
			InitializeFileMapping(pData, accessMode, true);
			if (ftruncate(pData->FileDesc, totalSize) < 0)
			{
				throw std::runtime_error("Failed to truncate file mapping.");
			}
#endif
		}

		void _OpenFileMapping(SM_Data* pData, SharedMemoryAccessMode accessMode)
		{
#ifdef IS_WINDOWS
			pData->Handle = OpenFileMappingA((accessMode == SharedMemoryAccessMode::ReadOnly
				? FILE_MAP_READ
				: FILE_MAP_ALL_ACCESS), FALSE, _GetFullName(pData->Name.c_str()).c_str());
			if (pData->Handle == nullptr)
			{
				std::stringstream ss;
				ss << "Failed to create file mapping: " << GetLastError();
				throw std::runtime_error(ss.str());
			}
#else
			InitializeFileMapping(pData, accessMode, false);
#endif
		}

		void _SetFileMappingView(SM_Data* pData, SharedMemoryAccessMode accessMode)
		{
#ifdef IS_WINDOWS
			if (pData->Size == 0)
			{
				auto tempPtr = MapViewOfFile(pData->Handle, FILE_MAP_READ, 0, 0,
					sizeof(SM_Header));
				if (tempPtr == nullptr)
				{
					CloseHandle(pData->Handle);
					throw std::runtime_error(
						"Failed to map file view for reading the shared memory's size.");
				}
				SM_Header header;
				memcpy(&header, tempPtr, sizeof(SM_Header));
				UnmapViewOfFile(tempPtr);
				pData->Size = header.TotalSize - header.HeaderSize;
			}
			pData->MappedPtr = MapViewOfFile(pData->Handle,
				(accessMode == SharedMemoryAccessMode::ReadOnly
					? FILE_MAP_READ
					: FILE_MAP_ALL_ACCESS),
				0, 0, _GetTotalSize(pData->Size));
			if (pData->MappedPtr == nullptr)
			{
				CloseHandle(pData->Handle);
				std::stringstream ss;
				ss << "Failed to map file view: " << GetLastError();
				throw std::runtime_error(ss.str());
			}
#else
			if (pData->Size == 0)
			{
				auto tempPtr = mmap(nullptr, sizeof(SM_Header), PROT_READ, MAP_SHARED,
					pData->FileDesc, 0);
				if (tempPtr == MAP_FAILED)
				{
					Unlink(pData);
					throw std::runtime_error(
						"Failed to map file view for reading the shared memory's size.");
				}
				SM_Header header;
				memcpy(&header, tempPtr, sizeof(SM_Header));
				munmap(tempPtr, sizeof(SM_Header));
				pData->Size = header.TotalSize - header.HeaderSize;
			}
			pData->MappedPtr = mmap(nullptr, sizeof(SM_Header),
				(accessMode == SharedMemoryAccessMode::ReadOnly
					? PROT_READ 
					: PROT_READ | PROT_WRITE), MAP_SHARED, pData->FileDesc, 0);
			if (pData->MappedPtr == MAP_FAILED)
			{
				Unlink(pData);
				throw std::runtime_error("Failed to map file view.");
			}
#endif
		}

		SM_Data* Open(const char* name, uint64_t size, SharedMemoryAccessMode accessMode)
		{
			auto pData = new SM_Data();
			pData->Role = SharedMemoryHandlingRole::Use;
			pData->AccessMode = accessMode;
			pData->Name = name;
			pData->Size = size;
			pData->IsOpened = false;
			_OpenFileMapping(pData, accessMode);
			_SetFileMappingView(pData, accessMode);
			pData->IsOpened = true;
			return pData;
		}

		SM_Data* Create(const char* name, uint64_t size, SharedMemoryAccessMode accessMode)
		{
			auto pData = new SM_Data();
			pData->Role = SharedMemoryHandlingRole::Create;
			pData->AccessMode = accessMode;
			pData->Name = name;
			pData->Size = size;
			pData->IsOpened = false;
			_CreateFileMapping(pData, accessMode);
			_SetFileMappingView(pData, accessMode);
			SM_Header header;
			header.TotalSize = _GetTotalSize(size);
			header.HeaderSize = sizeof(SM_Header);
			memcpy(pData->MappedPtr, &header, sizeof(SM_Header));
			pData->IsOpened = true;
			return pData;
		}

		void Close(SM_Data* pData)
		{
			if (pData && pData->IsOpened)
			{
#ifdef IS_WINDOWS
				UnmapViewOfFile(pData->MappedPtr);
				CloseHandle(pData->Handle);
#else
				munmap(pData->MappedPtr, _GetTotalSize(pData->Size));
				Unlink(pData);
#endif
				pData->IsOpened = false;
			}
		}

		void Remove(const char* name)
		{	
#ifdef IS_WINDOWS
			auto handle = OpenFileMappingA(PAGE_READWRITE, FALSE, _GetFullName(name).c_str());
			if (handle != nullptr)
			{
				char fileName[1024];
				GetFinalPathNameByHandleA(handle, fileName, _countof(fileName), VOLUME_NAME_NT);
				CloseHandle(handle);
				Core::RemoveFile(fileName);
			}
#else
			auto fullName = _GetFullName(name);
			auto fileDesc = shm_open(fullName.c_str(), O_RDONLY, S_IRUSR);
			if (fileDesc >= 0) shm_unlink(fullName.c_str());
#endif
		}

		void* GetPointer(SM_Data* pData)
		{
			assert(pData && pData->IsOpened);
			return reinterpret_cast<unsigned char*>(pData->MappedPtr) + sizeof(SM_Header);
		}
	}

	SharedMemory::SharedMemory()
	{
	}

	SharedMemory::~SharedMemory()
	{
		Core::detail::Close(m_Data.get());
	}

	void SharedMemory::Initialize(SharedMemoryHandlingRole role, SharedMemoryAccessMode accessMode,
		const char* name, size_t size)
	{
		if (role == SharedMemoryHandlingRole::Create)
		{
			Core::detail::Remove(name);
			m_Data.reset(Core::detail::Create(name, size, accessMode));
		}
		else
		{
			m_Data.reset(Core::detail::Open(name, size, accessMode));
		}
	}

	void SharedMemory::Write(const unsigned char* buffer, size_t size)
	{
		assert(m_Data->AccessMode == SharedMemoryAccessMode::ReadWrite);
		memcpy(GetPointer(), buffer, size);
	}

	void SharedMemory::Read(unsigned char* buffer, size_t size)
	{
		memcpy(buffer, GetPointer(), size);
	}

	void* SharedMemory::GetPointer()
	{
		return Core::detail::GetPointer(m_Data.get());
	}
}