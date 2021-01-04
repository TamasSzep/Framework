// TestCommon/FixedSizedOutputStream.h

#ifndef _CORE_FIXEDSIZEDOUTPUTSTREAM_HPP_
#define _CORE_FIXEDSIZEDOUTPUTSTREAM_HPP_

#include <array>
#include <string>
#include <cstdio>

#include <Core/Platform.h>

namespace Core
{
	template <size_t Size>
	class FixedSizedOutputStream
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

		inline void AdjustStartIndex()
		{
			while (m_StartIndex < Size && m_Buffer[m_StartIndex] != '\0') ++m_StartIndex;
		}

		inline void CopyString(const std::string& str)
		{
			size_t length = str.length();
			memcpy(GetDestinationPointer(), str.data(), length);
			m_StartIndex += length;
		}

		inline void CopyString(const char* str)
		{
			size_t length = strlen(str);
			memcpy(GetDestinationPointer(), str, length);
			m_StartIndex += length;
		}

	public:

		FixedSizedOutputStream()
			: m_StartIndex(0)
		{
		}

		void Flush()
		{
			m_Buffer[m_StartIndex] = '\0';

			printf("%s", m_Buffer.data());
		
			m_StartIndex = 0;
		}

		const char* ToCString()
		{
			m_Buffer[m_StartIndex] = '\0';
			return m_Buffer.data();
		}

		void Reset()
		{
			m_StartIndex = 0;
		}

		FixedSizedOutputStream& operator<<(const std::string& str)
		{
			CopyString(str);
			return *this;
		}

		FixedSizedOutputStream& operator<<(const char* str)
		{
			CopyString(str);
			return *this;
		}

		FixedSizedOutputStream& operator<<(int i)
		{
#ifdef IS_WINDOWS
			_itoa_s(i, GetDestinationPointer(), GetRemainingSize(), 10);
#else
			itoa(i, GetDestinationPointer(), 10);
#endif
			AdjustStartIndex();
			return *this;
		}
	};
}

#endif
