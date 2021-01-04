// SortTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/Sort.hpp>

#include <chrono>
#include <vector>

#pragma warning( disable : 4503) // Disabling truncated iterator warning.

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

int x = 0;

int GetRandomInteger()
{
	return rand() + (rand() << 15) + (rand() << 30);
}

const size_t c_CountSimulations = 10000;
const size_t c_VectorSize = 16 * 1024;
const size_t c_DataSize = 8;
const unsigned c_CountThreads = 8;
const bool c_IsStressTest = true;

struct Data
{
	int Key;
	unsigned char DataVector[c_DataSize];
};

void GenerateData(std::vector<int>& keys, std::vector<Data>& datas, size_t vectorSize)
{
	for (size_t i = 0; i < vectorSize; i++)
	{
		int key = GetRandomInteger();
		keys.push_back(key);
		datas.emplace_back();
		auto& data = datas.back();
		data.Key = key;
		auto dataVector = data.DataVector;
		//for (size_t j = 0; j < c_DataSize; j++)
		//{
		//	dataVector[j] = static_cast<unsigned char>(rand());
		//}
	}
}

int main()
{
	bool isCorrect = true;

	printf("Data:\n\n");

	for (size_t i = 0; i < c_CountSimulations; i++)
	{
		size_t vectorSize = (c_IsStressTest ? i / c_CountThreads : c_VectorSize);
		unsigned countThreads = (c_IsStressTest ? (i % c_CountThreads) + 1 : c_CountThreads);

		Core::ParalelSorter<int> paralelSorter(countThreads);

		std::vector<int> keys;
		std::vector<Data> data;
		std::vector<Data> result1, result2;
		Core::SortTempVectorType<int> tempVector;

		// We reserve memory for the sort call, since we want to measure only the sort algorithm's speed without memory allocations.
		result1.reserve(vectorSize);
		result2.reserve(vectorSize);

		// Generating random data.
		GenerateData(keys, data, vectorSize);

		// Sorting data vector using the keys.
		{
			if (c_CountSimulations == 1)
			{
				Start();
			}

			Core::SortByKeys(keys, data, result1, std::less<>(), tempVector);

			if (c_CountSimulations == 1)
			{
				Stop();
				Start();
			}

			paralelSorter.SortByKeys(keys, data, result2, std::less<>());

			if (c_CountSimulations == 1)
			{
				Stop();
			}
		}

		// Checking correctness.
		{
			for (size_t j = 1; j < vectorSize; j++)
			{
				if (result1[j].Key < result1[j - 1].Key || result2[j].Key < result2[j - 1].Key)
				{
					isCorrect = false;
					break;
				}
			}

			printf("%d: [%d @ %d] ", static_cast<int>(i), static_cast<int>(vectorSize), countThreads);
			if (isCorrect)
			{
				printf("Current simulation is correct.\n");
			}
			else
			{
				printf("ERROR!\n");
			}

			if (!isCorrect)
			{
				break;
			}
		}
	}

	if (isCorrect)
	{
		printf("Correct!\n");
	}
	else
	{
		printf("ERROR!\n");
		return 0;
	}

	printf("\n\nKey:\n\n");

	for (size_t i = 0; i < c_CountSimulations; i++)
	{
		size_t vectorSize = (c_IsStressTest ? i / c_CountThreads : c_VectorSize);
		unsigned countThreads = (c_IsStressTest ? (i % c_CountThreads) + 1 : c_CountThreads);

		std::vector<int> ints;
		ints.reserve(vectorSize);
		for (unsigned i = 0; i < vectorSize; i++)
		{
			ints.push_back(GetRandomInteger());
		}
		auto intsCopy = ints;

		Core::ParalelSorter<int> keySorter(countThreads);

		if (c_CountSimulations == 1)
		{
			Start();
		}

		auto pResult = keySorter.SortKeys(intsCopy);

		if (c_CountSimulations == 1)
		{
			Stop();
			Start();
		}

		std::sort(ints.begin(), ints.end());

		if (c_CountSimulations == 1)
		{
			Stop();
		}

		isCorrect = (*pResult == ints);

		printf("%d: [%d @ %d] ", static_cast<int>(i), static_cast<int>(vectorSize), countThreads);
		if (isCorrect)
		{
			printf("Current simulation is correct.\n");
		}
		else
		{
			printf("ERROR!\n");
		}

		if (!isCorrect)
		{
			break;
		}
	}

	if (isCorrect)
	{
		printf("Correct!\n");
	}
	else
	{
		printf("ERROR!\n");
	}

    return 0;
}