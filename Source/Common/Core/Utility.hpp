// Core/Utility.hpp

#pragma once

#include <cassert>
#include <cstdint>

namespace Core
{
	template <typename T>
	inline bool IsPowerOfTwo(T x)
	{
		return ((x > 0) && (x & (x - 1)) == 0);
	}

	template <typename T>
	inline T AlignSize(T size, T alignment)
	{
		assert(IsPowerOfTwo(alignment));
		T alignMask = alignment - 1;
		return ((size + alignMask) & ~alignMask);
	}

	// For details see: https://graphics.stanford.edu/~seander/bithacks.html
	inline uint32_t GetNext2Power(uint32_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
}
