// StringTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Core/String.hpp>

#include <cassert>

int main()
{
	std::string empty = "";
	std::string s1 = "abc";
	std::string s2 = "\n\r\f\t    \n \v \rabcabc";
	std::string s3 = "\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f";
	std::string s4 = "abcabc\f\t  \n\r\f\n";

	assert(Core::Trim("\n") == "");
	assert(Core::Trim(empty) == "");
	assert(Core::Trim("") == "");
	assert(Core::TrimLeft(empty) == "");
	assert(Core::TrimLeft("") == "");
	assert(Core::TrimRight(empty) == "");
	assert(Core::TrimRight("") == "");

	std::string deb1 = Core::Trim(s1);
	std::string deb2 = Core::Trim(s2);
	std::string deb3 = Core::Trim(s3);
	std::string deb4x = Core::Trim(s4);

	std::string deb4 = Core::TrimLeft(s1);
	std::string deb5 = Core::TrimLeft(s2);
	std::string deb6 = Core::TrimLeft(s3);
	std::string deb8x = Core::TrimLeft(s4);

	std::string deb7 = Core::TrimRight(s1);
	std::string deb8 = Core::TrimRight(s2);
	std::string deb9 = Core::TrimRight(s3);
	std::string deb12x = Core::TrimRight(s4);

	std::string deb21 = Core::Trim(s1, Core::IsAnyOf("\n\r"));
	std::string deb22 = Core::Trim(s2, Core::IsAnyOf("\n\r"));
	std::string deb23 = Core::Trim(s3, Core::IsAnyOf("\n\r"));
	std::string deb24x = Core::Trim(s4, Core::IsAnyOf("\n\r"));

	std::string deb24 = Core::TrimLeft(s1, Core::IsAnyOf("\n\r"));
	std::string deb25 = Core::TrimLeft(s2, Core::IsAnyOf("\n\r"));
	std::string deb26 = Core::TrimLeft(s3, Core::IsAnyOf("\n\r"));
	std::string deb28x = Core::TrimLeft(s4, Core::IsAnyOf("\n\r"));

	std::string deb27 = Core::TrimRight(s1, Core::IsAnyOf("\n\r"));
	std::string deb28 = Core::TrimRight(s2, Core::IsAnyOf("\n\r"));
	std::string deb29 = Core::TrimRight(s3, Core::IsAnyOf("\n\r"));
	std::string deb212x = Core::TrimRight(s4, Core::IsAnyOf("\n\r"));

	assert(deb1 == "abc");
	assert(deb2 == "abcabc");
	assert(deb3 == "abcabc");
	assert(deb4x == "abcabc");

	assert(deb4 == "abc");
	assert(deb5 == "abcabc");
	assert(deb6 == "abcabc\f\t  \n\r\f");
	assert(deb8x == "abcabc\f\t  \n\r\f\n");

	assert(deb7 == "abc");
	assert(deb8 == "\n\r\f\t    \n \v \rabcabc");
	assert(deb9 == "\n\r\f\t    \n \v \rabcabc");
	assert(deb12x == "abcabc");

	assert(deb21 == "abc");
	assert(deb22 == "\f\t    \n \v \rabcabc");
	assert(deb23 == "\f\t    \n \v \rabcabc\f\t  \n\r\f");
	assert(deb24x == "abcabc\f\t  \n\r\f");

	assert(deb24 == "abc");
	assert(deb25 == "\f\t    \n \v \rabcabc");
	assert(deb26 == "\f\t    \n \v \rabcabc\f\t  \n\r\f");
	assert(deb28x == "abcabc\f\t  \n\r\f\n");

	assert(deb27 == "abc");
	assert(deb28 == "\n\r\f\t    \n \v \rabcabc");
	assert(deb29 == "\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f");
	assert(deb212x == "abcabc\f\t  \n\r\f");

	assert(Core::Trim("abc") == "abc");
	assert(Core::Trim("\n\r\f\t    \n \v \rabcabc") == "abcabc");
	assert(Core::Trim("\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f") == "abcabc");
	assert(Core::Trim(s2) == "abcabc");

	assert(Core::TrimLeft("abc") == "abc");
	assert(Core::TrimLeft("\n\r\f\t    \n \v \rabcabc") == "abcabc");
	assert(Core::TrimLeft("\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f") == "abcabc\f\t  \n\r\f");
	assert(Core::TrimLeft("abcabc\f\t  \n\r\f\n") == "abcabc\f\t  \n\r\f\n");

	assert(Core::TrimRight("abc") == "abc");
	assert(Core::TrimRight("\n\r\f\t    \n \v \rabcabc") == "\n\r\f\t    \n \v \rabcabc");
	assert(Core::TrimRight("\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f") == "\n\r\f\t    \n \v \rabcabc");
	assert(Core::TrimRight("abcabc\f\t  \n\r\f\n") == "abcabc");

	assert(Core::Trim("abc", Core::IsAnyOf("\n\r")) == "abc");
	assert(Core::Trim("\n\r\f\t    \n \v \rabcabc", Core::IsAnyOf("\n\r")) == "\f\t    \n \v \rabcabc");
	assert(Core::Trim("\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f", Core::IsAnyOf("\n\r")) == "\f\t    \n \v \rabcabc\f\t  \n\r\f");
	assert(Core::Trim("abcabc\f\t  \n\r\f\n", Core::IsAnyOf("\n\r")) == "abcabc\f\t  \n\r\f");

	assert(Core::TrimLeft("abc", Core::IsAnyOf("\n\r")) == "abc");
	assert(Core::TrimLeft("\n\r\f\t    \n \v \rabcabc", Core::IsAnyOf("\n\r")) == "\f\t    \n \v \rabcabc");
	assert(Core::TrimLeft("\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f", Core::IsAnyOf("\n\r")) == "\f\t    \n \v \rabcabc\f\t  \n\r\f");
	assert(Core::TrimLeft("abcabc\f\t  \n\r\f\n", Core::IsAnyOf("\n\r")) == "abcabc\f\t  \n\r\f\n");

	assert(Core::TrimRight("abc", Core::IsAnyOf("\n\r")) == "abc");
	assert(Core::TrimRight("\n\r\f\t    \n \v \rabcabc", Core::IsAnyOf("\n\r")) == "\n\r\f\t    \n \v \rabcabc");
	assert(Core::TrimRight("\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f", Core::IsAnyOf("\n\r")) == "\n\r\f\t    \n \v \rabcabc\f\t  \n\r\f");
	assert(Core::TrimRight("abcabc\f\t  \n\r\f\n", Core::IsAnyOf("\n\r")) == "abcabc\f\t  \n\r\f");

	assert(Core::IsStartingWith("tomi", "tom"));
	assert(!Core::IsStartingWith("tomi", "tox"));
	assert(!Core::IsStartingWith("tomi", "xom"));
	assert(!Core::IsStartingWith("tomi", "txm"));
	assert(!Core::IsStartingWith("tomi", "tomo"));
	assert(!Core::IsStartingWith("tomi", "tomotomo"));
	assert(!Core::IsStartingWith("tomi", "tomitomi"));
	assert(!Core::IsStartingWith("", "tomitomi"));
	assert(Core::IsStartingWith("tomi", empty));
	assert(Core::IsStartingWith(empty, empty));
	assert(!Core::IsStartingWith(empty, "x"));

	assert(Core::IsContaining("aabbtomiaabb", "tomi"));
	assert(!Core::IsContaining("aabbtomaabb", "tomi"));
	assert(Core::IsContaining("tomiaabb", "tomi"));
	assert(Core::IsContaining("aabbtomi", "tomi"));
	assert(Core::IsContaining("tomi", "tomi"));
	assert(Core::IsContaining("", ""));
	assert(!Core::IsContaining("", "a"));
	assert(Core::IsContaining("a", ""));
	assert(Core::IsContaining("aaaa", "aa"));
	assert(!Core::IsContaining("a", "aa"));
	assert(Core::IsContaining("a", "a"));
	assert(!Core::IsContaining("a", "b"));

	assert(Core::Replace("abcdTomiabcdTomiabcd", "tomi", "xxx") == "abcdTomiabcdTomiabcd");
	assert(Core::Replace("abcdTomiabcdTomiabcd", "Tomi", "xxx") == "abcdxxxabcdxxxabcd");
	assert(Core::ReplaceFirst("abcdTomiabcdTomiabcd", "Tomi", "xxx") == "abcdxxxabcdTomiabcd");
	assert(Core::ReplaceFirst("abcdTomiabcdTomiabcd", "tomi", "xxx") == "abcdTomiabcdTomiabcd");
	assert(Core::ReplaceLast("abcdTomiabcdTomiabcd", "Tomi", "xxx") == "abcdTomiabcdxxxabcd");
	assert(Core::ReplaceLast("abcdTomiabcdTomiabcd", "tomi", "xxx") == "abcdTomiabcdTomiabcd");

	assert(Core::Replace("TomiabcdTomiabcd", "tomi", "xxx") == "TomiabcdTomiabcd");
	assert(Core::Replace("TomiabcdTomiabcd", "Tomi", "xxx") == "xxxabcdxxxabcd");
	assert(Core::ReplaceFirst("TomiabcdTomiabcd", "Tomi", "xxx") == "xxxabcdTomiabcd");
	assert(Core::ReplaceFirst("TomiabcdTomiabcd", "tomi", "xxx") == "TomiabcdTomiabcd");
	assert(Core::ReplaceLast("TomiabcdTomiabcd", "Tomi", "xxx") == "Tomiabcdxxxabcd");
	assert(Core::ReplaceLast("TomiabcdTomiabcd", "tomi", "xxx") == "TomiabcdTomiabcd");

	assert(Core::Replace("abcdTomiabcdTomi", "tomi", "xxx") == "abcdTomiabcdTomi");
	assert(Core::Replace("abcdTomiabcdTomi", "Tomi", "xxx") == "abcdxxxabcdxxx");
	assert(Core::ReplaceFirst("abcdTomiabcdTomi", "Tomi", "xxx") == "abcdxxxabcdTomi");
	assert(Core::ReplaceFirst("abcdTomiabcdTomi", "tomi", "xxx") == "abcdTomiabcdTomi");
	assert(Core::ReplaceLast("abcdTomiabcdTomi", "Tomi", "xxx") == "abcdTomiabcdxxx");
	assert(Core::ReplaceLast("abcdTomiabcdTomi", "tomi", "xxx") == "abcdTomiabcdTomi");

	assert(Core::Replace("kTomiabcdTomiabcd", "tomi", "xxx") == "kTomiabcdTomiabcd");
	assert(Core::Replace("kTomiabcdTomiabcd", "Tomi", "xxx") == "kxxxabcdxxxabcd");
	assert(Core::ReplaceFirst("kTomiabcdTomiabcd", "Tomi", "xxx") == "kxxxabcdTomiabcd");
	assert(Core::ReplaceFirst("kTomiabcdTomiabcd", "tomi", "xxx") == "kTomiabcdTomiabcd");
	assert(Core::ReplaceLast("kTomiabcdTomiabcd", "Tomi", "xxx") == "kTomiabcdxxxabcd");
	assert(Core::ReplaceLast("kTomiabcdTomiabcd", "tomi", "xxx") == "kTomiabcdTomiabcd");

	assert(Core::Replace("abcdTomiabcdTomik", "tomi", "xxx") == "abcdTomiabcdTomik");
	assert(Core::Replace("abcdTomiabcdTomik", "Tomi", "xxx") == "abcdxxxabcdxxxk");
	assert(Core::ReplaceFirst("abcdTomiabcdTomik", "Tomi", "xxx") == "abcdxxxabcdTomik");
	assert(Core::ReplaceFirst("abcdTomiabcdTomik", "tomi", "xxx") == "abcdTomiabcdTomik");
	assert(Core::ReplaceLast("abcdTomiabcdTomik", "Tomi", "xxx") == "abcdTomiabcdxxxk");
	assert(Core::ReplaceLast("abcdTomiabcdTomik", "tomi", "xxx") == "abcdTomiabcdTomik");

	assert(Core::Replace("", "a", "a") == "");
	assert(Core::ReplaceFirst("", "a", "a") == "");
	assert(Core::ReplaceLast("", "a", "a") == "");

	assert(Core::Replace("a", "", "b") == "a");
	assert(Core::ReplaceFirst("a", "", "b") == "a");
	assert(Core::ReplaceLast("a", "", "b") == "a");

	assert(Core::Replace("a", "a", "") == "");
	assert(Core::Replace("aaaaa", "a", "") == "");
	assert(Core::ReplaceFirst("a", "a", "") == "");
	assert(Core::ReplaceLast("a", "a", "") == "");

	assert(Core::Replace("aaaaa", "a", "a") == "aaaaa");
	assert(Core::Replace("ababababab", "ab", "ab") == "ababababab");
	assert(Core::Replace("ababababab", "ab", "abab") == "abababababababababab");

	auto res0 = Core::Split("abcdXefghYijkl", true);
	assert(res0.size() == 1);
	assert(res0[0] == "abcdXefghYijkl");
	auto res1 = Core::Split("abcdXefghYijkl", Core::IsAnyOf("XY"), true);
	assert(res1.size() == 3);
	assert(res1[0] == "abcd");
	assert(res1[1] == "efgh");
	assert(res1[2] == "ijkl");
	auto res2 = Core::Split("dXefghYi", Core::IsAnyOf("XY"), true);
	assert(res2.size() == 3);
	assert(res2[0] == "d");
	assert(res2[1] == "efgh");
	assert(res2[2] == "i");
	auto res3 = Core::Split("XefghYijkl", Core::IsAnyOf("XY"), true);
	assert(res3.size() == 2);
	assert(res3[0] == "efgh");
	assert(res3[1] == "ijkl");
	auto res4 = Core::Split("abcdXefghY", Core::IsAnyOf("XY"), true);
	assert(res4.size() == 2);
	assert(res4[0] == "abcd");
	assert(res4[1] == "efgh");
	auto res5 = Core::Split("XXYXYXYXYYXXYY", Core::IsAnyOf("XY"), true);
	assert(res5.size() == 0);
	auto res6 = Core::Split("X", Core::IsAnyOf("XY"), true);
	assert(res6.size() == 0);
	auto res7 = Core::Split("", Core::IsAnyOf("XY"), true);
	assert(res7.size() == 0);
	auto res8 = Core::Split("", Core::IsAnyOf(""), true);
	assert(res8.size() == 0);
	auto res9 = Core::Split("abcdXefghYijkl", Core::IsAnyOf(""), true);
	assert(res9.size() == 1);
	assert(res9[0] == "abcdXefghYijkl");
	auto res10 = Core::Split("XefghY", Core::IsAnyOf("XY"), true);
	assert(res10.size() == 1);
	assert(res10[0] == "efgh");
	auto res11 = Core::Split("XefghY", Core::IsAnyOf("XY"), false);
	assert(res11.size() == 3);
	assert(res11[0] == "");
	assert(res11[1] == "efgh");
	assert(res11[2] == "");
	auto res12 = Core::Split("abcdXefghYijkl", Core::IsAnyOf("XY"), false);
	assert(res12.size() == 3);
	assert(res12[0] == "abcd");
	assert(res12[1] == "efgh");
	assert(res12[2] == "ijkl");
	auto res13 = Core::Split("abcdXXXefghYijkl", Core::IsAnyOf("XY"), false);
	assert(res13.size() == 5);
	assert(res13[0] == "abcd");
	assert(res13[1] == "");
	assert(res13[2] == "");
	assert(res13[3] == "efgh");
	assert(res13[4] == "ijkl");
	auto res14 = Core::Split("abcdXXYYYXXefghYYYYYXXYYijkl", Core::IsAnyOf("XY"), true);
	assert(res14.size() == 3);
	assert(res14[0] == "abcd");
	assert(res14[1] == "efgh");
	assert(res14[2] == "ijkl");

    return 0;
}

