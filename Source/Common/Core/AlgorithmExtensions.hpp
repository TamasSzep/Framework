// Core/AlgorithmExtensions.hpp

#ifndef _CORE_ALGORITHMEXTENSIONS_HPP_
#define _CORE_ALGORITHMEXTENSIONS_HPP_

#include <vector>
#include <cassert>
#include <limits>

namespace Core
{
	template <typename T>
	inline T GetMinimumSimple(const T* start, const T* end)
	{
		T min = std::numeric_limits<T>::max();
		for (; start != end; ++start)
		{
			auto value = *start;
			if (value < min) min = value;
		}
		return min;
	}

	template <typename T>
	inline T GetMaximumSimple(const T* start, const T* end)
	{
		T max = std::numeric_limits<T>::lowest();
		for (; start != end; ++start)
		{
			auto value = *start;
			if (value > max) max = value;
		}
		return max;
	}

	template <typename T>
	inline size_t GetIntersectionSize(T* start1, T* end1, T* start2, T* end2)
	{
		if (start1 == end1 || start2 == end2)
		{
			return 0;
		}

		std::sort(start1, end1);
		std::sort(start2, end2);

		size_t countCommon = 0;

		T value1 = *start1;
		T value2 = *start2;

		while (true)
		{
			if (value1 < value2)
			{
				if (++start1 == end1)
				{
					return countCommon;
				}
				value1 = *start1;
			}
			else if (value2 < value1)
			{
				if (++start2 == end2)
				{
					return countCommon;
				}
				value2 = *start2;
			}
			else
			{
				++countCommon;

				if (++start1 == end1 || ++start2 == end2)
				{
					return countCommon;
				}

				value1 = *start1;
				value2 = *start2;
			}
		}

		return countCommon;
	}

	template <typename T>
	inline size_t GetIntersectionSize(std::vector<T>& v1, std::vector<T>& v2)
	{
		auto d1 = v1.data();
		auto d2 = v2.data();
		return GetIntersectionSize(d1, d1 + v1.size(), d2, d2 + v2.size());
	}
}
#endif