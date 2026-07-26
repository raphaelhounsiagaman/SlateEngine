#pragma once

#include "Event.h"
#include "KeyCodes.h"

#include <format>

namespace Slate
{
	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }
	protected:
		KeyEvent(KeyCode keycode)
			: m_KeyCode(keycode) {}

		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keycode, bool isRepeat)
			: KeyEvent(keycode), m_IsRepeat(isRepeat) {}

		inline bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override
		{
			return std::format(
				"KeyPressedEvent: {} (repeat={})",
				static_cast<unsigned int>(m_KeyCode),
				m_IsRepeat
			);
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			return std::format(
				"KeyReleasedEvent: {}",
				static_cast<unsigned int>(m_KeyCode)
			);
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
}


