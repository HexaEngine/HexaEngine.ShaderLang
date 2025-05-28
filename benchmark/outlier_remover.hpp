#ifndef OUTLIER_REMOVER_HPP
#define OUTLIER_REMOVER_HPP

#include "pch/std.hpp"

struct OutlierResult
{
	std::vector<uint64_t> samples;
	std::vector<size_t> outliers;
};

namespace MAD
{
	template<typename Num>
	static double computeMedian(const std::vector<Num>& data)
	{
		static_assert(std::is_arithmetic_v<Num>, "computeMedian requires numeric type");
		const size_t n = data.size();
		if (n % 2 == 0)
		{
			return static_cast<double>(data[n / 2 - 1] + data[n / 2]) / 2.0;
		}
		else
		{
			return static_cast<double>(data[n / 2]);
		}
	}

	constexpr double epsilon = 1e-9;

	OutlierResult removeOutliers(const std::vector<uint64_t>& samples, double threshold = 3.5)
	{
		if (samples.size() < 4) return { samples, {} };

		std::vector<uint64_t> sorted = samples;
		std::sort(sorted.begin(), sorted.end());

		const size_t n = samples.size();

		double med = computeMedian(sorted);

		std::vector<double> deviations;
		deviations.resize(n);
		for (size_t i = 0; i < n; ++i)
		{
			deviations[i] = (std::abs(static_cast<double>(samples[i]) - med));
		}

		double mad = computeMedian(deviations);

		if (mad < epsilon) return { samples, {} };

		OutlierResult result;
		result.samples.reserve(n);
		for (size_t i = 0; i < n; ++i)
		{
			double modified_z_score = 0.6745 * (static_cast<double>(samples[i]) - med) / mad;
			if (std::abs(modified_z_score) <= threshold)
			{
				result.samples.emplace_back(samples[i]);
			}
			else
			{
				result.outliers.emplace_back(i);
			}
		}

		return result;
	}
}

#endif