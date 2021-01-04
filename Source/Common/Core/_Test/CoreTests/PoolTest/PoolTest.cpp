// PoolTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/DataStructures/Pool.hpp>

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

size_t c_PoolSize = 32 * 1024;
size_t c_CountRequests = 1 * 1024;
size_t c_SimulationCount = 10000;

int main()
{
	Core::SimplePool<int> pool1(c_PoolSize);
	Core::SimpleStackPool<int> pool2(c_PoolSize);

	Start();

	for (size_t i = 0; i < c_SimulationCount; i++)
	{
		for (size_t i = 0; i < c_CountRequests; i++)
		{
			pool1.RequestWithoutReconstruction();
		}
		pool1.ReleaseAll();
	}

	Stop();
	Start();

	for (size_t i = 0; i < c_SimulationCount; i++)
	{
		for (size_t i = 0; i < c_CountRequests; i++)
		{
			pool2.RequestWithoutReconstruction();
		}
		pool2.ReleaseAll();
	}

	Stop();


    return 0;
}

