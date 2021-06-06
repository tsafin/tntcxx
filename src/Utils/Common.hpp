#pragma once
/*
 * Copyright 2010-2021, Tarantool AUTHORS, please see AUTHORS file.
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
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <array>
#include <cassert>
#include <tuple>
#include <type_traits>

namespace tnt {
/**
 * For unification borrow std::integral constant.
 */
template <class T, T V>
using integral_constant = std::integral_constant<T, V>;

/**
 * Delayer of static_assert evaluation.
 */
template <class>
constexpr bool always_false_v = false;

/**
 * Standard unreachable.
 */
[[noreturn]] inline void unreachable() { assert(false); __builtin_unreachable(); }

/**
 * Remove the first element from tuple
 */
template <class T>
struct tuple_cut { static_assert(always_false_v<T>, "Wrong usage!"); };

template <class T, class... U>
struct tuple_cut<std::tuple<T, U...>> { using type = std::tuple<U...>; };

template <class T>
using tuple_cut_t = typename tuple_cut<T>::type;

/**
 * First and last types of a tuple.
 */
template <class TUPLE>
using first_t = std::tuple_element_t<0, TUPLE>;

template <class TUPLE>
using last_t = std::tuple_element_t<std::tuple_size_v<TUPLE> - 1, TUPLE>;

/**
 * Find an index in tuple of a type which has given size.
 */
template <size_t S, class TUPLE>
struct tuple_find_size;

template <size_t S>
struct tuple_find_size<S, std::tuple<>> { static constexpr size_t value = 0; };

template <size_t S, class TUPLE>
struct tuple_find_size {
	static constexpr size_t value =
		sizeof(std::tuple_element_t<0, TUPLE>) == S ?
		0 : 1 + tuple_find_size<S, tuple_cut_t<TUPLE>>::value;
};

template <size_t S, class TUPLE>
constexpr size_t tuple_find_size_v = tuple_find_size<S, TUPLE>::value;

/**
 * All standard uint and int types.
 */
using uint_types = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;
using int_types = std::tuple<int8_t, int16_t, int32_t, int64_t>;

/* Traits */

/**
 * Safe underlying_type extractor by enum type.
 * Unlike std::underlying_type_t which is (can be) undefined for non-enum
 * type, the checker below reveals underlying type for enums and leaves the
 * type for the rest types.
 */
namespace details {
template <class T, bool IS_ENUM> struct base_enum_h;
template <class T> struct base_enum_h<T, false> { using type = T; };
template <class T> struct base_enum_h<T, true> {
	using type = std::underlying_type_t<T>;
};
} // namespace details {

template <class T> using base_enum_t =
	typename details::base_enum_h<T, std::is_enum_v<T>>::type;

/**
 * is_integer_v is true for integral types (see std::is_integral) except bool
 * and for enum types.
 */
template <class T>
constexpr bool is_integer_v = std::is_enum_v<T> ||
	(std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>);

/**
 * Trait whether is_integer_v (see above) which is signed.
 */
template <class T>
constexpr bool is_signed_integer_v =
	is_integer_v<T> && std::is_signed_v<base_enum_t<T>>;

/**
 * Trait whether is_integer_v (see above) which is unsigned.
 */
template <class T>
constexpr bool is_unsigned_integer_v =
	is_integer_v<T> && std::is_unsigned_v<base_enum_t<T>>;

/**
 * Traits to detect and unwrap std::integral constant. See below.
 */
namespace details {
template <class T>
struct uni_integral_traits {
	static constexpr bool is_const = false;
	using base_type = T;
};
template <class T, class U, U V>
struct uni_integral_traits<std::integral_constant<T, V>> {
	static constexpr bool is_const = true;
	using base_type = T;
};
} // namespace details {

/**
 * Check whether the type is std::integral_constant.
 */
template <class T>
constexpr bool is_integral_constant_v =
	details::uni_integral_traits<std::remove_cv_t<T>>::is_const;

/**
 * Safe value_type extractor by std::integral_constant type.
 * It is std::integral_constant::value_type if it as integral_constant,
 * or given type without (!) cv qualifiers otherwise.
 */
template <class T>
using uni_integral_base_t =
	typename details::uni_integral_traits<std::remove_cv_t<T>>::base_type;

/**
 * Universal value extractor. Returns static value member for
 * std::integral_constant, or the value itself otherwise.
 */
template <class T>
constexpr uni_integral_base_t<T> uni_value([[maybe_unused]] T value)
{
	if constexpr (is_integral_constant_v<T>)
		return T::value;
	else
		return value;
}

/**
 * Check whether the type is universal integral, that is either integral
 * or integral_constant with integral base.
 */
template <class T>
constexpr bool is_uni_integral_v = std::is_integral_v<uni_integral_base_t<T>>;

/**
 * Check whether the type is universal bool, that is either bool
 * or integral_constant with bool base.
 */
template <class T>
constexpr bool is_uni_bool_v = std::is_same_v<bool, uni_integral_base_t<T>>;

/**
 * Check whether the type is universal integer, that is either integer
 * or integral_constant with integer base. See is_integer_v for
 * integer definition.
 */
template <class T>
constexpr bool is_uni_integer_v = is_integer_v<uni_integral_base_t<T>>;

/**
 * Check whether the type is C bounded array (with certain size), like int [10].
 * Identical to std::is_bounded_array_v from C++20.
 * Note that cv qualifiers for C array are passed to its elements,
 */
namespace details {
template <class T>
struct is_c_array_h : std::false_type {};
template <class T, std::size_t N>
struct is_c_array_h<T[N]> : std::true_type {};
} //namespace details {
template <class T>
constexpr bool is_c_array_v = details::is_c_array_h<T>::value;

/**
 * Check whether the type is std::array, like std::array<int, 10>.
 */
namespace details {
template <class T>
struct is_std_array_h : std::false_type {};
template <class T, std::size_t N>
struct is_std_array_h<std::array<T, N>> : std::true_type {};
} //namespace details {

template <class T>
constexpr bool is_std_array_v =
	details::is_std_array_h<std::remove_cv_t<T>>::value;

/**
 * Check whether the type is bounded C array or std::array.
 */
template <class T>
constexpr bool is_any_array_v = is_c_array_v<T> || is_std_array_v<T>;

/**
 * Safe extent getter. Gets extent of C or std array, returns 0 for other types.
 */
namespace details {
template <class T>
struct any_extent_h { static constexpr size_t size = 0; };
template <class T, std::size_t N>
struct any_extent_h<T[N]> { static constexpr size_t size = N; };
template <class T, std::size_t N>
struct any_extent_h <std::array<T, N>> { static constexpr size_t size = N; };
} //namespace details {

template <class T>
constexpr size_t any_extent_v =
	details::any_extent_h<std::remove_cv_t<T>>::size;

/**
 * Remove one extent from C or std array.
 */
namespace details {
template <class T>
struct remove_std_extent_h { using type = T; };
template <class T, std::size_t N>
struct remove_std_extent_h<std::array<T, N>> { using type = T; };
} //namespace details {

template <class T>
using remove_any_extent_t = std::conditional_t<tnt::is_std_array_v<T>,
	typename details::remove_std_extent_h<std::remove_cv_t<T>>::type,
	std::remove_extent_t<T>>;

/**
 * Check whether the type is (cv) pointer to (cv) char.
 */
template <class T>
constexpr bool is_char_ptr_v = std::is_pointer_v<T> &&
	std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>>;

/**
 * Check whether the type is the type of std::ignore.
 */
template <class T>
constexpr bool 	is_ignore_v =
	std::is_same_v<std::remove_cv<decltype(std::ignore)>,
		       std::remove_cv<T>>;

} // namespace mpp {
