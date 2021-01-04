// MathHelper.h

#ifndef _CORE_MATHHELPER_H_
#define _CORE_MATHHELPER_H_

#include <vector>

namespace Core
{
	class MathHelper
	{
	public:

		static void SolveLinearEquationSystem(const std::vector<std::vector<float>>& A,
			const std::vector<float>& b, std::vector<float>& x);
	};
}

#endif