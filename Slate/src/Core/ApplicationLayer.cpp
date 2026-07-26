#include "Slate/Core/ApplicationLayer.h"

#include "Slate/Core/Application.h"

namespace Slate
{
	void ApplicationLayer::QueueTransition(std::unique_ptr<ApplicationLayer> toLayer)
	{
		// TODO: don't do this
		auto& layerStack = Application::Get().m_LayerStack;
		for (auto& layer : layerStack)
		{
			if (layer.get() == this)
			{
				layer = std::move(toLayer);
				return;
			}
		}

	}


}