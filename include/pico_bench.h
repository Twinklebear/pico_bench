#ifndef PICO_BENCH_H
#define PICO_BENCH_H

#include <vector>
#include <cassert>
#include <chrono>
#include <algorithm>
#include <type_traits>
#include <ostream>

namespace pico_bench {
class Statistics {
	std::vector<std::chrono::milliseconds> samples;

public:
	Statistics(std::vector<std::chrono::milliseconds> samples);
	std::chrono::milliseconds percentile(const double p) const;
	// Winsorize the data, sets all entries above 100 - limit percentile and below limit percentile
	// to the value of that percentile
	void winsorize(const float limit);
	std::chrono::milliseconds median() const;
	std::chrono::milliseconds median_abs_dev() const;
	std::chrono::milliseconds mean() const;
	std::chrono::milliseconds std_dev() const;
	std::chrono::milliseconds min() const;
	std::chrono::milliseconds max() const;
	size_t size() const;

private:
	// Winsorize the data, sets all entries above 100 - limit percentile and below limit percentile
	// to the value of that percentile
	static void winsorize(const float limit, std::vector<std::chrono::milliseconds> &samples);
	static std::chrono::milliseconds percentile(const double p,
			const std::vector<std::chrono::milliseconds> &samples);
};

class Benchmarker {
	const size_t MAX_ITER;
	const std::chrono::milliseconds MAX_RUNTIME;

	template<typename Fn>
	struct BenchWrapper {
		Fn fn;

		BenchWrapper(Fn fn) : fn(fn){}
		std::chrono::milliseconds operator()(){
			auto start = std::chrono::high_resolution_clock::now();
			fn();
			auto end = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		}
	};

public:
	Benchmarker(const size_t max_iter, const std::chrono::seconds max_runtime);

	template<typename Fn>
	typename std::enable_if<std::is_void<decltype(std::declval<Fn>()())>::value, Statistics>::type
	operator()(Fn fn) const {
		return (*this)(BenchWrapper<Fn>{fn});
	}

	template<typename Fn>
	typename std::enable_if<std::is_same<decltype(std::declval<Fn>()()), std::chrono::milliseconds>::value,
		Statistics>::type
	operator()(Fn fn) const {
		// Do a single un-timed warm up run
		fn();
		std::chrono::milliseconds elapsed{0};
		std::vector<std::chrono::milliseconds> samples;
		for (size_t i = 0; i < MAX_ITER && elapsed < MAX_RUNTIME; ++i, elapsed += samples.back()){
			samples.push_back(fn());
		}
		return Statistics{samples};
	}
};
}

std::ostream& operator<<(std::ostream &os, const pico_bench::Statistics &stats);

#endif

