#ifndef BENCHMARK_BASE_HPP
#define BENCHMARK_BASE_HPP

#include "pch/std.hpp"
#include "outlier_remover.hpp"
#include "clock.hpp"

struct BenchStats
{
	size_t steps;
	size_t N;
	size_t runs;
	size_t runsWarmup;
	double tick_to_second;
	std::vector<uint64_t> rawSamples;
	std::vector<uint64_t> samples;
	std::vector<size_t> outliers;
	double sum;
	double mean;
	double stddev;
	uint64_t min;
	uint64_t max;

	static void format_time(double seconds, double& value, const char*& unit)
	{
		if (seconds < 1e-6)
		{
			value = seconds * 1e9;
			unit = "ns";
		}
		else if (seconds < 1e-3)
		{
			value = seconds * 1e6;
			unit = "µs";
		}
		else if (seconds < 1)
		{
			value = seconds * 1e3;
			unit = "ms";
		}
		else
		{
			value = seconds;
			unit = "s";
		}
	}

	void print_stats() const
	{
		std::cout << "Benchmark results (" << steps << " steps, " << runs << " runs, " << N << " items each):\n";
		double val;
		const char* unit;

		format_time(sum, val, unit);
		std::cout << "  Sum time: " << std::fixed << std::setprecision(3) << val << " " << unit << "\n";

		format_time(mean, val, unit);
		std::cout << "  Avg time: " << std::fixed << std::setprecision(3) << val << " " << unit << "\n";

		format_time(stddev, val, unit);
		std::cout << "  Std dev:  " << std::fixed << std::setprecision(3) << val << " " << unit << "\n";

		std::unordered_set<uint64_t> visited;
		if (!outliers.empty())
		{
			auto x = 0;
			for (auto i : outliers)
			{
				auto s = rawSamples[i];
				if (visited.insert(s).second)
				{
					auto t = s * tick_to_second;
					double val;
					const char* unit;
					format_time(t, val, unit);
					std::cout << " ->  Removed outlier:  " << std::fixed << std::setprecision(3) << val << " " << unit << "\n";
				}
				x++;
				if (x == 10)
				{
					std::cout << "...\n";
					break;
				}
			}
		}
	}
};

template <typename Derived, typename clock = Bench::clock, typename duration = Bench::duration>
class Benchmark
{
	//static_assert(std::is_base_of_v<Benchmark<Derived>, Derived>, "Derived must inherit from Benchmark<Derived>");
protected:
	size_t steps;
	size_t N;
	size_t runs;
	size_t runsWarmup;

public:
	Benchmark(size_t steps, size_t N, size_t runs, size_t runsWarmup) : steps(steps), N(N), runs(runs), runsWarmup(runsWarmup) {}

	virtual ~Benchmark() = default;

	void setup()
	{
		static_cast<Derived*>(this)->setup();
	}

	void reset()
	{
		static_cast<Derived*>(this)->reset();
	}

	void run_operation()
	{
		static_cast<Derived*>(this)->run_operation();
	}

	void tear_down()
	{
		static_cast<Derived*>(this)->tear_down();
	}

	void warmup()
	{
		for (size_t r = 0; r < runsWarmup; ++r)
		{
			run_operation();
		}
	}

	BenchStats run()
	{
		setup();
		reset();
		warmup();

		std::vector<uint64_t> samples;
		samples.resize(runs * steps);

		for (size_t s = 0; s < steps; ++s)
		{
			size_t gIdx = s * runs;
			reset();
			for (size_t r = 0; r < runs; ++r)
			{
				auto start = clock::now();
				run_operation();
				auto end = clock::now();

				auto diff = end - start;
				samples[gIdx + r] = diff;
			}
		}

		tear_down();

		return compute_stats(samples);
	}

protected:
	BenchStats compute_stats(const std::vector<uint64_t>& rawSamples) const
	{
		auto result = MAD::removeOutliers(rawSamples);

		const double tick_to_seconds = 1.0 / static_cast<double>(duration::freqency());

		BenchStats stats;
		stats.steps = steps;
		stats.N = N;
		stats.runs = runs;
		stats.runsWarmup = runsWarmup;
		stats.tick_to_second = tick_to_seconds;
		stats.rawSamples = rawSamples;
		stats.samples = result.samples;
		stats.outliers = result.outliers;
		auto& samples = stats.samples;

		const double size_inv = 1.0 / samples.size();

		uint64_t sum = 0;
		uint64_t min = 0;
		uint64_t max = std::numeric_limits<uint64_t>::max();
		for (uint64_t sample : samples)
		{
			sum += sample;
			min = std::min(min, sample);
			max = std::max(max, sample);
		}

		stats.sum = sum * tick_to_seconds;
		stats.mean = stats.sum * size_inv;
		stats.max = max;
		stats.min = min;

		double sq_sum = 0.0;
		for (uint64_t t : samples)
		{
			const double td = t * tick_to_seconds;
			sq_sum += (td - stats.mean) * (td - stats.mean);
		}
		stats.stddev = std::sqrt(sq_sum * size_inv);

		return stats;
	}
};

#endif