// EngineBuildingBlocks/Math/IntervalArithmetic.cpp

#include <EngineBuildingBlocks/Math/IntervalArithmetic.h>

#include <algorithm>

using namespace EngineBuildingBlocks::Math;

IntervalF::IntervalF()
	: Min(c_FloatMin)
	, Max(c_FloatMax)
{}

IntervalF::IntervalF(float value)
	: Min(value)
	, Max(value)
{}

IntervalF::IntervalF(float min, float max)
	: Min(min)
	, Max(max)
{}

bool IntervalF::IsLengthZero() const
{
	return Min == Max;
}

float IntervalF::GetMiddle() const
{
	return (Min + Max) * 0.5f;
}

void IntervalF::Split(IntervalF* first, IntervalF* second) const
{
	float middle = GetMiddle();
	*first = { Min, middle };
	*second = { middle, Max };
}

IntervalF IntervalF::operator-() const
{
	return { -Max, -Min };
}

IntervalF IntervalF::operator+(float right) const
{
	return { Min + right, Max + right };
}

IntervalF IntervalF::operator-(float right) const
{
	return { Min - right, Max - right };
}

IntervalF IntervalF::operator*(float right) const
{
	if (right < 0.0f)  return { Max * right, Min * right };
	return { Min * right, Max * right };
}

IntervalF IntervalF::operator/(float right) const
{
	if(right == 0.0f) return { c_FloatMin, c_FloatMax };
	if(right < 0.0f) return { Max / right, Min / right };
	return { Min / right, Max / right };
}

IntervalF IntervalF::operator+(const IntervalF& right) const
{
	return { Min + right.Min, Max + right.Max };
}

IntervalF IntervalF::operator-(const IntervalF& right) const
{
	return { Min - right.Max, Max - right.Min };
}

IntervalF IntervalF::operator*(const IntervalF& right) const
{
	float values[4];
	values[0] = Min * right.Min;
	values[1] = Min * right.Max;
	values[2] = Max * right.Min;
	values[3] = Max * right.Max;
	return {
		std::min(std::min(std::min(values[0], values[1]), values[2]), values[3]),
		std::max(std::max(std::max(values[0], values[1]), values[2]), values[3]) };
}

IntervalF IntervalF::operator/(const IntervalF& right) const
{
	if (right.Min <= 0.0f && right.Max >= 0.0f) return { c_FloatMin, c_FloatMax };
	float values[4];
	values[0] = Min / right.Min;
	values[1] = Min / right.Max;
	values[2] = Max / right.Min;
	values[3] = Max / right.Max;
	return {
		std::min(std::min(std::min(values[0], values[1]), values[2]), values[3]),
		std::max(std::max(std::max(values[0], values[1]), values[2]), values[3]) };
}

IntervalF IntervalF::GetInverse() const
{
	if (Min <= 0.0f && Max >= 0.0f) return { c_FloatMin, c_FloatMax };
	return { 1.0f / Max, 1.0f / Min };
}

namespace EngineBuildingBlocks
{
	namespace Math
	{

		IntervalF operator+(float left, const IntervalF& right) { return right + left; }
		IntervalF operator-(float left, const IntervalF& right) { return (-right) + left; }
		IntervalF operator*(float left, const IntervalF& right) { return right * left; }
		IntervalF operator/(float left, const IntervalF& right) { return right.GetInverse() * left; }
	}
}