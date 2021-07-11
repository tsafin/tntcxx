
#include <benchmark/benchmark.h>

#include <cstring>
#include <memory>
#include <tuple>
#include <utility>

#include "../src/Buffer/Buffer.hpp"
#include "Utils/Out.hpp"
#include "Utils/PerfTimer.hpp"

constexpr size_t N = 16 * 1024 * 1024;

struct alignas(2) SimpleData_t : std::tuple<uint8_t> {};
using ComplexData_t = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;
struct VariadicData_t {
	uint8_t size;
	char data[15];
};
constexpr size_t MAX_SIZE =
	std::max({ sizeof(SimpleData_t), sizeof(ComplexData_t),
		   sizeof(VariadicData_t) });

static_assert(sizeof(SimpleData_t) == 2, "Just for understanding");
static_assert(sizeof(ComplexData_t) == 16, "Just for understanding");
static_assert(sizeof(VariadicData_t) == 16, "Just for understanding");

SimpleData_t simpleDataIn[N];
SimpleData_t simpleDataOut[N];
ComplexData_t complexDataIn[N];
ComplexData_t complexDataOut[N];
VariadicData_t variadicDataIn[N];
VariadicData_t variadicDataOut[N];

char staticBuffer[N * MAX_SIZE];

struct StaticBuffer {
	char *p = staticBuffer;

	template <bool>
	char *begin() const
	{
		return staticBuffer;
	}
	template <bool>
	char *end() const
	{
		return p;
	}
	template <class T>
	void addBack(T &&t)
	{
		mempcpy(p, &t, sizeof(t));
		p += sizeof(t);
	}
	void addBack(wrap::Data d)
	{
		mempcpy(p, d.data, d.size);
		p += d.size;
	}
};

template <size_t... I, class... T>
void
gen_helper(std::index_sequence<I...>, std::tuple<T...> &t)
{
	auto set = [](auto &t) { t = rand(); };
	(set(std::get<I>(t)), ...);
}

template <class... T>
void
gen(std::tuple<T...> &t)
{
	gen_helper(std::make_index_sequence<sizeof...(T)> {}, t);
}

void
gen(VariadicData_t &t)
{
	t.size = 1 + rand() % sizeof(t.data);
	for (size_t i = 0; i < t.size; i++)
		t.data[i] = rand();
}

template <class... T>
constexpr size_t
dataSize(const std::tuple<T...> &)
{
	return (sizeof(T) + ...);
}

template <class... T>
constexpr size_t
dataSize(const VariadicData_t &t)
{
	return t.size + 1;
}

template <class T, size_t N>
size_t dataSizeTotal(T (&arr)[N])
{
	size_t res = 0;
	for (auto &x : arr)
		res += dataSize(x);
	return res;
}

template <class T>
size_t
dataSizeTotal(T *b, T *e)
{
	size_t res = 0;
	for (; b != e; ++b)
		res += dataSize(*b);
	return res;
}

template <size_t... I, class... T>
size_t
dataCheckSumHelper(std::index_sequence<I...>, const std::tuple<T...> &t)
{
	return (std::get<I>(t) + ...);
}

template <class... T>
size_t
dataCheckSum(const std::tuple<T...> &t)
{
	return dataCheckSumHelper(std::make_index_sequence<sizeof...(T)> {}, t);
}

size_t
dataCheckSum(const VariadicData_t &t)
{
	size_t res = t.size;
	for (size_t i = 0; i < t.size; i++)
		res = (res << 1) + uint8_t(t.data[i]);
	return res;
}

template <class T, size_t N>
size_t dataCheckSumTotal(T (&arr)[N])
{
	size_t res = 0;
	for (auto &x : arr)
		res += dataCheckSum(x);
	return res;
}

template <class T>
size_t
dataCheckSumTotal(T *b, T *e)
{
	size_t res = 0;
	for (; b != e; ++b)
		res += dataCheckSum(*b);
	return res;
}

template <class T>
const char *
dataName(const T &)
{
	static_assert(std::is_same_v<T, SimpleData_t> ||
		      std::is_same_v<T, ComplexData_t> ||
		      std::is_same_v<T, VariadicData_t>);
	if constexpr (std::is_same_v<T, SimpleData_t>)
		return "Simple data";
	if constexpr (std::is_same_v<T, ComplexData_t>)
		return "Complex data";
	if constexpr (std::is_same_v<T, VariadicData_t>)
		return "Variadic data";
	return "";
}

template <class CONT, size_t... I, class... T>
void
write_helper(CONT &c, std::index_sequence<I...>, const std::tuple<T...> &t)
{
	(c.addBack(std::get<I>(t)), ...);
}

template <class CONT, class... T>
void
write(CONT &c, const std::tuple<T...> &t)
{
	write_helper(c, std::make_index_sequence<sizeof...(T)> {}, t);
}

template <class CONT>
void
write(CONT &c, VariadicData_t &t)
{
	c.addBack(t.size);
	c.addBack(wrap::Data { t.data, t.size });
}

template <class T>
void
read_one(StaticBuffer &, char *&itr, T &t)
{
	std::memcpy(&t, &*itr, sizeof(t));
	itr += sizeof(t);
}

template <size_t N, class ALL, class T>
void
read_one(tnt::Buffer<N, ALL> &b, typename tnt::Buffer<N, ALL>::iterator &itr,
	 T &t)
{
	b.get(itr, t);
	itr += sizeof(t);
}

template <size_t N, class ALL, class T>
void
read_one(tnt::Buffer<N, ALL> &b,
	 typename tnt::Buffer<N, ALL>::light_iterator &itr, T &t)
{
	b.get(itr, t);
	itr += sizeof(t);
}

void
read_one(StaticBuffer &, char *&itr, VariadicData_t &t)
{
	t.size = *itr;
	itr += 1;
	std::memcpy(t.data, &*itr, t.size);
	itr += t.size;
}

template <size_t N, class ALL>
void
read_one(tnt::Buffer<N, ALL> &b, typename tnt::Buffer<N, ALL>::iterator &itr,
	 VariadicData_t &t)
{
	b.get(itr, t.size);
	++itr;
	b.get(itr, t.data, t.size);
	itr += t.size;
}

template <size_t N, class ALL>
void
read_one(tnt::Buffer<N, ALL> &b,
	 typename tnt::Buffer<N, ALL>::light_iterator &itr, VariadicData_t &t)
{
	b.get(itr, t.size);
	++itr;
	b.get(itr, t.data, t.size);
	itr += t.size;
}

template <class CONT, class ITR, size_t... I, class... T>
void
read_helper(CONT &c, ITR &itr, std::index_sequence<I...>, std::tuple<T...> &t)
{
	(read_one(c, itr, std::get<I>(t)), ...);
}

template <class CONT, class ITR, class... T>
void
read(CONT &c, ITR &itr, std::tuple<T...> &t)
{
	read_helper(c, itr, std::make_index_sequence<sizeof...(T)> {}, t);
}

template <class CONT, class ITR>
void
read(CONT &c, ITR &itr, VariadicData_t &t)
{
	read_one(c, itr, t);
}

std::string
contName(const StaticBuffer &)
{
	return "static buffer";
}

template <size_t N, class ALL>
std::string
contName(const tnt::Buffer<N, ALL> &)
{
	return "Buffer<" + std::to_string(N) + ">";
}

template <class T, class U>
void
reconstruct(T &t, U &&u)
{
	t.~T();
	new (&t) T(std::forward<U>(u));
}

template <class CONT, class DATA, bool IS_LIGHT>
static void
BM_write_test(benchmark::State &state)
{
	constexpr size_t N = 64 * 1024;
	constexpr size_t K = 16;
	std::unique_ptr<CONT> cont;
	DATA wr_data[N];
	DATA rd_data[N];
	DATA *p, *e = wr_data + N;
	size_t total_count = 0;
	size_t total_size = 0;

	auto prepare = [&]() {
		cont.reset(new CONT);
		for (auto &x : wr_data)
			gen(x);
		p = wr_data;
	};

	auto check = [&]() {
		size_t count = p - wr_data;
		total_count += count;
		total_size += dataSizeTotal(wr_data, wr_data + count);

		auto itr = cont->template begin<IS_LIGHT>();
		for (size_t i = 0; i < count; i++)
			read(*cont, itr, rd_data[i]);

		if (dataCheckSumTotal(wr_data, p) !=
		    dataCheckSumTotal(rd_data, rd_data + count)) {
			state.SkipWithError("FAILURE: wrong checksum (w)!");
			return false;
		}
		return true;
	};

	prepare();

	for (auto _ : state) {
		if (p == e) {
			state.PauseTiming();
			if (!check())
				abort();
			prepare();
			state.ResumeTiming();
		}
		// benchmark::DoNotOptimize();
		for (size_t i = 0; i < K; i++) {
			write(*cont, *p);
			++p;
		}
	}

	check();

	state.SetItemsProcessed(total_count);
	state.SetBytesProcessed(total_size);
}

template <class CONT, class DATA, bool IS_LIGHT>
static void
BM_read_test(benchmark::State &state)
{
	constexpr size_t N = 64 * 1024;
	constexpr size_t K = 16;
	std::unique_ptr<CONT> cont(new CONT);
	auto itr = cont->template begin<IS_LIGHT>();
	DATA wr_data[N];
	DATA rd_data[N];
	DATA *p, *e = rd_data + N;
	size_t total_count = 0;
	size_t total_size = 0;

	auto prepare = [&]() {
		cont.reset(new CONT);
		for (auto &x : wr_data)
			gen(x);
		for (size_t i = 0; i < N; i++)
			write(*cont, wr_data[i]);
		p = rd_data;
		// Can't execute simple:
		// itr = cont->begin();
		// .. because operator=() expects iterator from the same buffer.
		// Use some magic instead.
		reconstruct(itr, cont->template begin<IS_LIGHT>());
	};

	auto check = [&]() {
		size_t count = p - rd_data;
		total_count += count;
		total_size += dataSizeTotal(wr_data, wr_data + count);

		if (dataCheckSumTotal(wr_data, wr_data + count) !=
		    dataCheckSumTotal(rd_data, p)) {
			state.SkipWithError("FAILURE: wrong checksum (r)!");
			return false;
		}
		return true;
	};

	prepare();

	for (auto _ : state) {
		if (p == e) {
			state.PauseTiming();
			if (!check())
				abort();
			prepare();
			state.ResumeTiming();
		}
		// benchmark::DoNotOptimize();
		for (size_t i = 0; i < K; i++) {
			read(*cont, itr, *p);
			++p;
		}
	}

	check();

	state.SetItemsProcessed(total_count);
	state.SetBytesProcessed(total_size);
}

constexpr bool Common = false;
constexpr bool Light = true;

BENCHMARK_TEMPLATE(BM_write_test, StaticBuffer, SimpleData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, StaticBuffer, SimpleData_t, Common);
BENCHMARK_TEMPLATE(BM_write_test, tnt::Buffer<>, SimpleData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, tnt::Buffer<>, SimpleData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, tnt::Buffer<>, SimpleData_t, Light);
BENCHMARK_TEMPLATE(BM_write_test, StaticBuffer, ComplexData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, StaticBuffer, ComplexData_t, Common);
BENCHMARK_TEMPLATE(BM_write_test, tnt::Buffer<>, ComplexData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, tnt::Buffer<>, ComplexData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, tnt::Buffer<>, ComplexData_t, Light);
BENCHMARK_TEMPLATE(BM_write_test, StaticBuffer, VariadicData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, StaticBuffer, VariadicData_t, Common);
BENCHMARK_TEMPLATE(BM_write_test, tnt::Buffer<>, VariadicData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, tnt::Buffer<>, VariadicData_t, Common);
BENCHMARK_TEMPLATE(BM_read_test, tnt::Buffer<>, VariadicData_t, Light);

BENCHMARK_MAIN();
