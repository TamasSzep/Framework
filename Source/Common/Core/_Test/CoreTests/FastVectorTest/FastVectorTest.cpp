// FastVectorTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/DataStructures/SimpleTypeUnorderedVector.hpp>
#include <Core/DataStructures/SimpleTypeUnorderedVectorWithInvalidElements.hpp>

#include <map>
#include <set>
#include <vector>
#include <chrono>
#include <cassert>

enum class Operation
{
	Add, Remove, Clear, Iterate,
	COUNT
};

const size_t c_CountSimulationSteps = 100000;
const float c_OperationWeights[] = { 100, 10, 0, 100 };

float GetTotalOperationWeight()
{
	float sum = 0.0f;
	for (size_t i = 0; i < static_cast<const size_t>(Operation::COUNT); i++)
	{
		sum += c_OperationWeights[i];
	}
	return sum;
}

Operation GetRandomOperation(float totalOperationWeight)
{
	float value = (rand() / static_cast<float>(RAND_MAX)) * totalOperationWeight;
	float sum = 0.0f;
	for (size_t i = 0; i < static_cast<const size_t>(Operation::COUNT); i++)
	{
		sum += c_OperationWeights[i];
		if (sum >= value)
		{
			return static_cast<Operation>(i);
		}
	}

	assert(false);
	return Operation::Add;
}

int GetRandomInteger()
{
	int value;
	do
	{
		value = rand() + (rand() << 15) + (rand() << 30);
	} while (value == 0xfffffffe || value == 0xffffffff);
	return value;
}

struct OperationData
{
	Operation Operation;

	typedef union
	{
		int Int32Data;
		size_t UInt64Data;
	} DataType;

	DataType Data1;
	DataType Data2;
};

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

int main()
{
	std::map<size_t, size_t> randomKeyMap;

	float totalOperationWeight = GetTotalOperationWeight();

	// Creating operation vector.
	std::vector<OperationData> operationDatas;
	Core::SimpleTypeUnorderedVector<int> fastCreateVector;
	for (size_t i = 0; i < c_CountSimulationSteps; i++)
	{
		Operation operation = GetRandomOperation(totalOperationWeight);

		operationDatas.push_back(OperationData());
		auto& opData = operationDatas.back();
		opData.Operation = operation;

		switch (operation)
		{
		case Operation::Add:
		{
			int value = GetRandomInteger();
			size_t key = fastCreateVector.Add(value);
			randomKeyMap.insert({ GetRandomInteger(), key });
			opData.Data1.UInt64Data = key;
			opData.Data2.Int32Data = value;
			break;
		}
		case Operation::Remove:
		{
			if (fastCreateVector.GetSize() > 0)
			{
				auto keyIt = randomKeyMap.begin();
				size_t key = keyIt->second;
				randomKeyMap.erase(keyIt);
				fastCreateVector.Remove(key);
				opData.Data1.UInt64Data = key;
			}
			else
			{
				operationDatas.pop_back();
			}
			break;
		}
		case Operation::Clear:
		{
			fastCreateVector.Clear();
			randomKeyMap.clear();
			break;
		}
		case Operation::Iterate:
		{
			break;
		}
		}
	}

	// Measuring fastest vector speed.
	Core::SimpleTypeUnorderedVectorWithInvalidElements<int, 0xfffffffe, 0xffffffff, unsigned> fastestVector;
	{
		Start();

		for (size_t i = 0; i < operationDatas.size(); i++)
		{
			auto& opData = operationDatas[i];
			switch (opData.Operation)
			{
			case Operation::Add:
			{
				fastestVector.Add(opData.Data2.Int32Data);
				break;
			}
			case Operation::Remove:
			{
				fastestVector.Remove(static_cast<unsigned>(opData.Data1.UInt64Data));
				break;
			}
			case Operation::Clear:
			{
				fastestVector.Clear();
				break;
			}
			case Operation::Iterate:
			{
				auto end = fastestVector.GetEndIterator();
				for (auto it = fastestVector.GetBeginIterator(); it != end; ++it) {}
				break;
			}
			}
		}

		Stop();
	}

	// Measuring fast vector speed.
	Core::SimpleTypeUnorderedVector<int, unsigned> fastVector;
	{
		Start();

		for (size_t i = 0; i < operationDatas.size(); i++)
		{
			auto& opData = operationDatas[i];
			switch (opData.Operation)
			{
			case Operation::Add:
			{
				fastVector.Add(opData.Data2.Int32Data);
				break;
			}
			case Operation::Remove:
			{
				fastVector.Remove(static_cast<unsigned>(opData.Data1.UInt64Data));
				break;
			}
			case Operation::Clear:
			{
				fastVector.Clear();
				break;
			}
			case Operation::Iterate:
			{
				auto end = fastVector.GetEndIterator();
				for (auto it = fastVector.GetBeginIterator(); it != end; ++it) {}
				break;
			}
			}
		}

		Stop();
	}

	// Measuring fast map speed.
	Core::SimpleTypeMap<size_t, int> fastMap;
	{
		Start();

		for (size_t i = 0; i < operationDatas.size(); i++)
		{
			auto& opData = operationDatas[i];
			switch (opData.Operation)
			{
			case Operation::Add:
			{
				fastMap.Add(opData.Data1.UInt64Data, opData.Data2.Int32Data);
				break;
			}
			case Operation::Remove:
			{
				fastMap.Remove(opData.Data1.UInt64Data);
				break;
			}
			case Operation::Clear:
			{
				fastMap.Clear();
				break;
			}
			case Operation::Iterate:
			{
				auto end = fastMap.GetDataEndIterator();
				for (auto it = fastMap.GetDataBeginIterator(); it != end; ++it) {}
			}
			}
		}

		Stop();
	}

	// Measuring map speed.
	std::map<size_t, int> map;
	{
		Start();

		for (size_t i = 0; i < operationDatas.size(); i++)
		{
			auto& opData = operationDatas[i];
			switch (opData.Operation)
			{
			case Operation::Add:
			{
				map.insert({ opData.Data1.UInt64Data, opData.Data2.Int32Data });
				break;
			}
			case Operation::Remove:
			{
				map.erase(opData.Data1.UInt64Data);
				break;
			}
			case Operation::Clear:
			{
				map.clear();
				break;
			}
			case Operation::Iterate:
			{
				for (auto& it : map) {}
				break;
			}
			}
		}

		Stop();
	}

	// Checking correctness.
	{
		std::multiset<int> checkSet1, checkSet2, checkSet3, checkSet4;
		{
			auto it = fastestVector.GetBeginIterator();
			auto end = fastestVector.GetEndIterator();
			for (; it != end; ++it)
			{
				checkSet1.insert(*it);
			}
		}
		{
			auto it = fastVector.GetBeginIterator();
			auto end = fastVector.GetEndIterator();
			for (; it != end; ++it)
			{
				checkSet2.insert(*it);
			}
		}
		{
			auto it = fastMap.GetDataBeginIterator();
			auto end = fastMap.GetDataEndIterator();
			for (; it != end; ++it)
			{
				checkSet3.insert(*it);
			}
		}
		for (auto& it : map)
		{
			checkSet4.insert(it.second);
		}

		if (checkSet1 != checkSet4)
		{
			printf("Fastest vector error.\n");
		}
		else if (checkSet2 != checkSet4)
		{
			printf("Fast vector error.\n");
		}
		else if (checkSet3 != checkSet4)
		{
			printf("Fast map error.\n");
		}
		else
		{
			printf("Correct!\n");
		}
	}

    return 0;
}

