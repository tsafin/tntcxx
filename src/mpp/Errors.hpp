#pragma once
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

#include "../Utils/Common.hpp"
/**
 * Default handler has all the errors that can happen during (de)coding.
 */
namespace mpp {
struct DefaultErrorHandler {
	template <class VALUE, class MIN, class NAMES, class... RSRV>
	constexpr bool
	UnderMin(const VALUE&, const MIN&, const NAMES&, const RSRV&...)
	{
		return false;
	}
	template <class VALUE, class MAX, class NAMES, class... RSRV>
	constexpr bool
	OverMax(const VALUE&, const MAX&, const NAMES&, const RSRV&...)
	{
		return false;
	}
	template <class VALUE, class LIMIT, class NAMES, class... RSRV>
	constexpr bool
	FixedOverflow(const VALUE&, const LIMIT&, const NAMES&, const RSRV&...)
	{
		return false;
	}
	template <class VALUE, class LIMIT, class NAMES, class... RSRV>
	constexpr bool
	SizeOverflow(const VALUE&, const LIMIT&, const NAMES&, const RSRV&...)
	{
		return false;
	}
};

#define MAKE_ERROR_HANDLER(name)						\
MAKE_IS_METHOD_CALLABLE_CHECKER(name);						\
template <size_t I, class... HANDLERS, class... T>				\
constexpr bool									\
name##Helper(const std::tuple<HANDLERS...>& handlers, const T&... t)		\
{										\
	static_assert(I < sizeof...(HANDLERS));					\
	auto& handler = std::get<I>(handlers);					\
	if constexpr(is_##name##_callable_v<decltype(handler), const T&...>)	\
		return handler.name(t...);					\
	else									\
		return name##Helper<I + 1>(handlers, t...);			\
}										\
template <class... HANDLERS, class... T>					\
constexpr bool									\
name(const std::tuple<HANDLERS...>& handlers, const T&... t)			\
{										\
	return name##Helper<0>(handlers, t...);					\
}										\
struct forgot_to_add_semicolon

MAKE_ERROR_HANDLER(UnderMin);
MAKE_ERROR_HANDLER(OverMax);
MAKE_ERROR_HANDLER(FixedOverflow);
MAKE_ERROR_HANDLER(SizeOverflow);

#undef MAKE_ERROR_HANDLER

} // namespace mpp {
