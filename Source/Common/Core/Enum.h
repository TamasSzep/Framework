// Core/Enum.h

#pragma once

#include <type_traits>

#define UseEnumAsFlagSet(enumName)														\
	constexpr inline enumName operator|(enumName a, enumName b)							\
	{																					\
	return static_cast<enumName>(static_cast<std::underlying_type_t<enumName>>(a)		\
	| static_cast<std::underlying_type_t<enumName>>(b));								\
	}																					\
	constexpr inline enumName& operator|=(enumName& a, enumName b)						\
	{																					\
	a = static_cast<enumName>(static_cast<std::underlying_type_t<enumName>>(a)			\
	| static_cast<std::underlying_type_t<enumName>>(b));								\
	return a;																			\
	}																					\
	constexpr inline enumName operator&(enumName a, enumName b)							\
	{																					\
	return static_cast<enumName>(static_cast<std::underlying_type_t<enumName>>(a)		\
	& static_cast<std::underlying_type_t<enumName>>(b));								\
	}																					\
	constexpr inline enumName& operator&=(enumName& a, enumName b)						\
	{																					\
	a = static_cast<enumName>(static_cast<std::underlying_type_t<enumName>>(a)			\
	& static_cast<std::underlying_type_t<enumName>>(b));								\
	return a;																			\
	}																					\
	constexpr inline enumName operator~(enumName a)										\
	{																					\
	return static_cast<enumName>(~static_cast<std::underlying_type_t<enumName>>(a));	\
	}																					\
	constexpr inline bool HasFlag(enumName a, enumName b)								\
	{return ((static_cast<std::underlying_type_t<enumName>>(a)							\
	& static_cast<std::underlying_type_t<enumName>>(b)) != 0);							\
	}																					\
	constexpr inline bool IsSubset(enumName superSet, enumName subset)					\
	{return ((~static_cast<std::underlying_type_t<enumName>>(superSet)					\
	& static_cast<std::underlying_type_t<enumName>>(subset)) == 0);						\
	}
