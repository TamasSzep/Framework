// AlgorithmExtensionTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/AlgorithmExtensions.hpp>

#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>

std::chrono::high_resolution_clock::time_point s_StartTime;

void Start()
{
	s_StartTime = std::chrono::high_resolution_clock::now();
}

void Stop()
{
	auto endTime = std::chrono::high_resolution_clock::now();
	printf("%lld us\n", std::chrono::duration_cast<std::chrono::microseconds>(endTime - s_StartTime).count());
}

size_t c_CountElements = 1024 * 1024;
size_t c_CountSimulations = 1;

int main()
{
	for (size_t i = 0; i < c_CountSimulations; i++)
	{
		std::vector<int> values1, values2, values3, values4;
		values1.reserve(c_CountElements);
		values2.reserve(c_CountElements);
		for (size_t i = 0; i < c_CountElements; i++)
		{
			values1.push_back(rand());
			values2.push_back(rand());
		}
		values3 = values1;
		values4 = values2;

		if (c_CountSimulations == 1)
		{
			Start();
		}

		size_t size1 = Core::GetIntersectionSize(
			values1.data(), values1.data() + values1.size(),
			values2.data(), values2.data() + values2.size());

		if (c_CountSimulations == 1)
		{
			Stop();
			Start();
		}

		std::sort(values3.begin(), values3.end());
		std::sort(values4.begin(), values4.end());

		std::vector<int> v_intersection;
		std::set_intersection(values3.begin(), values3.end(),
			values4.begin(), values4.end(),
			std::back_inserter(v_intersection));
		size_t size2 = v_intersection.size();

		if (c_CountSimulations == 1)
		{
			Stop();
		}

		if (size1 != size2)
		{
			printf("ERROR!\n");
			
			return 0;
		}
	}

	printf("Correct.\n");

    return 0;
}

