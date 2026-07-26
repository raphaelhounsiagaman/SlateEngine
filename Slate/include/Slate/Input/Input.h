#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"
#include "Slate/Math/Vector.h"

namespace Slate
{
	class Application;
	class Window;

	// Frame-based input state. "Pressed" and "Released" are true only during the
	// frame in which the transition occurred; "Down" remains true while held.
	class Input final
	{
	public:
		Input() = delete;

		static bool IsKeyDown(KeyCode key);
		static bool IsKeyPressed(KeyCode key);
		static bool IsKeyReleased(KeyCode key);

		static bool IsMouseButtonDown(MouseButton button);
		static bool IsMouseButtonPressed(MouseButton button);
		static bool IsMouseButtonReleased(MouseButton button);

		static Vector2i GetMousePosition();
		static Vector2i GetMouseDelta();
		static Vector2f GetScrollDelta();

	private:
		static void BeginFrame();
		static void SetKeyState(KeyCode key, bool isDown);
		static void SetMouseButtonState(MouseButton button, bool isDown);
		static Vector2i SetMousePosition(const Vector2i& position);
		static void AddMouseScroll(const Vector2f& offset);
		static void Reset();

		friend class Application;
		friend class Window;
	};
}
