#pragma once

#include "Slate/Math/Vector.h"

#include "Event.h"

#include <format>

namespace Slate
{

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(unsigned int x, unsigned int y)
			: m_MousePosition(x, y) {}

		inline Vector2<unsigned int> GetPosition() const { return m_MousePosition; }
		inline unsigned int GetX() const { return m_MousePosition.X; }
		inline unsigned int GetY() const { return m_MousePosition.Y; }

		std::string ToString() const override
		{
			return std::format("MouseMovedEvent: {}, {}", m_MousePosition.X, m_MousePosition.Y);
		}

		EVENT_CLASS_TYPE(MouseMoved)
	private:
		Vector2<unsigned int> m_MousePosition;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(int xOffset, int yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		inline int GetXOffset() const { return m_XOffset; }
		inline int GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			return std::format("MouseScrolledEvent: {}, {}", m_XOffset, m_YOffset);
		}

		EVENT_CLASS_TYPE(MouseScrolled)
	private:
		int m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline int GetMouseButton() const { return m_Button; }
	protected:
		MouseButtonEvent(int button)
			: m_Button(button) {}

		int m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			return std::format("MouseButtonPressedEvent: {}", m_Button);
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			return std::format("MouseButtonReleasedEvent: {}", m_Button);
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}