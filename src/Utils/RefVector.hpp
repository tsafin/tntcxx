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

#include <array>
#include <cstddef>

namespace tnt {

/**
 * Ref vector is a container that have Standard container API (at least a
 * part of it) and is connected (references) with an existing array (C or C++)
 * and a size.
 * Since it only holds references it does not have own state. That make
 * copy/move construction/assignment and destruction primitive. Usually it's
 * better to pass such a vector by value.
 * Of course actual array and size must be stored separately and must live
 * while the ref vector is used.
 */

template<class T, size_t N, class S>
class RefVector {
public:
	RefVector() = default;
	RefVector(T* data, S& size) : m_data(data), m_size(&size) {}

	static constexpr size_t static_capacity = N;
	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	void clear() noexcept;
	T* data() noexcept;
	const T* data() const noexcept;
	T& operator[](size_t i) noexcept;
	const T& operator[](size_t i) const noexcept;
	S size() const noexcept;
	constexpr static size_t capacity() { return N; }
	void push_back(const T& value);
	void push_back(T&& value);
	template< class... Args>
	reference emplace_back(Args&&... args);
	T* begin();
	const T* begin() const;
	const T* cbegin() const;
	T* end();
	const T* end() const;
	const T* cend() const;
private:
	T *m_data = nullptr;
	S *m_size = nullptr;
};

template <class ARR, class SIZE>
inline auto make_ref_vector(ARR& r, SIZE& s);

template<class T, size_t N, class S, class U>
inline auto subst(RefVector<T, N, S> vec, U& u);

} // namespace tnt {