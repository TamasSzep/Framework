// EngineBuildingBlocks/Math/IntervalArithmetic.h

#pragma once

#include <limits>

namespace EngineBuildingBlocks
{
	namespace Math
	{
		const float c_FloatMax = std::numeric_limits<float>::max();
		const float c_FloatMin = std::numeric_limits<float>::lowest();

		struct IntervalF
		{
			float Min, Max;

			IntervalF();
			IntervalF(float value);
			IntervalF(float min, float max);

			bool IsLengthZero() const;
			float GetMiddle() const;
			void Split(IntervalF* first, IntervalF* second) const;

			IntervalF operator-() const;

			IntervalF operator+(float right) const;
			IntervalF operator-(float right) const;
			IntervalF operator*(float right) const;
			IntervalF operator/(float right) const;

			IntervalF operator+(const IntervalF& right) const;
			IntervalF operator-(const IntervalF& right) const;
			IntervalF operator*(const IntervalF& right) const;
			IntervalF operator/(const IntervalF& right) const;

			IntervalF GetInverse() const;
		};

		IntervalF operator+(float left, const IntervalF& right);
		IntervalF operator-(float left, const IntervalF& right);
		IntervalF operator*(float left, const IntervalF& right);
		IntervalF operator/(float left, const IntervalF& right);
	}
}
