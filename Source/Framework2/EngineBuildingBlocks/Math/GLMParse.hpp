// EngineBuildingBlocks/Math/GLMParse.hpp

#ifndef ENGINEBUILDINGBLOCKS_GLMPARSE_HPP_INLUDED_
#define ENGINEBUILDINGBLOCKS_GLMPARSE_HPP_INLUDED_

#include <Core/Parse.hpp>
#include <Core/String.hpp>
#include <EngineBuildingBlocks/Math/GLM.h>

#include <cassert>

namespace Core
{
	namespace detail
	{
		template <typename T, glm::precision P>
		inline glm::tvec2<T, P> ParseGLM_tvec2(const char* str)
		{
			auto parts = Core::Split(str, Core::IsCharacter(','), false);
			assert(parts.size() == 2);
			return{ Core::Parse<T>(parts[0]), Core::Parse<T>(parts[1]) };
		}

		template <typename T, glm::precision P>
		inline glm::tvec3<T, P> ParseGLM_tvec3(const char* str)
		{
			auto parts = Core::Split(str, Core::IsCharacter(','), false);
			assert(parts.size() == 3);
			return{ Core::Parse<T>(parts[0]), Core::Parse<T>(parts[1]), Core::Parse<T>(parts[2]) };
		}

		template <typename T, glm::precision P>
		inline glm::tvec4<T, P> ParseGLM_tvec4(const char* str)
		{
			auto parts = Core::Split(str, Core::IsCharacter(','), false);
			assert(parts.size() == 4);
			return{ Core::Parse<T>(parts[0]), Core::Parse<T>(parts[1]),
					Core::Parse<T>(parts[2]), Core::Parse<T>(parts[3]) };
		}
	}

	template <>
	inline glm::vec2 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec2<float, glm::highp>(str);
	}

	template <>
	inline glm::dvec2 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec2<double, glm::highp>(str);
	}

	template <>
	inline glm::ivec2 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec2<int, glm::highp>(str);
	}

	template <>
	inline glm::uvec2 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec2<unsigned, glm::highp>(str);
	}

	template <>
	inline glm::bvec2 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec2<bool, glm::highp>(str);
	}

	template <>
	inline glm::vec3 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec3<float, glm::highp>(str);
	}

	template <>
	inline glm::dvec3 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec3<double, glm::highp>(str);
	}

	template <>
	inline glm::ivec3 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec3<int, glm::highp>(str);
	}

	template <>
	inline glm::uvec3 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec3<unsigned, glm::highp>(str);
	}

	template <>
	inline glm::bvec3 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec3<bool, glm::highp>(str);
	}

	template <>
	inline glm::vec4 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec4<float, glm::highp>(str);
	}

	template <>
	inline glm::dvec4 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec4<double, glm::highp>(str);
	}

	template <>
	inline glm::ivec4 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec4<int, glm::highp>(str);
	}

	template <>
	inline glm::uvec4 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec4<unsigned, glm::highp>(str);
	}

	template <>
	inline glm::bvec4 Parse(const char* str)
	{
		return Core::detail::ParseGLM_tvec4<bool, glm::highp>(str);
	}
}

#endif