#pragma once

#include "Slate/Graphics/Window.h"
#include "Slate/Graphics/Renderer.h"

#include "Slate/Input/InputEvents.h"
#include "ApplicationLayer.h"

#include <vector>

namespace Slate
{

	class Application
	{
	public:
		Application(const WindowInformation& info);
		virtual ~Application();

		void Run();

		void EmitEvent(Event& event);

		template<typename TLayer>
			requires(std::is_base_of_v<ApplicationLayer, TLayer>)
		void PushLayer()
		{
			m_LayerStack.push_back(std::make_unique<TLayer>());
		}

		template<typename TLayer>
			requires(std::is_base_of_v<ApplicationLayer, TLayer>)
		TLayer* GetLayer()
		{
			for (const auto& layer : m_LayerStack)
			{
				if (auto casted = dynamic_cast<TLayer*>(layer.get()))
					return casted;
			}
			return nullptr;
		}

		const Slate::Window& GetWindow() const { return m_Window; }
		Slate::Renderer& GetRenderer() { return m_Renderer; }

		static Application& Get();

	protected:
		Slate::Window m_Window;
		Slate::Renderer m_Renderer;

	private:

		void Stop();

		bool m_Running = false;

		

		std::vector<std::unique_ptr<ApplicationLayer>> m_LayerStack;

		friend class ApplicationLayer;

	};

	Application* CreateApplication(Slate::WindowInformation windowInfo);
}


