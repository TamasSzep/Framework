// TestCommon/MockObject.h

#ifndef _TESTCOMMON_MOCKOBJECT_HPP_
#define _TESTCOMMON_MOCKOBJECT_HPP_

#include <Core/FixedSizedOutputStream.hpp>
#include <TestCommon/FixedSizedString.hpp>

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <sstream>
#include <map>

namespace TestCommon
{
	class MockObject
	{
		class StaticDataPovider
		{
		public:

			static const unsigned s_ExpectedElementSize = 1024;

			typedef Core::FixedSizedOutputStream<1024> OutputBuffer;
			typedef FixedSizedString<256> String;

		private:

			struct ThreadData
			{
				OutputBuffer OutputBuffer;
			};

			std::map<std::thread::id, ThreadData*> m_ThreadData;
			std::vector<String*> m_Names;
			std::vector<bool> m_IsNameUsed;

			ThreadData* GetThreadData();

		public:

			StaticDataPovider();
			~StaticDataPovider();

			OutputBuffer& GetOutputStream();
			String* RequestNameObject();
			void ReleaseNameObject(String* object);
		};

		static std::mutex s_Mutex;
		static unsigned s_Id;
		static unsigned s_CountObjects;
		static StaticDataPovider s_StaticDataPovider;

		unsigned m_Id;
		StaticDataPovider::String* m_Name;

		void Initialize(const char* name = nullptr);

		StaticDataPovider::OutputBuffer& StartMocking() const;
		void EndMocking(StaticDataPovider::OutputBuffer& ss) const;

		void MockThisObject(StaticDataPovider::OutputBuffer& ss) const;

		void StartFunctionMocking(StaticDataPovider::OutputBuffer& ss,
			const char* functionName) const;
		void EndFunctionMocking(StaticDataPovider::OutputBuffer& ss) const;

		void AddToMock(StaticDataPovider::OutputBuffer& ss, const char* str) const;

		StaticDataPovider::OutputBuffer& GetOutputStream() const;

	private: // Simulating a resource object.

		int* m_Resource;

	public:

		MockObject();

		MockObject(const std::string& name);

		~MockObject();

		MockObject(const MockObject& other);
		MockObject(MockObject&& other);

		MockObject& operator=(const MockObject& other);
		MockObject& operator=(MockObject&& other);

		void Function();
		void ConstFunction() const;

		void Function_Copy(MockObject other);
		void Function_Ref(MockObject& other);
		void Function_ConstRef(const MockObject& other);
		void Function_RightRef(MockObject&& other);
		void ConstFunction_Copy(MockObject other) const;
		void ConstFunction_Ref(MockObject& other) const;
		void ConstFunction_ConstRef(const MockObject& other) const;
		void ConstFunction_RightRef(MockObject&& other) const;

		void FunctionWithTwoIntParameter(int i, int j) const;

		static void ReportMemoryLeaks();
	};
}

#endif
