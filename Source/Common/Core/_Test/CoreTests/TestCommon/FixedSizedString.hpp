// TestCommon/FixedSizedString.h

#ifndef _TESTCOMMON_FIXEDSIZEDSTRING_HPP_
#define _TESTCOMMON_FIXEDSIZEDSTRING_HPP_

#include <array>
#include <string>
#include <cstdio>

#include <Core/Platform.h>

namespace TestCommon
{
	template <size_t Size>
	class FixedSizedString
	{
		std::array<char, Size + 1> m_Buffer;
		size_t m_StartIndex;

		inline char* GetDestinationPointer()
		{
			return &m_Buffer[m_StartIndex];
		}

		inline size_t GetRemainingSize() const
		{
			return (Size - m_StartIndex);
		}

	public:

		FixedSizedString()
			: m_StartIndex(0)
		{
		}
	
		const char* ToCString()
		{
			m_Buffer[m_StartIndex] = '\0';
			return m_Buffer.data();
		}

		FixedSizedString& operator=(const char* str)
		{
			size_t length = strlen(str);
			memcpy(m_Buffer.data(), str, length);
			m_StartIndex = length;
			return *this;
		}
	};
}

#endif