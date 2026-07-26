#pragma once

#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace Slate
{

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

	class Event
	{
	public:
		bool Handled = false;

		virtual ~Event() = default;
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }
	};

	class EventDispatcher
	{
	public:
		explicit EventDispatcher(Event& event)
			: m_Event(event) {}

		template<typename TEvent, typename TFunction>
			requires std::is_base_of_v<Event, TEvent>
		bool Dispatch(TFunction&& function)
		{
			if (m_Event.GetEventType() == TEvent::GetStaticType() &&
				!m_Event.Handled)
			{
				m_Event.Handled = std::invoke(
					std::forward<TFunction>(function),
					static_cast<TEvent&>(m_Event)
				);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};


}
