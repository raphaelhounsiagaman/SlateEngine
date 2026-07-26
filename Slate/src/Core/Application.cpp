#include "Slate/Core/Application.h"

#include "Slate/Input/Input.h"

#include <ranges>
#include <chrono>

namespace Slate
{ 

	static Application* s_Application = nullptr;

	Application::Application(const Slate::WindowInformation& info)
	{
		s_Application = this;

		m_Window.SetEventCallback([this](Event& event)
		{
			EmitEvent(event);
		});
		m_Window.Create(info);
		m_Renderer.Create(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

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
		using namespace std::chrono;

		auto lastTime = high_resolution_clock::now();

		while (m_Running)
		{
			Input::BeginFrame();
			m_Window.ProcessEvents();

			if (m_Window.ShouldClose() || !m_Running)
			{
				Stop();
				break;
			}

			auto currentTime = high_resolution_clock::now();

			if (m_Window.IsMinimized())
			{
				lastTime = currentTime;
				m_Window.WaitForEvents();
				continue;
			}

			float deltaTime = duration<float, seconds::period>(currentTime - lastTime).count();
			lastTime = currentTime;

			for (const std::unique_ptr<ApplicationLayer>& layer : m_LayerStack)
				layer->OnUpdate(deltaTime);

			m_Renderer.BeginFrame();

			for (const std::unique_ptr<ApplicationLayer>& layer : m_LayerStack)
				layer->OnRender();

			m_Renderer.Present();
			ProcessLayerOperations();

		}
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
		//assert(s_Application);
		return *s_Application;
	}

}
