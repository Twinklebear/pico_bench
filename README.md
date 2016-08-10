pico\_bench
===

A minimal C++11 benchmarking library.

Documentation
---

pico\_bench aims to make as few assumptions as possible about the code being benchmarked. To this end
there are two options for running benchmarks and collecting statistics. First though we create
a benchmarker to run the code, specifying a maximum number of iterations and max time
to run the benchmark for. The benchmark will stop running when either limit has been reached.

```c++
auto bencher = pico_bench::Benchmarker{10, std::chrono::seconds{1}};
```

The benchmarker can take a functor or a lambda, the function can either return a run time
in milliseconds of the operation you want to time or return void, in which case timing
code will be inserted for you. The first option exists to allow you to perform per-run setup
without including it in the run-time of the benchmarked code itself.

The benchmarker will then return the statistics measured while running the function such as
median and mean run time, median abs deviation and so on.

**Benchmarking a Functor**

Your functor should define `operator()` which will either return the measured time in milliseconds or void.

```c++
#include <iostream>
#include <chrono>
#include <pico_bench.h>

struct MyFn {
	// This functor will time itself to exclude measuring time spent during setup
	std::chrono::milliseconds operator()(){
		return std::chrono::milliseconds{1};
	}
};

struct MyVoidFn {
	// This function will be timed externally
	void operator()(){
		std::cout << "Hi there\n";
	}
};

int main(){
	using namespace std::chrono;
	// Note: passing no timeout will enforce that all iterations are run
	auto bencher = pico_bench::Benchmarker<milliseconds>{10, seconds{1}};
	auto stats = bencher(MyFn{});
	std::cout << "MyFn " << stats << "\n";

	stats = bencher(MyVoidFn{});
	std::cout << "MyVoidFn " << stats << "\n";
	return 0;
}
```

**Benchmarking a Lambda**

You can also provide a lambda function which will either return the measured time in milliseconds or void.

```c++
#include <iostream>
#include <chrono>
#include <pico_bench.h>

int main(){
	auto bencher = pico_bench::Benchmarker{10, std::chrono::seconds{1}};
	// This lambda will time itself to exclude measuring time spent during setup
	auto stats = bencher([](){ return std::chrono::milliseconds{1}; });
	std::cout << "Self timed lambda " << stats << "\n";

	// This lambda will be timed externally
	stats = bencher([](){ std::cout << "Hi there\n"; });
	std::cout << "External timed lambda " << stats << "\n";
	return 0;
}
```

