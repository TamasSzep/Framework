// SharedPointerTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/SharedPointerST.hpp>

class A {};
class B : public A {};


void F(const Core::SharedPointerST<A>& a)
{
}

int main()
{
	auto b = Core::MakeSharedPointerST<B>();
	F(b);

    return 0;
}

