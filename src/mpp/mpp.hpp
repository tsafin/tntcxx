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
#include <iostream> // TODO - make output to iostream optional?
#include <tuple>
#include <type_traits>
#include <variant>

namespace mpp {

// TODO: move to mpp_dec.hpp ?
enum Types : uint8_t {
	MP_NIL,
	MP_UINT,
	MP_INT,
	MP_STR,
	MP_BIN,
	MP_ARR,
	MP_MAP,
	MP_BOOL,
	MP_FLT,
	MP_DBL,
	MP_EXT,
	MP_END
};

inline const char *TypeNames[] = {
	"MP_NIL",
	"MP_UINT",
	"MP_INT",
	"MP_STR",
	"MP_BIN",
	"MP_ARR",
	"MP_MAP",
	"MP_BOOL",
	"MP_FLT",
	"MP_DBL",
	"MP_EXT",
	"MP_BAD" // note that "MP_BAD" stands for MP_END.
};

std::ostream& operator<<(std::ostream& strm, Types t) {
	if (t >= Types::MP_END)
		return strm << TypeNames[Types::MP_END]
			    << "(" << static_cast<uint32_t>(t) << ")";
	return strm << TypeNames[t];
}

// TODO: move to mpp_utils.hpp ?
#if 0
uint8_t  bswap8(uint8_t x)   { return x; }
uint16_t bswap16(uint16_t x) { return __builtin_bswap16(x); }
uint32_t bswap32(uint32_t x) { return __builtin_bswap32(x); }
uint64_t bswap64(uint64_t x) { return __builtin_bswap64(x); }
#endif

uint8_t  bswap(uint8_t x)  { return x; }
uint16_t bswap(uint16_t x) { return __builtin_bswap16(x); }
uint32_t bswap(uint32_t x) { return __builtin_bswap32(x); }
uint64_t bswap(uint64_t x) { return __builtin_bswap64(x); }

template <class T, class _ = void>
struct under_uint {  };

template <class T>
struct under_uint<T, std::enable_if_t<sizeof(T) == 1, void>> { using type = uint8_t; };
template <class T>
struct under_uint<T, std::enable_if_t<sizeof(T) == 2, void>> { using type = uint16_t; };
template <class T>
struct under_uint<T, std::enable_if_t<sizeof(T) == 4, void>> { using type = uint32_t; };
template <class T>
struct under_uint<T, std::enable_if_t<sizeof(T) == 8, void>> { using type = uint64_t; };

template <class T>
using under_uint_t = typename under_uint<T>::type;

#define DEFINE_WRAPPER(name) \
template <class T> \
struct name { \
	using type = T; \
	const T& value; \
	explicit name(const T& arg) : value(arg) {} \
}; \
\
template <class T> \
struct name<T> as_##name(const T& t) { return name<T>{t}; } \
\
template <class T> \
struct is_##name : std::false_type {}; \
\
template <class T> \
struct is_##name<name<T>> : std::true_type {}; \
\
template <class T> \
constexpr bool is_##name##_v = is_##name<T>::value

DEFINE_WRAPPER(str);
DEFINE_WRAPPER(bin);
DEFINE_WRAPPER(arr);
DEFINE_WRAPPER(map);

#undef DEFINE_WRAPPER

[[noreturn]] void unreachable() { assert(false); }

// TODO: move to mpp_utils.hpp or mpp_traits?
template <class ...ARGS>
using valid_types = void;

// TODO: use standard type from C++20.
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class T>
using is_cvref_char = std::is_same<char, remove_cvref_t<T>>;

template <class T>
constexpr bool is_cvref_char_v = is_cvref_char<T>::value;

template <class T>
struct is_tuple : std::false_type {};

template <class... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

template <class T>
constexpr bool is_tuple_v = is_tuple<T>::value;


template <class T, class _ = void>
struct looks_like_arr : std::false_type {};

template <class T>
struct looks_like_arr<
	T,
	valid_types<
		decltype(*std::cbegin(std::declval<T>())),
		decltype(*std::cend(std::declval<T>())),
		decltype(std::size(std::declval<T>()))
	>
> : public std::true_type {};

template <class T>
constexpr bool looks_like_arr_v = looks_like_arr<T>::value;


// Note that everything that looks like map also looks like array.
template <class T, class _ = void>
struct looks_like_map : std::false_type {};

template <class T>
struct looks_like_map<
	T,
	valid_types<
		decltype(std::cbegin(std::declval<T>())->first),
		decltype(std::cbegin(std::declval<T>())->second),
		decltype(std::cend(std::declval<T>())),
		decltype(std::size(std::declval<T>()))
	>
> : public std::true_type {};

template <class T>
constexpr bool looks_like_map_v = looks_like_map<T>::value;


// Note that everything that looks like string possibly also looks like array.
template <class T, class _ = void>
struct looks_like_str : std::false_type {};

template <class T>
struct looks_like_str<
	T,
	valid_types<
		decltype(*std::data(std::declval<T>())),
		decltype(std::size(std::declval<T>()))
	>
> : public is_cvref_char<decltype(*std::data(std::declval<T>()))> {};

template <class T>
constexpr bool looks_like_str_v = looks_like_str<T>::value;

// Actually checks that T is a char array.
template <class T>
struct is_char_arr : std::false_type {};

template <size_t N>
struct is_char_arr<char [N]> : std::true_type {};

template <class T>
constexpr bool is_char_arr_v = is_char_arr<T>::value;

template <class T>
constexpr bool is_c_str_v = std::is_same<const char*, T>::value;


// TODO: move to mpp_enc.hpp ?
template <class BUFFER>
class Enc
{
	using Buffer_t = BUFFER;
	using iterator_base_t = typename BUFFER::iterator;

public:
	struct iterator : public iterator_base_t
	{
		iterator(const iterator_base_t& b) : iterator_base_t(b) {}
		template <class T>
		void set_int(const T&) const;
	};

	explicit Enc(Buffer_t& buf) : m_Buf(buf) {}

	iterator put(uint8_t tag)
	{
		iterator res = m_Buf.appendBack(1);
		m_Buf.set(res, tag);
		return res;
	}

	template <class T>
	iterator put(uint8_t tag, T t)
	{
		iterator res = m_Buf.appendBack(1);
		m_Buf.set(res, tag);
		under_uint_t<T> x;
		memcpy(&x, &t, sizeof(x));
		m_Buf.addBack(bswap(x));
		return res;
	}

	iterator put_data(uint8_t tag, const char *data, size_t size)
	{
		iterator res = m_Buf.appendBack(1);
		m_Buf.set(res, tag);
		m_Buf.addBack(data, size);
		return res;
	}

	template <class T>
	iterator put_data(uint8_t tag, T size, const char *data)
	{
		iterator res = m_Buf.appendBack(1);
		m_Buf.set(res, tag);
		m_Buf.addBack(bswap(size));
		m_Buf.addBack(data, size);
		return res;
	}

	iterator add_null()
	{
		return put(0xc0);
	}

	iterator add_bool(bool b)
	{
		return put(0xc2 + b);
	}

	template <class T>
	iterator add_int(T t)
	{
		static_assert(std::is_integral_v<T>);
		if constexpr (std::is_signed_v<T>) if (t < 0) {
			if (t >= -32)
				return put(t);
			if constexpr (sizeof(T) > 4) if (t < INT32_MIN)
				return put<int64_t>(0xd3, t);
			if constexpr (sizeof(T) > 2) if (t < INT16_MAX)
				return put<int32_t>(0xd2, t);
			if constexpr (sizeof(T) > 1) if (t < INT8_MAX)
				return put<int16_t>(0xd1, t);
			return put<int8_t>(0xd0, t);
		}
		if (t <= 127)
			return put(t);
		if constexpr (sizeof(T) > 4) if (t > UINT32_MAX)
			return put<uint64_t>(0xcf, t);
		if constexpr (sizeof(T) > 2) if (t > UINT16_MAX)
			return put<uint32_t>(0xce, t);
		if constexpr (sizeof(T) > 1) if (t > UINT8_MAX)
			return put<uint16_t>(0xcd, t);
		return put<uint8_t>(0xcc, t);
	}

	template <class T>
	iterator add_flt(T t)
	{
		static_assert(std::is_floating_point_v<T>);
		static_assert((1u << sizeof(T)) & 12u, "Not a floating point");
		if constexpr (sizeof(T) == 4)
			return put(0xca, t);
		else if constexpr (sizeof(T) == 8)
			return put(0xcb, t);
	}

	template <class T>
	iterator add_str(const char *data, T size)
	{
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
		assert(size <= UINT32_MAX);
		if (size < 32)
			return put_data(0xa0 + size, data, size);
		if constexpr (sizeof(T) > 2) if (size > UINT16_MAX)
			return put_data<uint32_t>(0xdb, size, data);
		if constexpr (sizeof(T) > 1) if (size > UINT8_MAX)
			return put_data<uint16_t>(0xda, size, data);
		return put_data<uint8_t>(0xd9, size, data);
	}

	template <class T>
	iterator add_bin(const char *data, T size)
	{
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
		assert(size <= UINT32_MAX);
		if constexpr (sizeof(T) > 2) if (size > UINT16_MAX)
			return put_data<uint32_t>(0xc6, size, data);
		if constexpr (sizeof(T) > 1) if (size > UINT8_MAX)
			return put_data<uint16_t>(0xc5, size, data);
		return put_data<uint8_t>(0xc4, size, data);
	}

	template <class T>
	iterator add_arr(T size)
	{
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
		assert(size <= UINT32_MAX);
		if (size < 16)
			return put(0x90 + size);
		if constexpr (sizeof(T) > 2) if (size > UINT16_MAX)
			return put<uint32_t>(0xdd, size);
		return put<uint16_t>(0xdc, size);
	}

	template <class T>
	iterator add_map(T size)
	{
		static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>);
		assert(size <= UINT32_MAX);
		if (size < 16)
			return put(0x80 + size);
		if constexpr (sizeof(T) > 2) if (size > UINT16_MAX)
			return put<uint32_t>(0xdf, size);
		return put<uint16_t>(0xde, size);
	}

	template <class T>
	iterator add(const T& t) {
		if constexpr (std::is_same_v<T, nullptr_t>) {
			return add_null(t); (void)t;
		} else if constexpr (std::is_same_v<T, bool>) {
			return add_bool(t);
		} else if constexpr (std::is_integral_v<T>) {
			return add_int(t);
		} else if constexpr (std::is_floating_point_v<T>) {
			return add_flt(t);
		} else if constexpr (is_str_v<T>) {
			using U = typename T::type;
			const auto& u = t.value;
			static_assert(is_cvref_char_v<decltype(*std::data(u))>);
			if constexpr (is_char_arr_v<U>)
				return add_str(std::data(u), std::size(u) - 1);
			else if constexpr (is_c_str_v<U>)
				return add_str(std::data(u), strlen(u));
			else
				return add_str(std::data(u), std::size(u));
		} else if constexpr (is_bin_v<T>) {
			using U = typename T::type;
			const auto &u = t.value;
			if constexpr (is_c_str_v<U>)
				return add_bin(std::data(u), strlen(u));
			else
				return add_bin(std::data(u),
					       std::size(u) *
					       sizeof(*std::data(u)));
		} else if constexpr (is_arr_v<T>) {
			// TODO: Full rollback in case of any failure.
			static_assert(looks_like_arr_v<T> || is_tuple_v<T>,
				      "Wrong tuple was passed as array");
			if constexpr (looks_like_arr_v<T>) {
				iterator itr = add_arr(std::size(t));
				for (const auto& x : t)
					add(x);
				return itr;
			} else if constexpr (is_tuple_v<T>) {
				iterator itr = add_arr(std::tuple_size<T>::value);
				std::visit([this](const auto& a){ add(a); }, t);
				return itr;
			} else {
				unreachable();
			}
		} else if constexpr (is_map_v<T>) {
			// TODO: Full rollback in case of any failure.
			static_assert(looks_like_arr_v<T> || is_tuple_v<T>,
				      "Wrong tuple was passed as map");
			if constexpr (looks_like_map_v<T>) {
				iterator itr = add_map(std::size(t));
				for (const auto& x : t) {
					add(x.first);
					add(x.second);
				}
				return itr;
			} else if constexpr (looks_like_arr_v<T>) {
				assert(std::size(t) % 2 == 0);
				iterator itr = add_map(std::size(t) / 2);
				for (const auto& x : t)
					add(x);
				return itr;
			} else if constexpr (is_tuple_v<T>) {
				static_assert(std::tuple_size<T>::value % 2 == 0,
					      "Map expects even number of elements");
				iterator itr = add_map(std::tuple_size<T>::value / 2);
				std::visit([this](const auto& a){ add(a); }, t);
				return itr;
			} else {
				unreachable();
			}
		} else if constexpr (looks_like_str_v<T>) {
			return add(as_str(t));
		} else if constexpr (is_c_str_v<T>) {
			return add(as_str(t));
		} else if constexpr (looks_like_map_v<T>) {
			return add(as_map(t));
		} else if constexpr (looks_like_arr_v<T>) {
			return add(as_arr(t));
		} else if constexpr (is_tuple_v<T>) {
			return add(as_arr(t));
		} else {
			unreachable();
		}

	}

private:
	BUFFER& m_Buf;
};

// TODO: move to mpp_enc.hpp ?
template <class BUFFER>
class Dec
{
	using Buffer_t = BUFFER;
	using iterator_base_t = typename BUFFER::iterator;

public:
	Dec(Buffer_t& buf) : m_Buf(buf) {}

	struct Item {
		Item() = default;
		Item(const Item&) = delete;
		void operator=(const Item&) = delete;
		~Item()
		{
			while (child != nullptr) {
				Item * del = child;
				child = child->next;
				delete del;
			}
		}

		Types type;
		int8_t ext_type;
		uint8_t data_offset;
		uint8_t flags = 0;
		union {
			int64_t uint_value;
			uint64_t int_value;
			uint32_t str_size;
			uint32_t bin_size;
			uint32_t arr_size;
			uint32_t map_size;
			bool bool_value;
			float flt_value;
			double dbl_value;
			// TODO: MP_EXT
		};
		Item *next = nullptr;
		Item *child = nullptr;
		Item *shortcut[2] = {nullptr, nullptr};
	};

private:
	BUFFER& m_Buf;
};


} // namespace mpp {
