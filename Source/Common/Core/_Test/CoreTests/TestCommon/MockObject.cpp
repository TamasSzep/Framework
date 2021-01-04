// TestCommon/MockObject.cpp

#include "stdafx.h"

#include <TestCommon/MockObject.h>

#include <iostream>

using namespace TestCommon;

std::mutex MockObject::s_Mutex;
unsigned MockObject::s_Id = 0;
unsigned MockObject::s_CountObjects = 0;
MockObject::StaticDataPovider MockObject::s_StaticDataPovider;

MockObject::StaticDataPovider::StaticDataPovider()
	: m_IsNameUsed(s_ExpectedElementSize, false)
{
	m_Names.reserve(s_ExpectedElementSize);
	for (unsigned i = 0; i < s_ExpectedElementSize; i++)
	{
		m_Names.push_back(new String());
	}
}

MockObject::StaticDataPovider::~StaticDataPovider()
{
	for (auto& it : m_ThreadData)
	{
		delete it.second;
	}
	for (size_t i = 0; i < m_Names.size(); i++)
	{
		delete m_Names[i];
	}
}

MockObject::StaticDataPovider::ThreadData* MockObject::StaticDataPovider::GetThreadData()
{
	auto threadId = std::this_thread::get_id();
	auto it = m_ThreadData.find(threadId);
	if (it == m_ThreadData.end())
	{
		auto data = new ThreadData();
		m_ThreadData[threadId] = data;
		return data;
	}
	return it->second;
}

MockObject::StaticDataPovider::OutputBuffer&
MockObject::StaticDataPovider::GetOutputStream()
{
	return GetThreadData()->OutputBuffer;
}

MockObject::StaticDataPovider::String* MockObject::StaticDataPovider::RequestNameObject()
{
	for (size_t i = 0; i < m_Names.size(); i++)
	{
		if (!m_IsNameUsed[i])
		{
			m_IsNameUsed[i] = true;
			return m_Names[i];
		}
	}

	auto oldSize = m_Names.size();
	auto newSize = oldSize * 2;
	m_IsNameUsed.resize(newSize, false);
	m_Names.reserve(newSize);
	for (size_t i = oldSize; i < newSize; i++)
	{
		m_Names.push_back(new String());
	}
	m_IsNameUsed[oldSize] = true;
	return m_Names[oldSize];
}

void MockObject::StaticDataPovider::ReleaseNameObject(String* object)
{
	for (size_t i = 0; i < m_Names.size(); i++)
	{
		if (m_Names[i] == object)
		{
			m_IsNameUsed[i] = false;
			return;
		}
	}
}

MockObject::StaticDataPovider::OutputBuffer& MockObject::GetOutputStream() const
{
	std::lock_guard<std::mutex> guard(s_Mutex);
	return s_StaticDataPovider.GetOutputStream();
}

void MockObject::Initialize(const char* name)
{
	s_Mutex.lock();
	m_Id = s_Id++;
	m_Name = s_StaticDataPovider.RequestNameObject();
	s_CountObjects++;
	s_Mutex.unlock();

	if (name == nullptr)
	{
		*m_Name = "<unnamed>";
	}
	else
	{
		*m_Name = name;
	}
}

MockObject::StaticDataPovider::OutputBuffer& MockObject::StartMocking() const
{
	return GetOutputStream();
}

void MockObject::EndMocking(StaticDataPovider::OutputBuffer& ss) const
{
	ss << "\n";
	ss.Flush();
}

void MockObject::MockThisObject(StaticDataPovider::OutputBuffer& ss) const
{
	ss << "[" << m_Id << ":" << m_Name->ToCString() << "]";
}

void MockObject::StartFunctionMocking(StaticDataPovider::OutputBuffer& ss,
	const char* functionName) const
{
	ss << " ." << functionName << " ( ";
}

void MockObject::EndFunctionMocking(StaticDataPovider::OutputBuffer& ss) const
{
	ss << " )";
}

void MockObject::AddToMock(StaticDataPovider::OutputBuffer& ss, const char* str) const
{
	ss << str;
}

MockObject::MockObject()
	: m_Resource(new int(0))
{
	Initialize();

	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ctor");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

MockObject::MockObject(const std::string& name)
	: m_Resource(new int(0))
{
	Initialize(name.c_str());

	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ctor");
	AddToMock(ss, "\"");
	AddToMock(ss, name.c_str());
	AddToMock(ss, "\" const &");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

MockObject::~MockObject()
{
	s_Mutex.lock();
	s_CountObjects--;
	s_Mutex.unlock();

	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "dtor");
	EndFunctionMocking(ss);
	EndMocking(ss);

	delete m_Resource;
}

MockObject::MockObject(const MockObject& other)
	: m_Resource(new int(*other.m_Resource))
{
	Initialize();

	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ctor");
	other.MockThisObject(ss);
	AddToMock(ss, " const &");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

MockObject::MockObject(MockObject&& other)
	: m_Resource(other.m_Resource)
{
	Initialize();

	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ctor");
	other.MockThisObject(ss);
	AddToMock(ss, " &&");
	EndFunctionMocking(ss);
	EndMocking(ss);

	other.m_Resource = nullptr;
}

MockObject& MockObject::operator=(const MockObject& other)
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "=");
	other.MockThisObject(ss);
	AddToMock(ss, " const &");
	EndFunctionMocking(ss);
	EndMocking(ss);

	if (m_Resource != other.m_Resource)
	{
		delete m_Resource;
		m_Resource = new int(*other.m_Resource);
	}

	return *this;
}

MockObject& MockObject::operator=(MockObject&& other)
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "=");
	other.MockThisObject(ss);
	AddToMock(ss, " &&");
	EndFunctionMocking(ss);
	EndMocking(ss);

	if (m_Resource != other.m_Resource)
	{
		delete m_Resource;
		m_Resource = other.m_Resource;
		other.m_Resource = nullptr;
	}

	return *this;
}

void MockObject::Function()
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "Function");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::ConstFunction() const
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ConstFunction");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::Function_Copy(MockObject other)
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "Function_Copy");
	other.MockThisObject(ss);
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::Function_Ref(MockObject& other)
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "Function_Ref");
	other.MockThisObject(ss);
	AddToMock(ss, " &");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::Function_ConstRef(const MockObject& other)
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "Function_ConstRef");
	other.MockThisObject(ss);
	AddToMock(ss, " const &");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::Function_RightRef(MockObject&& other)
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "Function_RightRef");
	other.MockThisObject(ss);
	AddToMock(ss, " &&");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::ConstFunction_Copy(MockObject other) const
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ConstFunction_Copy");
	other.MockThisObject(ss);
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::ConstFunction_Ref(MockObject& other) const
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ConstFunction_Ref");
	other.MockThisObject(ss);
	AddToMock(ss, " &");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::ConstFunction_ConstRef(const MockObject& other) const
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ConstFunction_ConstRef");
	other.MockThisObject(ss);
	AddToMock(ss, " const &");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::ConstFunction_RightRef(MockObject&& other) const
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "ConstFunction_RightRef");
	other.MockThisObject(ss);
	AddToMock(ss, " &&");
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::FunctionWithTwoIntParameter(int i, int j) const
{
	auto& ss = StartMocking();
	MockThisObject(ss);
	StartFunctionMocking(ss, "FunctionWithTwoIntParameter");
	ss << i << ", " << j;
	EndFunctionMocking(ss);
	EndMocking(ss);
}

void MockObject::ReportMemoryLeaks()
{
	s_Mutex.lock();
	if (s_CountObjects == 0)
	{
		printf("No memory leaks.\n");
	}
	else
	{
		printf("%d MockObject were not destructed.\n", s_CountObjects);
	}
	s_Mutex.unlock();
}