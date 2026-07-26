#include "Slate/Core/ApplicationLayer.h"

#include "Slate/Core/Application.h"

namespace Slate
{
	void ApplicationLayer::QueueTransition(std::unique_ptr<ApplicationLayer> toLayer)
	{
		Application::Get().QueueLayerTransition(this, std::move(toLayer));
	}
}
