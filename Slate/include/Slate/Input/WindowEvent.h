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
			: m_Width(width), m_Height(height)
		{}

		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override
		{
			return std::format("WindowResizeEvent: {}, {}", m_Width, m_Height);
		}

		EVENT_CLASS_TYPE(WindowResize)

	private:
		unsigned int m_Width;
		unsigned int m_Height;
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
