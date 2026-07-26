#include "Slate/Core/Application.h"

#include "Slate/Input/Input.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <ranges>
#include <stdexcept>
#include <thread>

namespace Slate
{
	namespace
	{
		using ApplicationClock = std::chrono::steady_clock;

		ApplicationClock::duration GetRateInterval(double rateLimit)
		{
			if (rateLimit <= 0.0)
			{
				return ApplicationClock::duration::zero();
			}

			return std::chrono::duration_cast<ApplicationClock::duration>(
				std::chrono::duration<double>(1.0 / rateLimit)
			);
		}
	}

	static Application* s_Application = nullptr;

	Application::Application(const Slate::WindowInformation& info)
	{
		s_Application = this;

		m_Window.SetEventCallback([this](Event& event)
		{
			EmitEvent(event);
		});
		m_Window.Create(info);
		m_Renderer.Create(
			m_Window.GetHandle(),
			m_Window.GetClientWidthPixels(),
			m_Window.GetClientHeightPixels()
		);

		m_Running = true;
	}

	Application::~Application()
	{
		for (const std::unique_ptr<ApplicationLayer>& layer : m_LayerStack)
		{
			layer->OnDetach();
		}
		m_PendingLayerTransitions.clear();
		m_LayerStack.clear();

		m_Renderer.Destroy();
		m_Window.Destroy();

		s_Application = nullptr;
	}

	void Application::Run()
	{
		const ApplicationClock::duration updateInterval =
			GetRateInterval(m_LoopSettings.UpdateRateLimit);
		const ApplicationClock::duration frameInterval =
			GetRateInterval(m_LoopSettings.FrameRateLimit);

		auto previousUpdateTime = ApplicationClock::now();
		auto nextUpdateTime = previousUpdateTime;
		auto nextFrameTime = previousUpdateTime;
		auto statisticsStartTime = previousUpdateTime;

		std::uint64_t updateCount = 0;
		std::uint64_t frameCount = 0;
		double accumulatedUpdateDurationSeconds = 0.0;
		double accumulatedFrameDurationSeconds = 0.0;

		while (m_Running)
		{
			m_Window.ProcessEvents();

			if (m_Window.ShouldClose() || !m_Running)
			{
				Stop();
				break;
			}

			if (m_Window.IsMinimized())
			{
				m_Window.WaitForEvents();
				previousUpdateTime = ApplicationClock::now();
				nextUpdateTime = previousUpdateTime;
				nextFrameTime = previousUpdateTime;
				statisticsStartTime = previousUpdateTime;
				updateCount = 0;
				frameCount = 0;
				accumulatedUpdateDurationSeconds = 0.0;
				accumulatedFrameDurationSeconds = 0.0;
				continue;
			}

			const auto currentTime = ApplicationClock::now();
			const bool shouldUpdate =
				updateInterval == ApplicationClock::duration::zero() ||
				currentTime >= nextUpdateTime;
			const bool shouldRender =
				frameInterval == ApplicationClock::duration::zero() ||
				currentTime >= nextFrameTime;

			if (shouldUpdate)
			{
				const float elapsedSeconds =
					std::chrono::duration<float>(
						currentTime - previousUpdateTime
					).count();
				const float deltaTimeSeconds = std::min(
					elapsedSeconds,
					m_LoopSettings.MaximumDeltaTimeSeconds
				);
				previousUpdateTime = currentTime;

				const auto updateStartTime = ApplicationClock::now();
				for (const std::unique_ptr<ApplicationLayer>& layer :
					m_LayerStack)
				{
					layer->OnUpdate(deltaTimeSeconds);
				}
				const auto updateEndTime = ApplicationClock::now();

				accumulatedUpdateDurationSeconds +=
					std::chrono::duration<double>(
						updateEndTime - updateStartTime
					).count();
				++updateCount;
				Input::EndUpdate();

				nextUpdateTime = updateInterval ==
					ApplicationClock::duration::zero()
					? updateEndTime
					: currentTime + updateInterval;
			}

			if (shouldRender)
			{
				const auto frameStartTime = ApplicationClock::now();
				m_Renderer.BeginFrame();

				for (const std::unique_ptr<ApplicationLayer>& layer :
					m_LayerStack)
				{
					layer->OnRender();
				}

				m_Renderer.Present();
				const auto frameEndTime = ApplicationClock::now();

				accumulatedFrameDurationSeconds +=
					std::chrono::duration<double>(
						frameEndTime - frameStartTime
					).count();
				++frameCount;

				nextFrameTime = frameInterval ==
					ApplicationClock::duration::zero()
					? frameEndTime
					: currentTime + frameInterval;
			}

			if (shouldUpdate || shouldRender)
			{
				ProcessLayerOperations();
			}

			const auto statisticsTime = ApplicationClock::now();
			const double statisticsElapsedSeconds =
				std::chrono::duration<double>(
					statisticsTime - statisticsStartTime
				).count();
			if (statisticsElapsedSeconds >=
				m_LoopSettings.StatisticsSampleIntervalSeconds)
			{
				m_PerformanceStatistics.UpdatesPerSecond =
					static_cast<double>(updateCount) /
					statisticsElapsedSeconds;
				m_PerformanceStatistics.FramesPerSecond =
					static_cast<double>(frameCount) /
					statisticsElapsedSeconds;
				m_PerformanceStatistics.AverageUpdateDurationMilliseconds =
					updateCount == 0
					? 0.0
					: accumulatedUpdateDurationSeconds * 1000.0 /
						static_cast<double>(updateCount);
				m_PerformanceStatistics.AverageFrameDurationMilliseconds =
					frameCount == 0
					? 0.0
					: accumulatedFrameDurationSeconds * 1000.0 /
						static_cast<double>(frameCount);
				++m_PerformanceStatistics.SampleNumber;

				statisticsStartTime = statisticsTime;
				updateCount = 0;
				frameCount = 0;
				accumulatedUpdateDurationSeconds = 0.0;
				accumulatedFrameDurationSeconds = 0.0;
			}

			if (!shouldUpdate && !shouldRender)
			{
				const auto maximumEventLatency =
					std::chrono::milliseconds(1);
				std::this_thread::sleep_until(
					std::min(
						std::min(nextUpdateTime, nextFrameTime),
						ApplicationClock::now() + maximumEventLatency
					)
				);
			}
		}
	}

	void Application::SetLoopSettings(
		const ApplicationLoopSettings& settings)
	{
		const bool hasValidRates =
			std::isfinite(settings.UpdateRateLimit) &&
			settings.UpdateRateLimit >= 0.0 &&
			std::isfinite(settings.FrameRateLimit) &&
			settings.FrameRateLimit >= 0.0;
		const bool hasValidTiming =
			std::isfinite(settings.MaximumDeltaTimeSeconds) &&
			settings.MaximumDeltaTimeSeconds > 0.0f &&
			std::isfinite(settings.StatisticsSampleIntervalSeconds) &&
			settings.StatisticsSampleIntervalSeconds > 0.0;

		if (!hasValidRates || !hasValidTiming)
		{
			throw std::invalid_argument(
				"Application loop rates must be non-negative and timing "
				"intervals must be greater than zero."
			);
		}

		m_LoopSettings = settings;
	}

	void Application::EmitEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(
			[this](WindowCloseEvent& closeEvent)
			{
				return OnWindowClose(closeEvent);
			}
		);
		dispatcher.Dispatch<WindowResizeEvent>(
			[this](WindowResizeEvent& resizeEvent)
			{
				return OnWindowResize(resizeEvent);
			}
		);

		for (auto& layer : std::views::reverse(m_LayerStack))
		{
			layer->OnEvent(event);
			if (event.Handled)
				break;
		}

	}

	bool Application::OnWindowClose(WindowCloseEvent&)
	{
		Stop();
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		if (event.GetWidth() > 0 && event.GetHeight() > 0)
		{
			m_Renderer.Resize(event.GetWidth(), event.GetHeight());
		}
		return false;
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	void Application::QueueLayerTransition(
		ApplicationLayer* source,
		std::unique_ptr<ApplicationLayer> destination)
	{
		if (source == nullptr || destination == nullptr)
		{
			return;
		}

		for (LayerTransition& transition : m_PendingLayerTransitions)
		{
			if (transition.Source == source)
			{
				transition.Destination = std::move(destination);
				return;
			}
		}

		m_PendingLayerTransitions.push_back(
			{ source, std::move(destination) }
		);
	}

	void Application::ProcessLayerOperations()
	{
		// Lifecycle callbacks are allowed to queue work for the next frame.
		// Moving the current batch prevents those additions from invalidating
		// this iteration.
		std::vector<LayerTransition> transitions =
			std::move(m_PendingLayerTransitions);
		m_PendingLayerTransitions.clear();

		for (LayerTransition& transition : transitions)
		{
			for (std::unique_ptr<ApplicationLayer>& layer : m_LayerStack)
			{
				if (layer.get() != transition.Source)
				{
					continue;
				}

				layer->OnDetach();
				transition.Destination->OnAttach();
				layer = std::move(transition.Destination);
				break;
			}
		}
	}

	Application& Application::Get()
	{
		assert(s_Application != nullptr);
		return *s_Application;
	}

}
