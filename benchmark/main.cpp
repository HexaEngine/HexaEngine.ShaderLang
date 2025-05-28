#include "utils/dense_map_simd.hpp"
#include "utils/dense_map.hpp"
#include "benchmark_base.hpp"

#include <windows.h>

class DenseMapSIMDBench : public Benchmark<DenseMapSIMDBench>
{
	dense_map_simd<size_t, size_t> map;
	size_t idx = 0;
public:
	DenseMapSIMDBench() : Benchmark(10, 1, 1000000, 1000)
	{
	}

	void setup()
	{
		map.reserve(1000000);
		//map.load_factor(0.2);
	}

	void reset()
	{
		idx = 0;
		map.clear();
	}

	void run_operation()
	{
		map.emplace(idx, idx * 2);
		idx++;
	}

	void tear_down()
	{
	}
};

class DenseMapBench : public Benchmark<DenseMapBench>
{
	dense_map<size_t, size_t> map;
	size_t idx = 0;
public:
	DenseMapBench() : Benchmark(10, 1, 1000000, 1000)
	{
	}

	void setup()
	{
		map.reserve(1000000);
		//map.load_factor(0.2);
	}

	void reset()
	{
		idx = 0;
		map.clear();
	}

	void run_operation()
	{
		map.emplace(idx, idx * 2);
		idx++;
	}

	void tear_down()
	{
	}
};

void pin_to_core(DWORD core_id = 0)
{
	DWORD_PTR mask = 1ULL << core_id;
	SetThreadAffinityMask(GetCurrentThread(), mask);
}

int main()
{
	SetConsoleOutputCP(CP_UTF8);
	pin_to_core();
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	std::cout << "dense_map non SIMD\n";
	DenseMapBench n;
	n.run().print_stats();

	std::cout << "dense_map SIMD AVX2\n";
	DenseMapSIMDBench bench;
	bench.run().print_stats();

	return 0;
}