/*
 * Copyright 2010-2021 Tarantool AUTHORS: please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../src/Utils/Common.hpp"
#include "Utils/Helpers.hpp"

#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

void
test_tuple_utils()
{
	TEST_INIT(0);

	{
		using full = std::tuple<int>;
		using cut = tnt::tuple_cut_t<full>;
		using expected = std::tuple<>;
		static_assert(std::is_same_v<cut, expected>);
		static_assert(std::is_same_v<tnt::first_t<full>, int>);
		static_assert(std::is_same_v<tnt::last_t<full>, int>);
	}
	{
		using full = std::tuple<int, float>;
		using cut = tnt::tuple_cut_t<full>;
		using expected = std::tuple<float>;
		static_assert(std::is_same_v<cut, expected>);
		static_assert(std::is_same_v<tnt::first_t<full>, int>);
		static_assert(std::is_same_v<tnt::last_t<full>, float>);
	}
	{
		using full = std::tuple<int, float, double>;
		using cut = tnt::tuple_cut_t<full>;
		using expected = std::tuple<float, double>;
		static_assert(std::is_same_v<cut, expected>);
		static_assert(std::is_same_v<tnt::first_t<full>, int>);
		static_assert(std::is_same_v<tnt::last_t<full>, double>);
	}

	{
		using T = std::tuple<char, uint64_t>;
		static_assert(tnt::tuple_find_size_v<1, T> == 0);
		static_assert(tnt::tuple_find_size_v<8, T> == 1);
	}
	{
		using T = std::tuple<char, uint64_t>;
		static_assert(tnt::tuple_find_size_v<1, T> == 0);
		static_assert(tnt::tuple_find_size_v<8, T> == 1);
		static_assert(tnt::tuple_find_size_v<4, T> == 2);
		static_assert(tnt::tuple_find_size_v<7, T> == 2);
	}
	{
		using T = std::tuple<char, uint64_t, uint32_t>;
		static_assert(tnt::tuple_find_size_v<1, T> == 0);
		static_assert(tnt::tuple_find_size_v<8, T> == 1);
		static_assert(tnt::tuple_find_size_v<4, T> == 2);
		static_assert(tnt::tuple_find_size_v<7, T> == 3);
	}
}

void
test_integer_traits()
{
	TEST_INIT(0);

	enum E1 { V1 = 0, V_BIG = UINT64_MAX };
	enum E2 { V2 = -1 };
	enum E3 : int { V3 = 0 };
	enum class E4 : uint8_t { V4 = 0 };
	struct Test { };

	// base_enum_t
	static_assert(std::is_unsigned_v<tnt::base_enum_t<E1>>);
	static_assert(std::is_signed_v<tnt::base_enum_t<E2>>);
	static_assert(std::is_same_v<int, tnt::base_enum_t<E3>>);
	static_assert(std::is_same_v<uint8_t, tnt::base_enum_t<E4>>);
	static_assert(std::is_unsigned_v<tnt::base_enum_t<const E1>>);
	static_assert(std::is_signed_v<tnt::base_enum_t<const E2>>);
	static_assert(std::is_same_v<int, tnt::base_enum_t<const E3>>);
	static_assert(std::is_same_v<uint8_t, tnt::base_enum_t<const E4>>);
	static_assert(std::is_same_v<int, tnt::base_enum_t<int>>);
	static_assert(std::is_same_v<char, tnt::base_enum_t<char>>);
	static_assert(std::is_same_v<Test, tnt::base_enum_t<Test>>);
	static_assert(std::is_same_v<E1&, tnt::base_enum_t<E1&>>);
	static_assert(std::is_same_v<E2&, tnt::base_enum_t<E2&>>);
	static_assert(std::is_same_v<E3&, tnt::base_enum_t<E3&>>);
	static_assert(std::is_same_v<E4&, tnt::base_enum_t<E4&>>);

	// are integers
	static_assert(tnt::is_integer_v<int>);
	static_assert(tnt::is_integer_v<const int>);
	static_assert(tnt::is_integer_v<char>);
	static_assert(tnt::is_integer_v<signed char>);
	static_assert(tnt::is_integer_v<unsigned char>);
	static_assert(tnt::is_integer_v<E1>);
	static_assert(tnt::is_integer_v<E2>);
	static_assert(tnt::is_integer_v<E3>);
	static_assert(tnt::is_integer_v<E4>);
	static_assert(tnt::is_integer_v<const E1>);
	static_assert(tnt::is_integer_v<const E2>);
	static_assert(tnt::is_integer_v<const E3>);
	static_assert(tnt::is_integer_v<const E4>);

	// are not integers
	static_assert(!tnt::is_integer_v<bool>);
	static_assert(!tnt::is_integer_v<const bool>);
	static_assert(!tnt::is_integer_v<float>);
	static_assert(!tnt::is_integer_v<double>);
	static_assert(!tnt::is_integer_v<int&>);
	static_assert(!tnt::is_integer_v<E1&>);
	static_assert(!tnt::is_integer_v<E2&>);
	static_assert(!tnt::is_integer_v<E3&>);
	static_assert(!tnt::is_integer_v<E4&>);
	static_assert(!tnt::is_integer_v<bool&>);
	static_assert(!tnt::is_integer_v<Test>);
	static_assert(!tnt::is_integer_v<const Test>);
	static_assert(!tnt::is_integer_v<Test&>);

	// is_signed_integer_v..
	static_assert(tnt::is_signed_integer_v<int>);
	static_assert(!tnt::is_signed_integer_v<uint64_t>);
	static_assert(!tnt::is_signed_integer_v<E1>);
	static_assert(tnt::is_signed_integer_v<E2>);
	static_assert(tnt::is_signed_integer_v<E3>);
	static_assert(!tnt::is_signed_integer_v<E4>);

	static_assert(tnt::is_signed_integer_v<const int>);
	static_assert(!tnt::is_signed_integer_v<const uint64_t>);
	static_assert(!tnt::is_signed_integer_v<const E1>);
	static_assert(tnt::is_signed_integer_v<const E2>);
	static_assert(tnt::is_signed_integer_v<const E3>);
	static_assert(!tnt::is_signed_integer_v<const E4>);

	static_assert(!tnt::is_signed_integer_v<Test>);
	static_assert(!tnt::is_signed_integer_v<float>);
	static_assert(!tnt::is_signed_integer_v<const Test>);
	static_assert(!tnt::is_signed_integer_v<const float>);

	// is_unsigned_integer_v..
	static_assert(!tnt::is_unsigned_integer_v<int>);
	static_assert(tnt::is_unsigned_integer_v<uint64_t>);
	static_assert(tnt::is_unsigned_integer_v<E1>);
	static_assert(!tnt::is_unsigned_integer_v<E2>);
	static_assert(!tnt::is_unsigned_integer_v<E3>);
	static_assert(tnt::is_unsigned_integer_v<E4>);

	static_assert(!tnt::is_unsigned_integer_v<const int>);
	static_assert(tnt::is_unsigned_integer_v<const uint64_t>);
	static_assert(tnt::is_unsigned_integer_v<const E1>);
	static_assert(!tnt::is_unsigned_integer_v<const E2>);
	static_assert(!tnt::is_unsigned_integer_v<const E3>);
	static_assert(tnt::is_unsigned_integer_v<const E4>);

	static_assert(!tnt::is_unsigned_integer_v<Test>);
	static_assert(!tnt::is_unsigned_integer_v<float>);
	static_assert(!tnt::is_unsigned_integer_v<const Test>);
	static_assert(!tnt::is_unsigned_integer_v<const float>);
}

void
test_integral_constant_traits()
{
	enum E { V = 1 };
	struct Test { };
	using const_int = std::integral_constant<int, 0>;
	using const_enum = std::integral_constant<E, V>;
	using const_bool = std::integral_constant<bool, false>;

	static_assert(tnt::is_integral_constant_v<const_int>);
	static_assert(tnt::is_integral_constant_v<const const_int>);
	static_assert(tnt::is_integral_constant_v<const_enum>);
	static_assert(tnt::is_integral_constant_v<const const_enum>);
	static_assert(tnt::is_integral_constant_v<const_bool>);
	static_assert(tnt::is_integral_constant_v<const const_bool>);
	static_assert(tnt::is_integral_constant_v<const_int>);
	static_assert(!tnt::is_integral_constant_v<const_int&>);
	static_assert(!tnt::is_integral_constant_v<int>);
	static_assert(!tnt::is_integral_constant_v<const int>);
	static_assert(!tnt::is_integral_constant_v<float>);
	static_assert(!tnt::is_integral_constant_v<Test>);

	static_assert(std::is_same_v<int, tnt::uni_integral_base_t<int>>);
	static_assert(std::is_same_v<int, tnt::uni_integral_base_t<const int>>);
	static_assert(std::is_same_v<E, tnt::uni_integral_base_t<E>>);
	static_assert(std::is_same_v<E, tnt::uni_integral_base_t<const E>>);
	static_assert(std::is_same_v<bool, tnt::uni_integral_base_t<bool>>);
	static_assert(std::is_same_v<bool, tnt::uni_integral_base_t<const bool>>);
	static_assert(std::is_same_v<int, tnt::uni_integral_base_t<const_int>>);
	static_assert(std::is_same_v<int, tnt::uni_integral_base_t<const const_int>>);
	static_assert(std::is_same_v<E, tnt::uni_integral_base_t<const_enum>>);
	static_assert(std::is_same_v<E, tnt::uni_integral_base_t<const const_enum>>);
	static_assert(std::is_same_v<bool, tnt::uni_integral_base_t<const_bool>>);
	static_assert(std::is_same_v<bool, tnt::uni_integral_base_t<const const_bool>>);
	static_assert(std::is_same_v<Test, tnt::uni_integral_base_t<Test>>);
	static_assert(std::is_same_v<Test, tnt::uni_integral_base_t<const Test>>);
	static_assert(std::is_same_v<float, tnt::uni_integral_base_t<float>>);
	static_assert(std::is_same_v<float, tnt::uni_integral_base_t<const float>>);

	static_assert(tnt::uni_value(const_int{}) == 0);
	static_assert(tnt::uni_value(const_enum{}) == V);
	static_assert(tnt::uni_value(const_bool{}) == false);
	fail_unless(tnt::uni_value(1) == 1);
	fail_unless(tnt::uni_value(V) == V);
	fail_unless(tnt::uni_value(true) == true);
	fail_unless(tnt::uni_value(nullptr) == nullptr);
	fail_unless(tnt::uni_value(1.f) == 1.f);
	fail_unless(tnt::uni_value(2.) == 2.);

	static_assert(tnt::is_uni_integral_v<int>);
	static_assert(tnt::is_uni_integral_v<const int>);
	static_assert(!tnt::is_uni_integral_v<int&>);
	static_assert(tnt::is_uni_integral_v<bool>);
	static_assert(tnt::is_uni_integral_v<const bool>);
	static_assert(!tnt::is_uni_integral_v<bool&>);
	static_assert(!tnt::is_uni_integral_v<E>);
	static_assert(!tnt::is_uni_integral_v<const E>);
	static_assert(!tnt::is_uni_integral_v<E&>);
	static_assert(tnt::is_uni_integral_v<const_int>);
	static_assert(tnt::is_uni_integral_v<const const_int>);
	static_assert(!tnt::is_uni_integral_v<const_int&>);
	static_assert(!tnt::is_uni_integral_v<const_enum>);
	static_assert(!tnt::is_uni_integral_v<const const_enum>);
	static_assert(!tnt::is_uni_integral_v<const_enum&>);
	static_assert(tnt::is_uni_integral_v<const_bool>);
	static_assert(tnt::is_uni_integral_v<const const_bool>);
	static_assert(!tnt::is_uni_integral_v<const_bool&>);
	static_assert(!tnt::is_uni_integral_v<Test>);
	static_assert(!tnt::is_uni_integral_v<float>);

	static_assert(tnt::is_uni_integer_v<int>);
	static_assert(tnt::is_uni_integer_v<const int>);
	static_assert(!tnt::is_uni_integer_v<int&>);
	static_assert(!tnt::is_uni_integer_v<bool>);
	static_assert(!tnt::is_uni_integer_v<const bool>);
	static_assert(!tnt::is_uni_integer_v<bool&>);
	static_assert(tnt::is_uni_integer_v<E>);
	static_assert(tnt::is_uni_integer_v<const E>);
	static_assert(!tnt::is_uni_integer_v<E&>);
	static_assert(tnt::is_uni_integer_v<const_int>);
	static_assert(tnt::is_uni_integer_v<const const_int>);
	static_assert(!tnt::is_uni_integer_v<const_int&>);
	static_assert(tnt::is_uni_integer_v<const_enum>);
	static_assert(tnt::is_uni_integer_v<const const_enum>);
	static_assert(!tnt::is_uni_integer_v<const_enum&>);
	static_assert(!tnt::is_uni_integer_v<const_bool>);
	static_assert(!tnt::is_uni_integer_v<const const_bool>);
	static_assert(!tnt::is_uni_integer_v<const_bool&>);
	static_assert(!tnt::is_uni_integer_v<Test>);
	static_assert(!tnt::is_uni_integer_v<float>);

	static_assert(!tnt::is_uni_bool_v<int>);
	static_assert(!tnt::is_uni_bool_v<const int>);
	static_assert(!tnt::is_uni_bool_v<int&>);
	static_assert(tnt::is_uni_bool_v<bool>);
	static_assert(tnt::is_uni_bool_v<const bool>);
	static_assert(!tnt::is_uni_bool_v<bool&>);
	static_assert(!tnt::is_uni_bool_v<E>);
	static_assert(!tnt::is_uni_bool_v<const E>);
	static_assert(!tnt::is_uni_bool_v<E&>);
	static_assert(!tnt::is_uni_bool_v<const_int>);
	static_assert(!tnt::is_uni_bool_v<const const_int>);
	static_assert(!tnt::is_uni_bool_v<const_int&>);
	static_assert(!tnt::is_uni_bool_v<const_enum>);
	static_assert(!tnt::is_uni_bool_v<const const_enum>);
	static_assert(!tnt::is_uni_bool_v<const_enum&>);
	static_assert(tnt::is_uni_bool_v<const_bool>);
	static_assert(tnt::is_uni_bool_v<const const_bool>);
	static_assert(!tnt::is_uni_bool_v<const_bool&>);
	static_assert(!tnt::is_uni_bool_v<Test>);
	static_assert(!tnt::is_uni_bool_v<float>);

	using int1 = std::integral_constant<int, 3>;
	using int2 = tnt::integral_constant<int, 3>;
	static_assert(std::is_same_v<int1, int2>);
}

void
test_array_traits()
{
	// That's a difference between C and std arrays:
	// std::array has its own cv qualifier while C passes it to values.
	static_assert(std::is_same_v<std::add_const_t<int[3]>,
				     const int[3]>);
	static_assert(std::is_same_v<std::add_const_t<std::array<int, 3>>,
				     const std::array<int, 3>>);

	using carr_t = int[10];
	using cv_carr_t = const volatile carr_t;
	using sarr_t = std::array<int, 11>;
	using cv_sarr_t = const volatile sarr_t;
	enum E { V = 1 };
	struct Test { };
	using const_int = std::integral_constant<int, 0>;

	static_assert(tnt::is_c_array_v<carr_t>);
	static_assert(tnt::is_c_array_v<cv_carr_t>);
	static_assert(!tnt::is_c_array_v<sarr_t>);
	static_assert(!tnt::is_c_array_v<cv_sarr_t>);
	static_assert(!tnt::is_c_array_v<int>);
	static_assert(!tnt::is_c_array_v<E>);
	static_assert(!tnt::is_c_array_v<Test>);
	static_assert(!tnt::is_c_array_v<const_int>);

	static_assert(!tnt::is_std_array_v<carr_t>);
	static_assert(!tnt::is_std_array_v<cv_carr_t>);
	static_assert(tnt::is_std_array_v<sarr_t>);
	static_assert(tnt::is_std_array_v<cv_sarr_t>);
	static_assert(!tnt::is_std_array_v<int>);
	static_assert(!tnt::is_std_array_v<E>);
	static_assert(!tnt::is_std_array_v<Test>);
	static_assert(!tnt::is_std_array_v<const_int>);

	static_assert(tnt::is_any_array_v<carr_t>);
	static_assert(tnt::is_any_array_v<cv_carr_t>);
	static_assert(tnt::is_any_array_v<sarr_t>);
	static_assert(tnt::is_any_array_v<cv_sarr_t>);
	static_assert(!tnt::is_any_array_v<int>);
	static_assert(!tnt::is_any_array_v<E>);
	static_assert(!tnt::is_any_array_v<Test>);
	static_assert(!tnt::is_any_array_v<const_int>);

	static_assert(10 == tnt::any_extent_v<carr_t>);
	static_assert(10 == tnt::any_extent_v<cv_carr_t>);
	static_assert(11 == tnt::any_extent_v<sarr_t>);
	static_assert(11 == tnt::any_extent_v<cv_sarr_t>);
	static_assert(0 == tnt::any_extent_v<int>);
	static_assert(0 == tnt::any_extent_v<E>);
	static_assert(0 == tnt::any_extent_v<Test>);
	static_assert(0 == tnt::any_extent_v<const_int>);

	static_assert(std::is_same_v<int, tnt::remove_any_extent_t<carr_t>>);
	static_assert(std::is_same_v<const volatile int, tnt::remove_any_extent_t<cv_carr_t>>);
	static_assert(std::is_same_v<int, tnt::remove_any_extent_t<sarr_t>>);
	static_assert(std::is_same_v<int, tnt::remove_any_extent_t<cv_sarr_t>>);
	static_assert(std::is_same_v<int, tnt::remove_any_extent_t<int>>);
	static_assert(std::is_same_v<E, tnt::remove_any_extent_t<E>>);
	static_assert(std::is_same_v<Test, tnt::remove_any_extent_t<Test>>);
	static_assert(std::is_same_v<const int, tnt::remove_any_extent_t<const int>>);

	static_assert(tnt::is_char_ptr_v<char *>);
	static_assert(tnt::is_char_ptr_v<const volatile char *>);
	static_assert(tnt::is_char_ptr_v<volatile char *const>);
	static_assert(!tnt::is_char_ptr_v<signed char *>);
	static_assert(!tnt::is_char_ptr_v<unsigned char *>);
	static_assert(!tnt::is_char_ptr_v<char>);
	static_assert(!tnt::is_char_ptr_v<int *>);
	static_assert(!tnt::is_char_ptr_v<char [10]>);
	static_assert(!tnt::is_char_ptr_v<std::array<char, 10>>);
	static_assert(!tnt::is_char_ptr_v<int>);
	static_assert(!tnt::is_char_ptr_v<E>);
	static_assert(!tnt::is_char_ptr_v<Test>);
}

void
test_misc_traits()
{
	enum E { V = 1 };
	struct Test { };
	using const_int = std::integral_constant<int, 0>;

	static_assert(tnt::is_ignore_v<decltype(std::ignore)>);
	static_assert(tnt::is_ignore_v<const decltype(std::ignore)>);
	static_assert(!tnt::is_ignore_v<E>);
	static_assert(!tnt::is_ignore_v<Test>);
	static_assert(!tnt::is_ignore_v<const_int>);
	static_assert(!tnt::is_ignore_v<nullptr_t>);
	static_assert(!tnt::is_ignore_v<void *>);
	static_assert(!tnt::is_ignore_v<bool>);

	static_assert(tnt::is_variant_v<std::variant<int>>);
	static_assert(tnt::is_variant_v<std::variant<int, float>>);
	static_assert(tnt::is_variant_v<const std::variant<int>>);
	static_assert(tnt::is_variant_v<volatile std::variant<int>>);
	static_assert(!tnt::is_variant_v<std::variant<int>&>);
	static_assert(!tnt::is_variant_v<std::tuple<int, float>>);
	static_assert(!tnt::is_variant_v<std::optional<int>>);
	static_assert(!tnt::is_variant_v<Test>);
	static_assert(!tnt::is_variant_v<E>);
	static_assert(!tnt::is_variant_v<const_int>);

	static_assert(tnt::is_optional_v<std::optional<int>>);
	static_assert(tnt::is_optional_v<const std::optional<int>>);
	static_assert(tnt::is_optional_v<volatile std::optional<int>>);
	static_assert(!tnt::is_optional_v<std::optional<int>&>);
	static_assert(!tnt::is_optional_v<std::variant<int>>);
	static_assert(!tnt::is_optional_v<std::tuple<int, float>>);
	static_assert(!tnt::is_optional_v<std::variant<int>&>);
	static_assert(!tnt::is_optional_v<Test>);
	static_assert(!tnt::is_optional_v<E>);
	static_assert(!tnt::is_optional_v<const_int>);
}

void
test_tuple_pair_traits()
{
	enum E { V = 1 };
	struct Test { };
	using const_int = std::integral_constant<int, 0>;

	static_assert(tnt::is_tuple_v<std::tuple<int>>);
	static_assert(tnt::is_tuple_v<std::tuple<int, int>>);
	static_assert(tnt::is_tuple_v<std::tuple<int, float, Test, E>>);
	static_assert(tnt::is_tuple_v<const std::tuple<int, int>>);
	static_assert(tnt::is_tuple_v<volatile std::tuple<int, int>>);
	static_assert(tnt::is_tuple_v<std::tuple<std::tuple<>>>);
	static_assert(!tnt::is_tuple_v<std::pair<int, float>>);
	static_assert(!tnt::is_tuple_v<Test>);
	static_assert(!tnt::is_tuple_v<int>);
	static_assert(!tnt::is_tuple_v<E>);
	static_assert(!tnt::is_tuple_v<Test>);
	static_assert(!tnt::is_tuple_v<const_int>);

	static_assert(tnt::is_pair_v<std::pair<int, float>>);
	static_assert(tnt::is_pair_v<const std::pair<int, float>>);
	static_assert(tnt::is_pair_v<volatile std::pair<int, float>>);
	static_assert(!tnt::is_pair_v<std::pair<int, float>&>);
	static_assert(!tnt::is_pair_v<std::tuple<int, int>>);
	static_assert(!tnt::is_pair_v<Test>);
	static_assert(!tnt::is_pair_v<int>);
	static_assert(!tnt::is_pair_v<E>);
	static_assert(!tnt::is_pair_v<Test>);
	static_assert(!tnt::is_pair_v<const_int>);
}

void
test_member_traits()
{
	struct Test { int i; const int ci; int f(); };
	using member_t = decltype(&Test::i);
	using cmember_t = decltype(&Test::ci);
	using method_t = decltype(&Test::f);
	enum E { V = 1 };
	using const_int = std::integral_constant<int, 0>;

	static_assert(tnt::is_member_ptr_v<member_t>);
	static_assert(tnt::is_member_ptr_v<const member_t>);
	static_assert(tnt::is_member_ptr_v<volatile member_t>);
	static_assert(tnt::is_member_ptr_v<cmember_t>);
	static_assert(tnt::is_member_ptr_v<const cmember_t>);
	static_assert(!tnt::is_member_ptr_v<method_t>);
	static_assert(!tnt::is_member_ptr_v<int>);
	static_assert(!tnt::is_member_ptr_v<const int>);
	static_assert(!tnt::is_member_ptr_v<E>);
	static_assert(!tnt::is_member_ptr_v<const_int>);

	static_assert(std::is_same_v<int, tnt::demember_t<member_t>>);
	static_assert(std::is_same_v<int, tnt::demember_t<const member_t>>);
	static_assert(std::is_same_v<int, tnt::demember_t<volatile member_t>>);
	static_assert(std::is_same_v<const int, tnt::demember_t<cmember_t>>);
	static_assert(std::is_same_v<const int, tnt::demember_t<const cmember_t>>);
	static_assert(std::is_same_v<method_t, tnt::demember_t<method_t>>);
	static_assert(std::is_same_v<int, tnt::demember_t<int>>);
	static_assert(std::is_same_v<const int, tnt::demember_t<const int>>);
	static_assert(std::is_same_v<E, tnt::demember_t<E>>);
	static_assert(std::is_same_v<const_int, tnt::demember_t<const_int>>);
}

struct BrandNewArray {
	int *begin() noexcept;
	int *end() noexcept;
	const int *begin() const noexcept;
	const int *end() const noexcept;
	int *data() noexcept;
	const int *data() const noexcept;
	int size() const noexcept;
	static constexpr size_t static_capacity = 10;
};

struct BrandNewSet {
	int *begin() noexcept;
	int *end() noexcept;
	const int *begin() const noexcept;
	const int *end() const noexcept;
	int size() const noexcept;
};

struct BrandNewMap {
	std::pair<int, int> *begin() noexcept;
	std::pair<int, int> *end() noexcept;
	const std::pair<int, int> *begin() const noexcept;
	const std::pair<int, int> *end() const noexcept;
	int size() const noexcept;
};

template <class CONT, bool SIZABLE, bool CONTIGUOUS, bool ITERABLE, bool PAIR_ITERABLE>
void check_cont()
{
	static_assert(tnt::is_sizable_v<CONT> == SIZABLE);
	static_assert(tnt::is_contiguous_v<CONT> == CONTIGUOUS);
	static_assert(tnt::is_const_iterable_v<CONT> == ITERABLE);
	static_assert(tnt::is_const_pairs_iterable_v<CONT> == PAIR_ITERABLE);
}

void
test_container_traits()
{
	check_cont<std::vector<int>, true, true, true, false>();
	check_cont<const std::vector<int>, true, true, true, false>();
	check_cont<const std::vector<int>&, true, true, true, false>();
	check_cont<std::vector<bool>, true, false, true, false>();
	check_cont<std::vector<std::pair<int, int>>, true, true, true, true>();

	check_cont<std::array<int, 5>, true, true, true, false>();
	check_cont<std::list<int>, true, false, true, false>();
	check_cont<std::forward_list<int>, false, false, true, false>();
	check_cont<std::deque<int>, true, false, true, false>();
	check_cont<std::set<int>, true, false, true, false>();
	check_cont<std::unordered_set<int>, true, false, true, false>();
	check_cont<std::map<int, int>, true, false, true, true>();
	check_cont<std::unordered_map<int, int>, true, false, true, true>();

	check_cont<BrandNewArray, true, true, true, false>();
	check_cont<BrandNewSet, true, false, true, false>();
	check_cont<BrandNewMap, true, false, true, true>();

	static_assert(tnt::is_limited_v<BrandNewArray>);
	static_assert(tnt::is_limited_v<const BrandNewArray>);
	static_assert(!tnt::is_limited_v<std::vector<int>>);
	static_assert(!tnt::is_limited_v<std::set<int>>);
}

MAKE_IS_METHOD_CALLABLE_CHECKER(set);

struct A1 { void set(int, float); };
struct A2 { void set(float, float); };
struct A3 { void set(int&, float&); };
struct A4 { void set(const int&, const float&); };
struct A5 { void get(int, float); };
struct A6 { template <class T, class U> void set(T, U); };
template <class T>
struct A7 { template <class U> void set(T, U); };

void
test_is_method_callable()
{
	static_assert(is_set_callable_v<A1, int, float>);
	static_assert(!is_set_callable_v<A1, int, void*>);
	static_assert(!is_set_callable_v<A1, int, float, float>);
	static_assert(is_set_callable_v<A1, int&, float&>);
	static_assert(is_set_callable_v<A2, int, float>); // convert int to float.
	static_assert(is_set_callable_v<A2, int&, float&>);
	static_assert(is_set_callable_v<A3, int&, float&>);
	static_assert(!is_set_callable_v<A3, int, float>); // can't convert rvalue to reference.
	static_assert(is_set_callable_v<A4, int&, float&>);
	static_assert(is_set_callable_v<A4, int, float>); // can convert rvalue to const reference.
	static_assert(!is_set_callable_v<A5, int, float>); // no method.
	static_assert(is_set_callable_v<A6, int, float>);
	static_assert(!is_set_callable_v<A6, int, float, int>); // too many args.
	static_assert(is_set_callable_v<A7<int>, int, float>);
	static_assert(!is_set_callable_v<A7<int*>, int, float>); // can't convert.
}

int main()
{
	static_assert(tnt::always_false_v<double> == false);
	test_tuple_utils();
	test_integer_traits();
	test_integral_constant_traits();
	test_array_traits();
	test_misc_traits();
	test_tuple_pair_traits();
	test_member_traits();
	test_container_traits();
	test_is_method_callable();
}
