// Core/Debug.h

#ifndef _CORE_DEBUG_H_INCLUDED_
#define _CORE_DEBUG_H_INCLUDED_

#include <Core/Windows.h>

#include <cassert>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#ifdef _MSC_VER

namespace Core
{
	inline void WriteToOutput(const char* str)
	{
		OutputDebugStringA(str);
	}

	inline void WriteToOutput(const std::string& str)
	{
		OutputDebugStringA(str.c_str());
	}

	inline void Debug_CheckSingleThreaded()
	{
		static std::set<std::thread::id> threadIds;
		static std::mutex m;
		std::lock_guard<std::mutex> lock(m);
		threadIds.insert(std::this_thread::get_id());
		if (threadIds.size() > 1)
		{
			printf("ERROR: Debug_CheckSingleThreaded()\n");
			assert(false);
		}
	}
}

#endif

#endif