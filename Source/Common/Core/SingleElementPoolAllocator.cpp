// Core/SingleElementPoolAllocator.cpp

#include <Core/SingleElementPoolAllocator.hpp>

Core::Detail::_SingleElementPoolAllocatorBase::CacheType Core::Detail::_SingleElementPoolAllocatorBase::m_ImplCache;
std::mutex Core::Detail::_SingleElementPoolAllocatorBase::m_Mutex;
