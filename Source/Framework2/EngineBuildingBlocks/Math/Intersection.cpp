// EngineBuildingBlocks/Math/Intersection.cpp

#include <EngineBuildingBlocks/Math/Intersection.h>

#include <algorithm>

namespace EngineBuildingBlocks
{
	namespace Math
	{
		float GetRayTriangleIntersection(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir)
		{
			// Triangle side vectors.
			auto a = p1 - p0;
			auto b = p2 - p1;
			auto c = p0 - p2;

			// Checking side vector lengths.
			const float lEpsilon = 1e-10f;
			if (glm::length(a) < lEpsilon || glm::length(b) < lEpsilon || glm::length(c) < lEpsilon) return c_InvalidIntersectionT;

			// Triangle's plane.
			auto n = glm::normalize(glm::cross(a, b));
			auto d = -glm::dot(n, p0);

			// Intersection with the triangle's plane. Also checking whether the ray is nearly parallel to the plane.
			auto div = glm::dot(n, rayDir);
			const float divEpsilon = 1e-6f;
			if (std::abs(div) < divEpsilon) return c_InvalidIntersectionT;
			auto t = -(d + glm::dot(n, rayOrigin)) / div;
			auto x = rayOrigin + rayDir * t;

			// Checking edge functions.
			if (glm::dot(n, glm::cross(a, x - p0)) < 0.0f
				|| glm::dot(n, glm::cross(b, x - p1)) < 0.0f
				|| glm::dot(n, glm::cross(c, x - p2)) < 0.0f) return c_InvalidIntersectionT;

			return t;
		}

		inline void RayAABBIntersection_LimitT(float o, float d, float min, float max, float* pTMin, float* pTMax)
		{
			if (std::abs(d) < 1e-6f)
			{
				if (o < min || o > max)
				{
					*pTMin = std::numeric_limits<float>::max();
					*pTMax = std::numeric_limits<float>::lowest();
				}
			}
			else
			{
				float t0 = (min - o) / d;
				float t1 = (max - o) / d;
				if (t0 > t1) std::swap(t0, t1);
				*pTMin = std::max(*pTMin, t0);
				*pTMax = std::min(*pTMax, t1);
			}
		}

		void GetRayAABBIntersection(const glm::vec3& min, const glm::vec3& max,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir, float* pTMin, float* pTMax)
		{
			float tMin = std::numeric_limits<float>::lowest();
			float tMax = std::numeric_limits<float>::max();
			RayAABBIntersection_LimitT(rayOrigin.x, rayDir.x, min.x, max.x, &tMin, &tMax);
			if (tMin > tMax) { *pTMin = *pTMax = c_InvalidIntersectionT; return; }
			RayAABBIntersection_LimitT(rayOrigin.y, rayDir.y, min.y, max.y, &tMin, &tMax);
			if (tMin > tMax) { *pTMin = *pTMax = c_InvalidIntersectionT; return; }
			RayAABBIntersection_LimitT(rayOrigin.z, rayDir.z, min.z, max.z, &tMin, &tMax);
			if (tMin > tMax) { *pTMin = *pTMax = c_InvalidIntersectionT; return; }		
			*pTMin = tMin;
			*pTMax = tMax;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		float GetRayTriangleIntersection_PositiveT(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir)
		{
			auto t = GetRayTriangleIntersection(p0, p1, p2, rayOrigin, rayDir);
			return (t >= 0.0f ? t : c_InvalidIntersectionT);
		}

		float GetRayAABBIntersection_PositiveT(const glm::vec3& min, const glm::vec3& max,
			const glm::vec3& rayOrigin, const glm::vec3 rayDir)
		{
			float tMin, tMax;
			GetRayAABBIntersection(min, max, rayOrigin, rayDir, &tMin, &tMax);
			if (tMin == c_InvalidIntersectionT || tMax < 0.0f) return c_InvalidIntersectionT;
			return std::max(tMin, 0.0f);
		}
	}
}