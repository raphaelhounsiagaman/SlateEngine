#include "Slate/Core/Application.h"

#include <ranges>
#include <chrono>

namespace Slate
{ 

	static Application* s_Application = nullptr;

	Application::Application(const Slate::WindowInformation& info)
	{
		s_Application = this;

		m_Window.Create(info);
		m_Renderer.Create(m_Window.GetHandle(), m_Window.GetWidth(), m_Window.GetHeight());

		m_Running = true;
	}

	Application::~Application()
	{
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
			m_Window.ProcessEvents();

			if (m_Window.ShouldClose())
			{
				Stop();
				break;
			}

			if (m_Window.WasResized())
			{
				m_Renderer.Resize(
					m_Window.GetWidth(),
					m_Window.GetHeight()
				);
				m_Window.ClearResizedFlag();
			}

			auto currentTime = high_resolution_clock::now();
			float deltaTime = duration<float, seconds::period>(currentTime - lastTime).count();
			lastTime = currentTime;

			for (const std::unique_ptr<ApplicationLayer>& layer : m_LayerStack)
				layer->OnUpdate(deltaTime);

			m_Renderer.BeginFrame();

			for (const std::unique_ptr<ApplicationLayer>& layer : m_LayerStack)
				layer->OnRender();


			m_Renderer.Present();

		}
	}

	void Application::EmitEvent(Event& event)
	{

		for (auto& layer : std::views::reverse(m_LayerStack))
		{
			layer->OnEvent(event);
			if (event.Handled)
				break;
		}

	}

	void Application::Stop()
	{
		m_Running = false;
	}

	Application& Application::Get()
	{
		//assert(s_Application);
		return *s_Application;
	}

}