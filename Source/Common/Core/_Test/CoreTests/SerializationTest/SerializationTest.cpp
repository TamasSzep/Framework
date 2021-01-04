// SerializationTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/SimpleBinarySerialization.hpp>

#include <vector>
#include <string>

struct A
{
	int I;
	std::string Str;

	void Serialize(Core::SimpleBinarySerializer& serializer) const
	{
		serializer.Serialize(I);
		serializer.Serialize(Str);
	}

	void Deserialize(Core::SimpleBinaryDeserializer& deserializer)
	{
		deserializer.Deserialize(I);
		deserializer.Deserialize(Str);
	}
};

struct B
{
	std::vector<A> As;
	std::string Str2;

	void Serialize(Core::SimpleBinarySerializer& serializer) const
	{
		serializer.Serialize(As);
		serializer.Serialize(Str2);
	}

	void Deserialize(Core::SimpleBinaryDeserializer& deserializer)
	{
		deserializer.Deserialize(As);
		deserializer.Deserialize(Str2);
	}
};

struct C
{
	std::vector<A> As2;
	std::vector<B> Bs;
	int I2;

	void Serialize(Core::SimpleBinarySerializer& serializer) const
	{
		serializer.Serialize(As2);
		serializer.Serialize(Bs);
		serializer.Serialize(I2);
	}

	void Deserialize(Core::SimpleBinaryDeserializer& deserializer)
	{
		deserializer.Deserialize(As2);
		deserializer.Deserialize(Bs);
		deserializer.Deserialize(I2);
	}
};

int main()
{
	C c;
	c.I2 = 42;
	c.As2.push_back({ 0, "zero" });
	c.Bs.push_back({});
	c.Bs.push_back({});
	c.Bs[0].Str2 = "B0";
	c.Bs[0].As.push_back({ 1, "one" });
	c.Bs[1].Str2 = "B1";
	c.Bs[1].As.push_back({ 2, "two" });
	C c2;
	c2.As2.push_back({});

	Core::SimpleBinarySerializer serializer;
	serializer.Start();
	serializer.Serialize(c);

	Core::SimpleBinaryDeserializer deserializer;
	deserializer.Deserialize(c2, serializer.GetBytes());

    return 0;
}

