#pragma once

#include "Event.h"

#include <format>

namespace Slate
{
	class WindowCloseEvent : public Event
	{
	public:
		EVENT_CLASS_TYPE(WindowClose)
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_WindowSize(width, height)
		{}

		Vector2iu GetWindowSize() const { return m_WindowSize; }

		std::string ToString() const override
		{
			return std::format("WindowResizeEvent: {}, {}", m_WindowSize.X, m_WindowSize.Y);
		}

		EVENT_CLASS_TYPE(WindowResize)

	private:
		Vector2iu m_WindowSize{};

	};

	class WindowFocusEvent : public Event
	{
	public:
		EVENT_CLASS_TYPE(WindowFocus)
	};

	class WindowLostFocusEvent : public Event
	{
	public:
		EVENT_CLASS_TYPE(WindowLostFocus)
	};
}
