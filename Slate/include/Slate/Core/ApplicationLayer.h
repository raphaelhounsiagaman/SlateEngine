#pragma once

#include "Slate/Input/Event.h"

#include <memory>

namespace Slate
{
	class ApplicationLayer
	{
	public:
		virtual ~ApplicationLayer() = default;

		virtual void OnEvent(Event& event) {}

		virtual void OnUpdate(float ts) {}
		virtual void OnRender() {}

		template<std::derived_from<ApplicationLayer> T, typename... Args>
		void TransitionTo(Args&&... args)
		{
			QueueTransition(std::move(std::make_unique<T>(std::forward<Args>(args)...)));
		}
	private:
		void QueueTransition(std::unique_ptr<ApplicationLayer> layer);
	};
}
