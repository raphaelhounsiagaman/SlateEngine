#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"
#include "Slate/Math/Vector.h"

namespace Slate
{
	class Application;
	class Window;

	// Update-based input state. "Pressed" and "Released" remain true until the
	// next completed update; "Down" remains true while held.
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

		static Vector2i GetMousePosition(); // Client-area pixels.
		static Vector2i GetMouseDelta();    // Pixels accumulated this frame.
		static Vector2f GetScrollDelta();

	private:
		static void EndUpdate();
		static void SetKeyState(KeyCode key, bool isDown);
		static void SetMouseButtonState(MouseButton button, bool isDown);
		static Vector2i SetMousePosition(const Vector2i& position);
		static void AddMouseScroll(const Vector2f& offset);
		static void Reset();

		friend class Application;
		friend class Window;
	};
}
