#pragma once

#include "Slate/Graphics/Window.h"
#include "Slate/Graphics/Renderer.h"

#include "Slate/Input/InputEvents.h"
#include "ApplicationLayer.h"
#include "PerformanceStatistics.h"

#include <memory>
#include <type_traits>
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
			std::unique_ptr<TLayer> layer = std::make_unique<TLayer>();
			layer->OnAttach();
			m_LayerStack.push_back(std::move(layer));
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

		const PerformanceStatistics& GetPerformanceStatistics() const
		{
			return m_PerformanceStatistics;
		}
		const ApplicationLoopSettings& GetLoopSettings() const
		{
			return m_LoopSettings;
		}
		void SetLoopSettings(const ApplicationLoopSettings& settings);

		static Application& Get();

	protected:
		Slate::Window m_Window;
		Slate::Renderer m_Renderer;

	private:

		void Stop();
		void QueueLayerTransition(
			ApplicationLayer* source,
			std::unique_ptr<ApplicationLayer> destination
		);
		void ProcessLayerOperations();
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

		struct LayerTransition
		{
			ApplicationLayer* Source = nullptr;
			std::unique_ptr<ApplicationLayer> Destination;
		};

		bool m_Running = false;
		ApplicationLoopSettings m_LoopSettings;
		PerformanceStatistics m_PerformanceStatistics;

		std::vector<std::unique_ptr<ApplicationLayer>> m_LayerStack;
		std::vector<LayerTransition> m_PendingLayerTransitions;

		friend class ApplicationLayer;

	};

	std::unique_ptr<Application> CreateApplication(
		Slate::WindowInformation windowInfo
	);
}


