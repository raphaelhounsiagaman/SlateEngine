#include "Slate/Input/Input.h"

#include <array>
#include <cstddef>
#include <type_traits>

namespace Slate
{
	namespace
	{
		struct ButtonState
		{
			bool IsDown = false;
			bool WasPressed = false;
			bool WasReleased = false;
		};

		constexpr std::size_t KeyCount = 256;
		constexpr std::size_t MouseButtonCount =
			static_cast<std::size_t>(MouseButton::Count);

		std::array<ButtonState, KeyCount> s_Keys{};
		std::array<ButtonState, MouseButtonCount> s_MouseButtons{};
		Vector2i s_MousePosition{};
		Vector2i s_MouseDelta{};
		Vector2f s_ScrollDelta{};
		bool s_HasMousePosition = false;

		template<typename T>
		constexpr std::size_t ToIndex(T value)
		{
			return static_cast<std::size_t>(
				static_cast<std::underlying_type_t<T>>(value)
			);
		}

		const ButtonState& GetKeyState(KeyCode key)
		{
			static const ButtonState EmptyState{};
			const std::size_t index = ToIndex(key);
			return index < s_Keys.size() ? s_Keys[index] : EmptyState;
		}

		const ButtonState& GetMouseButtonState(MouseButton button)
		{
			static const ButtonState EmptyState{};
			const std::size_t index = ToIndex(button);
			return index < s_MouseButtons.size()
				? s_MouseButtons[index]
				: EmptyState;
		}

		template<typename TSelector>
		bool QueryKey(KeyCode key, TSelector selector)
		{
			if (key == KeyCode::Shift)
			{
				return selector(GetKeyState(KeyCode::LeftShift)) ||
					selector(GetKeyState(KeyCode::RightShift));
			}

			if (key == KeyCode::Control)
			{
				return selector(GetKeyState(KeyCode::LeftControl)) ||
					selector(GetKeyState(KeyCode::RightControl));
			}

			if (key == KeyCode::Alt)
			{
				return selector(GetKeyState(KeyCode::LeftAlt)) ||
					selector(GetKeyState(KeyCode::RightAlt));
			}

			return selector(GetKeyState(key));
		}
	}

	bool Input::IsKeyDown(KeyCode key)
	{
		return QueryKey(key, [](const ButtonState& state)
		{
			return state.IsDown;
		});
	}

	bool Input::IsKeyPressed(KeyCode key)
	{
		return QueryKey(key, [](const ButtonState& state)
		{
			return state.WasPressed;
		});
	}

	bool Input::IsKeyReleased(KeyCode key)
	{
		return QueryKey(key, [](const ButtonState& state)
		{
			return state.WasReleased;
		});
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		return GetMouseButtonState(button).IsDown;
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return GetMouseButtonState(button).WasPressed;
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		return GetMouseButtonState(button).WasReleased;
	}

	Vector2i Input::GetMousePosition()
	{
		return s_MousePosition;
	}

	Vector2i Input::GetMouseDelta()
	{
		return s_MouseDelta;
	}

	Vector2f Input::GetScrollDelta()
	{
		return s_ScrollDelta;
	}

	void Input::EndUpdate()
	{
		for (ButtonState& state : s_Keys)
		{
			state.WasPressed = false;
			state.WasReleased = false;
		}

		for (ButtonState& state : s_MouseButtons)
		{
			state.WasPressed = false;
			state.WasReleased = false;
		}

		s_MouseDelta = {};
		s_ScrollDelta = {};
	}

	void Input::SetKeyState(KeyCode key, bool isDown)
	{
		const std::size_t index = ToIndex(key);
		if (key == KeyCode::Unknown || index >= s_Keys.size())
		{
			return;
		}

		ButtonState& state = s_Keys[index];
		if (state.IsDown == isDown)
		{
			return;
		}

		state.IsDown = isDown;
		state.WasPressed |= isDown;
		state.WasReleased |= !isDown;
	}

	void Input::SetMouseButtonState(MouseButton button, bool isDown)
	{
		const std::size_t index = ToIndex(button);
		if (index >= s_MouseButtons.size())
		{
			return;
		}

		ButtonState& state = s_MouseButtons[index];
		if (state.IsDown == isDown)
		{
			return;
		}

		state.IsDown = isDown;
		state.WasPressed |= isDown;
		state.WasReleased |= !isDown;
	}

	Vector2i Input::SetMousePosition(const Vector2i& position)
	{
		Vector2i movement{};
		if (s_HasMousePosition)
		{
			movement = position - s_MousePosition;
			s_MouseDelta = s_MouseDelta + movement;
		}

		s_MousePosition = position;
		s_HasMousePosition = true;
		return movement;
	}

	void Input::AddMouseScroll(const Vector2f& offset)
	{
		s_ScrollDelta = s_ScrollDelta + offset;
	}

	void Input::Reset()
	{
		for (ButtonState& state : s_Keys)
		{
			if (state.IsDown)
			{
				state.IsDown = false;
				state.WasReleased = true;
			}
		}

		for (ButtonState& state : s_MouseButtons)
		{
			if (state.IsDown)
			{
				state.IsDown = false;
				state.WasReleased = true;
			}
		}

		s_MouseDelta = {};
		s_ScrollDelta = {};
		s_HasMousePosition = false;
	}
}
