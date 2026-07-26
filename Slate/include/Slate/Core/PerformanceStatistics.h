#pragma once

#include <cstdint>

namespace Slate
{
	struct ApplicationLoopSettings
	{
		// A rate limit of zero means that stage is uncapped.
		double UpdateRateLimit = 0.0;
		double FrameRateLimit = 0.0;
		float MaximumDeltaTimeSeconds = 0.1f;
		double StatisticsSampleIntervalSeconds = 0.5;
	};

	struct PerformanceStatistics
	{
		double UpdatesPerSecond = 0.0;
		double FramesPerSecond = 0.0;
		double AverageUpdateDurationMilliseconds = 0.0;
		double AverageFrameDurationMilliseconds = 0.0;
		std::uint64_t SampleNumber = 0;
	};
}
