// ObjectCacheTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/ObjectCache.hpp>

struct A
{
	int I;
	int J;

	A()
	{
	}

	A(int i, int j)
		: I(i)
		, J(j)
	{
	}
};

struct B
{
	double D;
};

int main()
{
	Core::ObjectCache objectCache;

	auto pA1 = objectCache.Request<A>();
	auto pA2 = objectCache.Request<A>();
	objectCache.Release(pA1);
	auto pA3 = objectCache.Request<A>();
	auto pA4 = objectCache.Request<A>(42, 43);

	auto pB1 = objectCache.Request<B>();

	return 0;
}

