#pragma once
/*
 * Copyright 2010-2020, Tarantool AUTHORS, please see AUTHORS file.
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
#include <cstdint>
#include <cstring>
#include <tuple>

namespace mpp {
/**
 * Delayer of static_assert evaluation.
 */
template <class>
constexpr bool always_false_v = false;

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
template<class TUPLE>
using first_t = std::tuple_element_t<0, TUPLE>;

template<class TUPLE>
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
 * All standard uint and in types.
 */
using uint_types = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;
using int_types = std::tuple<int8_t, int16_t, int32_t, int64_t>;

/**
 * Getter of unsigned integer type with the size as given type.
 */
template <class T>
using under_uint_t = std::tuple_element_t<tuple_find_size_v<sizeof(T), uint_types>, uint_types>;

/**
 * Getter of signed integer type with the size as given type.
 */
template <class T>
using under_int_t = std::tuple_element_t<tuple_find_size_v<sizeof(T), int_types>, int_types>;

/**
 * bswap overloads.
 */
inline uint8_t  bswap(uint8_t x)  { return x; }
inline uint16_t bswap(uint16_t x) { return __builtin_bswap16(x); }
inline uint32_t bswap(uint32_t x) { return __builtin_bswap32(x); }
inline uint64_t bswap(uint64_t x) { return __builtin_bswap64(x); }

/**
 * msgpack encode bswap: convert any type to uint and bswap it.
 */
template <class T>
under_uint_t<T> bswap(T t)
{
	static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
	under_uint_t<T> tmp;
	memcpy(&tmp, &t, sizeof(T));
	return bswap(tmp);
}

/**
 * msgpack decode bswap: bswap give int and convert it to any type.
 */
template <class T>
T bswap(under_uint_t<T> t)
{
	static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
	t = bswap(t);
	T tmp;
	memcpy(&tmp, &t, sizeof(T));
	return tmp;
}

/**
 * Transform rvalue reference to const value. Don't transform otherwise.
 */
template <class T>
using save_or_ref = std::conditional_t<std::is_rvalue_reference_v<T&&>,
				       const std::remove_reference_t<T>, T>;

/**
 * Standard unreachable.
 */
[[noreturn]] inline void unreachable() { assert(false); __builtin_unreachable(); }

} // namespace mpp {
