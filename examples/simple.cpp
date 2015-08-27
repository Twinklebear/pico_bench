#include <chrono>
#include <thread>
#include <iostream>
#include <pico_bench.h>

struct BenchVoid {
	void operator()(){
		std::cout << "void bench\n";
	}
};

struct BenchNanos {
	std::chrono::milliseconds operator()(){
		std::cout << "benchnanos\n";
		return std::chrono::milliseconds{10};
	}
};

struct BenchInvalid {
	float operator()(){
		return 10.f;
	}
};

int main(int, char**){
	pico_bench::Benchmarker bencher{2, std::chrono::seconds{1}};
	bencher(BenchVoid{});
	bencher(BenchNanos{});
	auto stats = bencher([](){
			std::cout << "hi lambda\n";
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
	});
	std::cout << "Lambda " << stats << "\n";
	return 0;
}

