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
	Statistics(std::vector<std::chrono::milliseconds> samples) : samples(samples){
		std::sort(std::begin(samples), std::end(samples));
	}
	std::chrono::milliseconds percentile(const double p) const {
		return percentile(p, samples);
	}
	// Winsorize the data, sets all entries above 100 - limit percentile and below limit percentile
	// to the value of that percentile
	void winsorize(const float limit){
		winsorize(limit, samples);
	}
	std::chrono::milliseconds median() const {
		return percentile(50.0, samples);
	}
	std::chrono::milliseconds median_abs_dev() const {
		const auto m = median();
		std::vector<std::chrono::milliseconds> deviations;
		deviations.reserve(samples.size());
		std::transform(std::begin(samples), std::end(samples), std::back_inserter(deviations),
			[&m](const auto &t){
				return std::chrono::milliseconds{std::abs((t - m).count())};
			});
		std::sort(std::begin(deviations), std::end(deviations));
		return percentile(50.0, deviations);
	}
	std::chrono::milliseconds mean() const {
		const auto m = std::accumulate(std::begin(samples), std::end(samples), std::chrono::milliseconds{0});
		return m / samples.size();
	}
	std::chrono::milliseconds std_dev() const {
		const auto m = mean();
		auto val = std::accumulate(std::begin(samples), std::end(samples), std::chrono::milliseconds{0},
			[&m](const auto &p, const auto &t){
				return std::chrono::milliseconds{static_cast<std::chrono::milliseconds::rep>(
					p.count() + std::pow((t - m).count(), 2))};
			});
		return std::chrono::milliseconds{static_cast<std::chrono::milliseconds::rep>(
			std::sqrt(1.0 / static_cast<double>(samples.size()) * static_cast<double>(val.count())))};
	}
	std::chrono::milliseconds min() const {
		return samples.front();
	}
	std::chrono::milliseconds max() const {
		return samples.back();
	}
	size_t size() const {
		return samples.size();
	}

private:
	// Winsorize the data, sets all entries above 100 - limit percentile and below limit percentile
	// to the value of that percentile
	static void winsorize(const float limit, std::vector<std::chrono::milliseconds> &samples){
		const auto low = percentile(limit, samples);
		const auto high = percentile(100.0 - limit, samples);
		for (auto &t : samples){
			if (t < low){
				t = low;
			}
			else if (t > high){
				t = high;
			}
		}
	}
	static std::chrono::milliseconds percentile(const double p,
			const std::vector<std::chrono::milliseconds> &samples){
		assert(!samples.empty());
		assert(p <= 100.0);
		assert(p >= 0.0);
		if (samples.size() == 1){
			return samples.front();
		}
		if (p == 100.0){
			return samples.back();
		}
		const double rank = p / 100.0 * (static_cast<double>(samples.size()) - 1.0);
		const double low_r = std::floor(rank);
		const double dist = rank - low_r;
		const size_t k = static_cast<size_t>(low_r);
		const auto low = samples[k];
		const auto high = samples[k + 1];
		typedef std::chrono::milliseconds::rep Rep;
		return std::chrono::milliseconds{static_cast<Rep>(low.count() + (high - low).count() * dist)};
	}
};
std::ostream& operator<<(std::ostream &os, const Statistics &stats){
	os << "Statistics:\n"
		<< "\tmedian: " << stats.median().count() << "ms\n"
		<< "\tmedian abs dev: " << stats.median_abs_dev().count() << "ms\n"
		<< "\tmean: " << stats.mean().count() << "ms\n"
		<< "\tstd dev: " << stats.std_dev().count() << "ms\n"
		<< "\t# of samples: " << stats.size();
	return os;
}

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
	Benchmarker(const size_t max_iter, const std::chrono::seconds max_runtime)
		: MAX_ITER(max_iter),
		MAX_RUNTIME(std::chrono::duration_cast<std::chrono::milliseconds>(max_runtime))
	{}
	template<typename Fn>
	typename std::enable_if<std::is_void<decltype(std::declval<Fn>()())>::value, Statistics>::type
	operator()(Fn fn){
		return (*this)(BenchWrapper<Fn>{fn});
	}
	template<typename Fn>
	typename std::enable_if<std::is_same<decltype(std::declval<Fn>()()), std::chrono::milliseconds>::value,
		Statistics>::type
	operator()(Fn fn){
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

#endif

