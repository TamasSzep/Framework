// Core/IntervalData.hpp

#ifndef _CORE_INTERVALDATA_HPP_
#define _CORE_INTERVALDATA_HPP_

#include <Core/Comparison.h>
#include <Core/SimpleBinarySerialization.hpp>

namespace Core
{
	template <typename T>
	struct IntervalData
	{
		T First;
		T Last;
		T Step;

		bool IsValid() const
		{
			return (First <= Last);
		}

		bool operator==(const IntervalData<T>& other) const
		{
			NumericalEqualCompareBlock(First);
			NumericalEqualCompareBlock(Last);
			NumericalEqualCompareBlock(Step);
			return true;
		}

		bool operator!=(const IntervalData<T>& other) const
		{
			return !(*this == other);
		}

		bool operator<(const IntervalData<T>& other) const
		{
			NumericalLessCompareBlock(First);
			NumericalLessCompareBlock(Last);
			NumericalLessCompareBlock(Step);
			return false;
		}

		void SerializeSB(ByteVector& bytes) const
		{
			Core::SerializeSB(bytes, First);
			Core::SerializeSB(bytes, Last);
			Core::SerializeSB(bytes, Step);
		}

		void DeserializeSB(const unsigned char*& bytes)
		{
			DeserializeSB(bytes, First);
			DeserializeSB(bytes, Last);
			DeserializeSB(bytes, Step);
		}
	};
}

#endif