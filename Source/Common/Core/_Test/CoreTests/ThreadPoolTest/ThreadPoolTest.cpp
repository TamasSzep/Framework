// ThreadPoolTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <TestCommon/MockObject.h>
#include <Core/System/ThreadPool.h>
//#include <Core/System/SimpleThreadPool.hpp>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

unsigned TestIndex = 0;
unsigned BlockIndex = 0;

template <typename FunctionType, typename... Args>
void TestFunction(Core::ThreadPool* threadPool,
	FunctionType&& function, Args&&... args)
{
	printf("\n%d\n\n", (TestIndex + 1));

	threadPool->GetThread(0).Execute(std::forward<FunctionType>(function),
		std::forward<Args>(args)...);
	threadPool->Join();

	TestIndex++;
}

template <typename FunctionType, typename ObjectType, typename... Args>
void TestFunction(Core::ThreadPool* threadPool, 
	FunctionType&& function, ObjectType* object, Args&&... args)
{
	printf("\n%d\n\n", (TestIndex + 1));

	threadPool->GetThread(0).Execute(std::forward<FunctionType>(function),
		object, std::forward<Args>(args)...);
	threadPool->Join();

	TestIndex++;
}

template <typename FunctionType, bool IsFunctionConst>
struct TestParameteredFunctionsWithConstRefA
{
};

template <typename FunctionType>
struct TestParameteredFunctionsWithConstRefA<FunctionType, false>
{
	static void Test(Core::ThreadPool* threadPool, FunctionType&& function) {};
};

template <typename FunctionType>
struct TestParameteredFunctionsWithConstRefA<FunctionType, true>
{
public:

	static void Test(Core::ThreadPool* threadPool, FunctionType&& function)
	{
		TestCommon::MockObject a1("a1");
		TestCommon::MockObject b1("b1");
		auto& a1R = a1; const auto& a1CR = a1;
		auto& b1R = b1; const auto& b1CR = b1;

		TestFunction(threadPool, std::forward<FunctionType>(function),
			&a1CR, b1);
		TestFunction(threadPool, std::forward<FunctionType>(function),
			&a1CR, b1R);
		TestFunction(threadPool, std::forward<FunctionType>(function),
			&a1CR, b1CR);
		TestFunction(threadPool, std::forward<FunctionType>(function),
			&a1CR, std::move(b1));
	}
};

template <typename FunctionType, bool IsFunctionConst>
struct TestParameteredFunctionsWithConstRefB
{
};

template <typename FunctionType>
struct TestParameteredFunctionsWithConstRefB<FunctionType, false>
{
	static void Test(Core::ThreadPool* threadPool, FunctionType&& function) {};
};

template <typename FunctionType>
struct TestParameteredFunctionsWithConstRefB<FunctionType, true>
{
public:

	static void Test(Core::ThreadPool* threadPool, FunctionType&& function)
	{
		TestCommon::MockObject a1("a1");
		TestCommon::MockObject b1("b1");
		auto& a1R = a1; const auto& a1CR = a1;
		const auto& b1CR = b1;

		TestFunction(threadPool, std::forward<FunctionType>(function),
			&a1, b1CR);
		TestFunction(threadPool, std::forward<FunctionType>(function),
			&a1R, b1CR);
		TestFunction(threadPool, std::forward<FunctionType>(function),
			&std::move(a1), b1CR);
	}
};

template <bool IsFunctionConst, bool IsParameterConst, typename FunctionType>
void TestParameteredFunctions(Core::ThreadPool* threadPool, FunctionType&& function)
{
	printf("\nBlock %d:\n\n", (BlockIndex + 1));

	TestCommon::MockObject a1("a1");
	TestCommon::MockObject a2("a2");
	TestCommon::MockObject a3("a3");
	TestCommon::MockObject a4("a4");
	TestCommon::MockObject b1("b1");
	TestCommon::MockObject b2("b2");
	TestCommon::MockObject b3("b3");
	TestCommon::MockObject b4("b4");
	auto& a1R = a1; const auto& a1CR = a1;
	auto& a2R = a2; const auto& a2CR = a2;
	auto& a3R = a3; const auto& a3CR = a3;
	auto& a4R = a4; const auto& a4CR = a4;
	auto& b1R = b1; const auto& b1CR = b1;
	auto& b2R = b2; const auto& b2CR = b2;
	auto& b3R = b3; const auto& b3CR = b3;

	TestFunction(threadPool, std::forward<FunctionType>(function),
		&a1, b1);
	TestFunction(threadPool, std::forward<FunctionType>(function),
		&a1R, b1);
	
	// Not supporting right references for the object.
	//TestFunction(threadPool, std::forward<FunctionType>(function),
	//	std::move(a1), b1);
	//TestFunction(threadPool, std::forward<FunctionType>(function),
	//	std::move(a2), b1R);
	//TestFunction(threadPool, std::forward<FunctionType>(function),
	//	std::move(a4), std::move(b3));

	TestFunction(threadPool, std::forward<FunctionType>(function),
		&a2, b1R);
	TestFunction(threadPool, std::forward<FunctionType>(function),
		&a2R, b1R);

	TestFunction(threadPool, std::forward<FunctionType>(function),
		&a4, std::move(b1));
	TestFunction(threadPool, std::forward<FunctionType>(function),
		&a4R, std::move(b2));

	printf("\nEnd block %d:\n\n", (BlockIndex + 1));

	TestParameteredFunctionsWithConstRefA<FunctionType, IsFunctionConst>
		::Test(threadPool, std::forward<FunctionType>(function));
	TestParameteredFunctionsWithConstRefB<FunctionType, IsParameterConst>
		::Test(threadPool, std::forward<FunctionType>(function));

	BlockIndex++;
}

void X(TestCommon::MockObject* o, int i)
{
	printf("X\n");
	o->Function();
}

void Y(int i, int j)
{
	printf("Y(%d, %d)\n", i, j);
}

template <typename T>
void Z(T&& i)
{
}

void H(const TestCommon::MockObject& a1, const TestCommon::MockObject& a2,
	const TestCommon::MockObject& a3, const TestCommon::MockObject& a4)
{
	printf("H()\n");
}

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

class EmptyClass
{
public:

	void EmptyFunction()
	{
	}

	void EmptyFunction2(unsigned threadId, unsigned startIndex, unsigned endIndex)
	{
	}

	void EmptyFunction3(unsigned threadId, void* userData)
	{
	}

	template <typename T>
	void TemplateFunction(T a, T b, T c)
	{
		printf("template function: %d %d %d\n", a, b, c);
	}

	template <typename FunctionType, typename ObjectType>
	void CallerFunction(FunctionType&& function, ObjectType* pObject)
	{
		(pObject->*function)(1, 2, 3);
	}
};

void GlobalFunction1(unsigned a, unsigned b, unsigned c)
{
	printf("global called function: %d %d %d\n", a, b, c);
}

template <typename FunctionType>
void GlobalCallerFunction1(FunctionType&& function)
{
	(*function)(1, 2, 3);
}

template <typename T>
void GlobalTemplateFunction1(T a, T b, T c)
{
	printf("template function: %d %d %d\n", a, b, c);
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	{
		EmptyClass object;

		Core::ThreadPool threadPool(8);

		Core::ApplyFunction(&EmptyClass::EmptyFunction, std::tuple<EmptyClass*>(&object));

		threadPool.ExecuteWithStaticScheduling(10, []
			(unsigned threadId, unsigned startIndex, unsigned endIndex) 
			{ 
				for (unsigned i = startIndex; i < endIndex; i++)
				{
					printf("Processing work item %d with thread %d.\n", i, threadId);
				}
			}
		);

		for (size_t outerSimIndex = 0; outerSimIndex < 1; outerSimIndex++)
		{
			Start();

			auto& threads = threadPool.GetThreads();
			for (size_t simIndex = 0; simIndex < 1000; simIndex++)
			{
				for (size_t i = 0; i < 8; i++)
				{
					threads[i].Execute(&EmptyClass::EmptyFunction, &object);
				}
				threadPool.Join();
			}

			Stop();
		}

		for (size_t outerSimIndex = 0; outerSimIndex < 1; outerSimIndex++)
		{
			Start();

			threadPool.ExecuteWithStaticScheduling(1000, &EmptyClass::EmptyFunction2, &object);

			for (size_t simIndex = 0; simIndex < 1000; simIndex++)
			{
				threadPool.ExecuteWithDynamicScheduling(8, &EmptyClass::EmptyFunction2, &object);
			}

			Stop();
		}

		//for (size_t outerSimIndex = 0; outerSimIndex < 1; outerSimIndex++)
		//{
		//	Start();

		//	auto& threads = simpleThreadPool.GetThreads();
		//	for (size_t simIndex = 0; simIndex < 1000; simIndex++)
		//	{
		//		for (size_t i = 0; i < 8; i++)
		//		{
		//			threads[i].Execute(&EmptyClass::EmptyFunction3, &object);
		//		}
		//		simpleThreadPool.Join();
		//	}

		//	Stop();
		//}

		//for (size_t outerSimIndex = 0; outerSimIndex < 1; outerSimIndex++)
		//{
		//	Start();

		//	for (size_t simIndex = 0; simIndex < 1000; simIndex++)
		//	{
		//		simpleThreadPool.ExecuteWithStaticScheduling(8, &EmptyClass::EmptyFunction3, &object);
		//	}

		//	Stop();
		//}

		for (size_t outerSimIndex = 0; outerSimIndex < 1; outerSimIndex++)
		{
			EmptyClass e;

			Start();

			for (size_t simIndex = 0; simIndex < 1000; simIndex++)
			{
				e.EmptyFunction();
			}

			Stop();
		}
	}

	//{
	//	printf("\nInitialization:\n\n");

	//	auto threadPool = new Core::ThreadPool(1);
	//	auto& thread = threadPool->GetThread(0);

	//	printf("\nBlock 1:\n\n");
	//	{
	//		TestCommon::MockObject a("a");
	//		TestCommon::MockObject b("b");
	//		auto& aR = a;
	//		const auto& aCR = a;
	//		auto& bR = b;
	//		const auto& bCR = b;

	//		// Not supporting right references for the object.
	//		//TestFunction(threadPool, &TestCommon::MockObject::Function, std::move(a));
	//		//TestFunction(threadPool, &TestCommon::MockObject::ConstFunction, std::move(b));

	//		TestFunction(threadPool, &TestCommon::MockObject::Function, &a);
	//		TestFunction(threadPool, &TestCommon::MockObject::Function, &aR);
	//		
	//		TestFunction(threadPool, &TestCommon::MockObject::ConstFunction, &b);
	//		TestFunction(threadPool, &TestCommon::MockObject::ConstFunction, &bR);
	//		TestFunction(threadPool, &TestCommon::MockObject::ConstFunction, &bCR);

	//		printf("\nEnd block 1:\n\n");

	//		BlockIndex++;
	//	}

	//	TestParameteredFunctions<true, true>(threadPool, &TestCommon::MockObject::ConstFunction_Copy);
	//	TestParameteredFunctions<true, true>(threadPool, &TestCommon::MockObject::ConstFunction_ConstRef);
	//	TestParameteredFunctions<true, true>(threadPool, &TestCommon::MockObject::ConstFunction_Ref);
	//	TestParameteredFunctions<false, true>(threadPool, &TestCommon::MockObject::Function_Copy);
	//	TestParameteredFunctions<false, true>(threadPool, &TestCommon::MockObject::Function_ConstRef);
	//	TestParameteredFunctions<false, true>(threadPool, &TestCommon::MockObject::Function_Ref);
	//	TestParameteredFunctions<true, true>(threadPool, &TestCommon::MockObject::ConstFunction_RightRef);
	//	TestParameteredFunctions<false, true>(threadPool, &TestCommon::MockObject::Function_RightRef);

	//	TestCommon::MockObject a1("a1");
	//	TestCommon::MockObject a2("a2");
	//	TestCommon::MockObject a3("a3");
	//	TestCommon::MockObject a4("a4");
	//	TestCommon::MockObject b1("b1");
	//	TestCommon::MockObject b2("b2");
	//	TestCommon::MockObject b3("b3");
	//	TestCommon::MockObject b4("b4");
	//	auto& a1R = a1; const auto& a1CR = a1;
	//	auto& a2R = a2; const auto& a2CR = a2;
	//	auto& a3R = a3; const auto& a3CR = a3;
	//	auto& a4R = a4; const auto& a4CR = a4;
	//	auto& b1R = b1; const auto& b1CR = b1;
	//	auto& b2R = b2; const auto& b2CR = b2;
	//	auto& b3R = b3; const auto& b3CR = b3;

	//	for (size_t i = 0; i < 10000; i++)
	//	{
	//		threadPool->GetThread(0).Execute(
	//			&TestCommon::MockObject::FunctionWithTwoIntParameter, &a2, 7, 42);
	//	}

	//	int i = 42;
	//	int j = 43;
	//	TestFunction(threadPool, &Y, i, j);
	//	for (int x = 0; x < 10; x++)
	//	{
	//		threadPool->GetThread(0).Execute(
	//			&TestCommon::MockObject::FunctionWithTwoIntParameter, &a2, x, 42);
	//	}
	//	threadPool->Join();

	//	TestFunction(threadPool, &H, a1, a2, a3, a4);

	//	for (size_t i = 0; i < 1000; i++)
	//	{
	//		TestFunction(threadPool, &TestCommon::MockObject::ConstFunction_RightRef, &a2,
	//			std::move(b1));
	//		TestFunction(threadPool, &TestCommon::MockObject::ConstFunction_RightRef, &a2,
	//			std::move(b1));
	//	}

	//	printf("\nEnd:\n\n");

	//	delete threadPool;
	//}

	//TestCommon::MockObject::ReportMemoryLeaks();

    return 0;
}

