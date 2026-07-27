#pragma once

#include "Button.h"
#include "Image.h"
#include "Label.h"
#include "Slate/Input/InputEvents.h"

#include <memory>
#include <vector>

namespace Slate
{
	class Renderer;
	class UIElementVisitor;

	class UICanvas
	{
	public:
		Label& AddLabel(
			std::wstring text,
			const Rectangle2D& bounds,
			const TextStyle& style = {}
		);
		Button& AddButton(
			std::wstring text,
			const Rectangle2D& bounds,
			Button::ClickHandler onClick = {},
			const ButtonStyle& style = {}
		);
		Image& AddImage(
			const Texture2DHandle& texture,
			const Rectangle2D& bounds,
			float opacity = 1.0f
		);

		void OnEvent(Event& event);
		void Clear();

	private:
		void VisitElements(UIElementVisitor& visitor) const;

		bool OnMouseMoved(MouseMovedEvent& event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);
		bool OnWindowLostFocus(WindowLostFocusEvent& event);

		std::vector<std::unique_ptr<UIElement>> m_Elements;

		friend class Renderer;
	};
}
