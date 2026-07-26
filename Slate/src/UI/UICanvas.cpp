#include "Slate/UI/UICanvas.h"

#include "Slate/Input/Input.h"
#include "Slate/Input/InputEvents.h"

#include <ranges>
#include <utility>

namespace Slate
{
	Label& UICanvas::AddLabel(
		std::wstring text,
		const Rectangle2D& bounds,
		const TextStyle& style)
	{
		auto label = std::make_unique<Label>(
			std::move(text),
			bounds,
			style
		);
		Label& result = *label;
		m_Elements.push_back(std::move(label));
		return result;
	}

	Button& UICanvas::AddButton(
		std::wstring text,
		const Rectangle2D& bounds,
		Button::ClickHandler onClick,
		const ButtonStyle& style)
	{
		auto button = std::make_unique<Button>(
			std::move(text),
			bounds,
			std::move(onClick),
			style
		);
		Button& result = *button;
		m_Elements.push_back(std::move(button));
		return result;
	}

	Image& UICanvas::AddImage(
		const Texture2DHandle& texture,
		const Rectangle2D& bounds,
		float opacity)
	{
		auto image = std::make_unique<Image>(texture, bounds, opacity);
		Image& result = *image;
		m_Elements.push_back(std::move(image));
		return result;
	}

	void UICanvas::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseMovedEvent>(
			[this](MouseMovedEvent& mouseEvent)
			{
				return OnMouseMoved(mouseEvent);
			}
		);
		dispatcher.Dispatch<MouseButtonPressedEvent>(
			[this](MouseButtonPressedEvent& buttonEvent)
			{
				return OnMouseButtonPressed(buttonEvent);
			}
		);
		dispatcher.Dispatch<MouseButtonReleasedEvent>(
			[this](MouseButtonReleasedEvent& buttonEvent)
			{
				return OnMouseButtonReleased(buttonEvent);
			}
		);
		dispatcher.Dispatch<WindowLostFocusEvent>(
			[this](WindowLostFocusEvent& focusEvent)
			{
				return OnWindowLostFocus(focusEvent);
			}
		);
	}

	void UICanvas::Render(Renderer& renderer) const
	{
		for (const std::unique_ptr<UIElement>& element : m_Elements)
		{
			element->Render(renderer);
		}
	}

	void UICanvas::Clear()
	{
		m_Elements.clear();
	}

	bool UICanvas::OnMouseMoved(MouseMovedEvent& event)
	{
		for (const std::unique_ptr<UIElement>& element : m_Elements)
		{
			element->OnMouseMoved(event.GetPosition());
		}
		return false;
	}

	bool UICanvas::OnMouseButtonPressed(MouseButtonPressedEvent& event)
	{
		const Vector2i mousePosition = Input::GetMousePosition();
		for (const std::unique_ptr<UIElement>& element :
			m_Elements | std::views::reverse)
		{
			if (element->OnMouseButtonPressed(
				event.GetMouseButton(),
				mousePosition))
			{
				return true;
			}
		}
		return false;
	}

	bool UICanvas::OnMouseButtonReleased(MouseButtonReleasedEvent& event)
	{
		const Vector2i mousePosition = Input::GetMousePosition();
		for (const std::unique_ptr<UIElement>& element :
			m_Elements | std::views::reverse)
		{
			if (element->OnMouseButtonReleased(
				event.GetMouseButton(),
				mousePosition))
			{
				return true;
			}
		}
		return false;
	}

	bool UICanvas::OnWindowLostFocus(WindowLostFocusEvent&)
	{
		for (const std::unique_ptr<UIElement>& element : m_Elements)
		{
			element->CancelInteraction();
		}
		return false;
	}
}
