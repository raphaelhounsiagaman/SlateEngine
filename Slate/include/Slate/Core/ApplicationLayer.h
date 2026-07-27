#pragma once

#include "Slate/Input/Event.h"

#include <concepts>
#include <memory>
#include <utility>

namespace Slate
{
	class ApplicationLayer
	{
	public:
		virtual ~ApplicationLayer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnEvent(Event& event) {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnRender() {}

		template<std::derived_from<ApplicationLayer> T, typename... Args>
		void TransitionTo(Args&&... args)
		{
			QueueTransition(
				std::make_unique<T>(std::forward<Args>(args)...)
			);
		}

	private:
		void QueueTransition(std::unique_ptr<ApplicationLayer> layer);
	};
}
