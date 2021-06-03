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

#include <cassert>
#include <tuple>

namespace tnt {
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

} // namespace mpp {
