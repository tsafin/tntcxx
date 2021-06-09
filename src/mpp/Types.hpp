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

#include "../Utils/CStr.hpp"
#include "../Utils/ItrRange.hpp"
#include "Constants.hpp"

namespace mpp {

using std::integral_constant;
using tnt::CStr;

/**
 * Specificator is a wrapper around some objects, usually holds a constant
 * reference on that object and describes how that object must be handled
 * but msgpack codec.
 * For example std::tuple is packed as array by default, if you want it to
 * be packed as map, wrap it with mpp::as_map(<that tuple>).
 * Because of holding reference to an object a user must think about original
 * object's lifetime. The best practice is to use temporary specificator
 * objects, like encoder.add(mpp::as_map(<that tuple>)).
 *
 * List of type specificators in this header (only one is allowed):
 * as_str - treat a value as string.
 * as_bin - treat a value as binary data.
 * as_arr - treat a value as msgpack array.
 * as_map - treat a value as msgpack map.
 * as_ext - treat a value as msgpack ext of given type.
 * as_raw - write already packed msgpack object as raw data.
 * reserve - write nothing of given size.
 *
 * List of optional specificators in this header:
 * track - additionally save beginning and end positions of the written object.
 * as_fixed - fix underlying type of msgpack object.
 *
 * List of terminal types:
 * MPP_AS_CONST - compile-time constant value (except strings).
 * MPP_AS_CONSTR - compile-time constant string value.
 */

/**
 * Convert rvalue references and member pointers to rvalues.
 * Keep the original type other.
 */
template <class T>
using wrapped_t =
	std::conditional_t<std::is_rvalue_reference_v<T> ||
			   tnt::is_member_ptr_v<std::remove_reference_t<T>>,
			   std::remove_cv_t<std::remove_reference_t<T>>, T>;

/**
 * A group of specificators - as_str(..), as_bin(..), as_arr(..), as_map(..).
 * They create CommonWrapper with appropriate properties.
 * A wrapper takes a container and specify explicitly how it must be
 * packed/unpacked as msgpack object.
 * A bit outstanding is 'as_raw' - it means that the data passed is expected
 * to be a valid msgpack object and must be just copied to the stream.
 * Another outstanding wrapper is 'reserve' - it means some number of bytes
 * must be skipped * (not written) in msgpack stream..
 */
template <compact::Type TYPE, class T>
struct CommonWrapper {
	static constexpr compact::Type type = TYPE;
	T value;
};

template <compact::Type TYPE, class T>
auto as_simple(T&& t)
{
	return CommonWrapper<TYPE, wrapped_t<T>>{std::forward<T>(t)};
}

template <class T>
auto as_str(T&& t)
{
	return as_simple<compact::MP_STR>(std::forward<T>(t));
}

template <class T>
auto as_bin(T&& t)
{
	return as_simple<compact::MP_BIN>(std::forward<T>(t));
}

template <class T>
auto as_arr(T&& t)
{
	return as_simple<compact::MP_ARR>(std::forward<T>(t));
}

template <class T>
auto as_map(T&& t)
{
	return as_simple<compact::MP_MAP>(std::forward<T>(t));
}

/**
 * Specificator - as_ext(..). Creates a wrapper ExtWrapper that holds ext type
 * and a container (or range) and specifies that the data must packed/unpacked
 * as MP_EXIT msgpack object.
 */
template <class EXT_T, class T>
struct ExtWrapper {
	static constexpr compact::Type type = compact::MP_EXT;
	EXT_T ext_type;
	T value;
};

template <class EXT_T, class T>
auto as_ext(EXT_T&& ext_type, T&& t)
{
	using WEXT_T = wrapped_t<EXT_T>;
	using WT = wrapped_t<T>;
	return ExtWrapper<WEXT_T, WT>{std::forward<EXT_T>(ext_type),
				      std::forward<T>(t)};
}

/**
 * Specificator - track(..). Creates a wrapper TrackWrapper that holds a value
 * and a range - a pair of iterators. The first iterator will be set to the
 * beginning of msgpack object, the second - at the end of it.
*/
template <class T, class RANGE>
struct TrackWrapper {
	const T value;
	RANGE range;
};

template <class T, class RANGE>
TrackWrapper<T, RANGE> track(T&& t, RANGE&& r)
{
	using RR_RANGE = std::remove_reference_t<RANGE>;
	static_assert(tnt::is_itr_range_v<tnt::demember_t<RR_RANGE>>);
	using
	return TrackWrapper<wrapped_t<T>, wrapped_t<RANGE>>{std
	 {t, r};
}

/**
 * Reserve is an object that specifies that some number of bytes must be skipped
 * (not written) in msgpack stream.
 * Should be created by reserve<N>() and reserve(N).
 * There are also reserve<N>(range) and reserve(N, range) specificators,
 * that are synonyms for track(reserve<N>, range) and track(reserve(N), range).
 */
template <size_t N>
struct Reserve {
	static constexpr bool is_const_size_v = true;
	static constexpr size_t value = N;
};

template <>
struct Reserve<0> {
	static constexpr bool is_const_size_v = false;
	size_t value;
};

template <size_t N>
Reserve<N> reserve() { return {}; }

inline Reserve<0> reserve(size_t n) { return {n}; }

template <size_t N, class RANGE>
track_holder<Reserve<N>, RANGE> reserve(RANGE& r) { return {{}, r}; }

template <class RANGE>
track_holder<Reserve<0>, RANGE> reserve(size_t n, RANGE& r) { return {{n}, r}; }

/**
 * Specificator - is_fixed(..). Creates a wrapper fixed_holder that holds
 * a value and a definite underlying type by which the value must be written
 * to msgpack stream.
 * For example: as_fixed<uint8_t>(1) will be packed as "0xcc0x01",
 * as_fixed<uint64_t>(1) will be packed as "0xcf0x00x00x00x00x00x00x000x01",
 * as_fixed<void>(1) will be packed as "0x01".
 * By default the type is determined by the type of given value, but it also
 * may be specified explicitly.
 * The 'void' type means that a value must be one-byte packed into msgpack tag.
 */
template <class T, class U>
struct fixed_holder {
	using type = T;
	using hold_type = U;
	const U& value;
};

template <class T, class U>
fixed_holder<T, U> as_fixed(const U& u) { return {u}; }

template <class T>
fixed_holder<T, T> as_fixed(const T& t) { return {t}; }

/**
 * Constants are types that have a constant value enclosed in type itself,
 * as some constexpr static member of the class.
 * For the most of constants std::integral_constant works perfectly.
 * MPP_AS_CONST is just a short form of creating an integral_constant.
 * There' some complexity in creating string constants. There's a special class
 * for them - CStr, that could be instantiated in a pair of ways.
 * MPP_AS_CONSTR is just a macro that instantiates on one of those forms.
 * There are also 'as_const' and 'as_constr' macros that are disabled by
 * default.
 */
#ifndef MPP_DISABLE_AS_CONST_MACRO
#define MPP_AS_CONST(x) std::integral_constant<decltype(x), x>{}
#endif
#ifndef TNT_DISABLE_STR_MACRO
#define MPP_AS_CONSTR(x) TNT_CON_STR(x)
#else
#ifndef TNT_DISABLE_STR_LITERAL
#define MPP_AS_CONSTR(x) x##_cs
#endif
#endif

#ifdef MPP_USE_SHORT_CONST_MACROS
#define as_const(x) std::integral_constant<decltype(x), x>{}
#define as_constr(x) MPP_AS_CONSTR(x)
#endif // #ifdef MPP_USE_SHORT_CONST_MACROS

}; // namespace mpp {

namespace tnt {
using mpp::subst;
}