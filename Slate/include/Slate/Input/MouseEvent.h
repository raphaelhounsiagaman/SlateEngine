#pragma once

#include "Slate/Math/Vector.h"

#include "Event.h"
#include "MouseCodes.h"

#include <format>

namespace Slate
{

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int x, int y, int deltaX, int deltaY)
			: m_MousePosition{ x, y }, m_MouseDelta{ deltaX, deltaY }
		{}

		Vector2i GetPosition() const { return m_MousePosition; }
		Vector2i GetDelta() const { return m_MouseDelta; }

		std::string ToString() const override
		{
			return std::format("MouseMovedEvent: {}, {}", m_MousePosition.X, m_MousePosition.Y);
		}

		EVENT_CLASS_TYPE(MouseMoved)
	private:
		Vector2i m_MousePosition;
		Vector2i m_MouseDelta;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_Offset(xOffset, yOffset) 
		{}

		Vector2f GetOffset() const { return m_Offset; }

		std::string ToString() const override
		{
			return std::format("MouseScrolledEvent: {}, {}", m_Offset.X, m_Offset.Y);
		}

		EVENT_CLASS_TYPE(MouseScrolled)
	private:
		Vector2f m_Offset{};

	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseButton GetMouseButton() const { return m_Button; }
	protected:
		MouseButtonEvent(MouseButton button)
			: m_Button(button) {}

		MouseButton m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseButton button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			return std::format(
				"MouseButtonPressedEvent: {}",
				static_cast<unsigned int>(m_Button)
			);
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseButton button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			return std::format(
				"MouseButtonReleasedEvent: {}",
				static_cast<unsigned int>(m_Button)
			);
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}
