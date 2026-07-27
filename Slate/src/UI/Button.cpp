#include "Slate/UI/Button.h"

#include "UIElementVisitor.h"

#include <utility>

namespace Slate
{
	Button::Button(
		std::wstring text,
		const Rectangle2D& bounds,
		ClickHandler onClick,
		const ButtonStyle& style)
		: m_Text(std::move(text)),
		  m_OnClick(std::move(onClick)),
		  m_Style(style)
	{
		m_Bounds = bounds;
	}

	void Button::SetText(std::wstring text)
	{
		m_Text = std::move(text);
	}

	void Button::SetOnClick(ClickHandler onClick)
	{
		m_OnClick = std::move(onClick);
	}

	void Button::Accept(UIElementVisitor& visitor) const
	{
		visitor.Visit(*this);
	}

	bool Button::OnMouseMoved(const Vector2i& positionPixels)
	{
		m_IsHovered =
			m_IsVisible &&
			m_IsEnabled &&
			m_Bounds.Contains({
				static_cast<float>(positionPixels.X),
				static_cast<float>(positionPixels.Y)
			});
		return false;
	}

	bool Button::OnMouseButtonPressed(
		MouseButton button,
		const Vector2i& positionPixels)
	{
		if (button != MouseButton::Left || !m_IsVisible || !m_IsEnabled)
		{
			return false;
		}

		m_IsHovered = m_Bounds.Contains({
			static_cast<float>(positionPixels.X),
			static_cast<float>(positionPixels.Y)
			});
		m_IsPressed = m_IsHovered;
		return m_IsPressed;
	}

	bool Button::OnMouseButtonReleased(
		MouseButton button,
		const Vector2i& positionPixels)
	{
		if (button != MouseButton::Left || !m_IsPressed)
		{
			return false;
		}

		const bool shouldClick =
			m_IsVisible &&
			m_IsEnabled &&
			m_Bounds.Contains({
				static_cast<float>(positionPixels.X),
				static_cast<float>(positionPixels.Y)
				});
		m_IsPressed = false;
		m_IsHovered = shouldClick;

		if (shouldClick && m_OnClick)
		{
			m_OnClick();
		}
		return true;
	}

	void Button::CancelInteraction()
	{
		m_IsHovered = false;
		m_IsPressed = false;
	}
}
