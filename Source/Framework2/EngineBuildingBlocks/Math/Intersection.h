// EngineBuildingBlocks/Math/Intersection.h

#ifndef _ENGINEBUILDINGBLOCKS_INTERSECTION_H_INCLUDED_
#define _ENGINEBUILDINGBLOCKS_INTERSECTION_H_INCLUDED_

#include <EngineBuildingBlocks/Math/GLM.h>

#include <limits>

namespace EngineBuildingBlocks
{
	namespace Math
	{
		static const float c_InvalidIntersectionT = std::numeric_limits<float>::max();

		float GetRayTriangleIntersection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir);
		void GetRayAABBIntersection(const glm::vec3& min, const glm::vec3& max,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir, float* pTMin, float* pTMax);

		float GetRayTriangleIntersection_PositiveT(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir);
		float GetRayAABBIntersection_PositiveT(const glm::vec3& min, const glm::vec3& max,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir);
	}
}

#endif