#include "Slate/Graphics/Window.h"

#include "Slate/Input/Input.h"
#include "Slate/Input/InputEvents.h"

#include <stdexcept>
#include <utility>
#include <windowsx.h>

namespace Slate
{
	namespace
	{
		KeyCode TranslateKeyCode(WPARAM wParam, LPARAM lParam)
		{
			UINT virtualKey = static_cast<UINT>(wParam);
			const UINT scanCode =
				(static_cast<UINT>(lParam) >> 16) & 0xFFu;
			const bool isExtended =
				(static_cast<UINT>(lParam) & (1u << 24)) != 0;

			if (virtualKey == VK_SHIFT)
			{
				virtualKey = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);
			}
			else if (virtualKey == VK_CONTROL)
			{
				virtualKey = isExtended ? VK_RCONTROL : VK_LCONTROL;
			}
			else if (virtualKey == VK_MENU)
			{
				virtualKey = isExtended ? VK_RMENU : VK_LMENU;
			}

			if (virtualKey > 0xFFu)
			{
				return KeyCode::Unknown;
			}

			return static_cast<KeyCode>(virtualKey);
		}

		MouseButton TranslateXButton(WPARAM wParam)
		{
			return GET_XBUTTON_WPARAM(wParam) == XBUTTON1
				? MouseButton::X1
				: MouseButton::X2;
		}

		bool IsAnyMouseButtonDown()
		{
			return
				Input::IsMouseButtonDown(MouseButton::Left) ||
				Input::IsMouseButtonDown(MouseButton::Right) ||
				Input::IsMouseButtonDown(MouseButton::Middle) ||
				Input::IsMouseButtonDown(MouseButton::X1) ||
				Input::IsMouseButtonDown(MouseButton::X2);
		}
	}

	void Window::SetEventCallback(EventCallback callback)
	{
		m_EventCallback = std::move(callback);
	}

	void Window::Create(const WindowInformation& info)
	{
		m_WindowInfo = info;
		m_WindowSize = info.WindowSize;
		m_ShouldClose = false;
		m_IsMinimized = false;

		WNDCLASSEXW windowClass{};
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProcedure;
		windowClass.hInstance = m_WindowInfo.InstanceHandle;
		windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		windowClass.lpszClassName = c_WindowClassName.c_str();

		if (RegisterClassExW(&windowClass) == 0)
		{
			throw std::runtime_error("Failed to register the window class.");
		}

		RECT windowRectangle
		{
			0,
			0,
			static_cast<LONG>(m_WindowInfo.WindowSize.X),
			static_cast<LONG>(m_WindowInfo.WindowSize.Y)
		};

		if (!AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, FALSE))
		{
			UnregisterClassW(c_WindowClassName.c_str(), m_WindowInfo.InstanceHandle);
			throw std::runtime_error("Failed to calculate the window size.");
		}

		const int completeWidth =
			windowRectangle.right - windowRectangle.left;
		const int completeHeight =
			windowRectangle.bottom - windowRectangle.top;

		m_WindowHandle = CreateWindowExW(
			0,
			c_WindowClassName.c_str(),
			m_WindowInfo.Name.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			completeWidth,
			completeHeight,
			nullptr,
			nullptr,
			m_WindowInfo.InstanceHandle,
			this
		);

		if (m_WindowHandle == nullptr)
		{
			UnregisterClassW(c_WindowClassName.c_str(), m_WindowInfo.InstanceHandle);
			throw std::runtime_error("Failed to create the window.");
		}

		ShowWindow(m_WindowHandle, SW_SHOW);
		UpdateWindow(m_WindowHandle);
	}

	void Window::Destroy()
	{
		if (m_WindowHandle != nullptr && IsWindow(m_WindowHandle))
		{
			DestroyWindow(m_WindowHandle);
		}

		m_WindowHandle = nullptr;
		UnregisterClassW(
			c_WindowClassName.c_str(),
			m_WindowInfo.InstanceHandle
		);
	}

	void Window::ProcessEvents()
	{
		MSG message{};
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
			{
				m_ShouldClose = true;
				continue;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}

	void Window::WaitForEvents() const
	{
		WaitMessage();
	}

	LRESULT CALLBACK Window::WindowProcedure(
		HWND windowHandle,
		UINT message,
		WPARAM wParam,
		LPARAM lParam)
	{
		Window* window = reinterpret_cast<Window*>(
			GetWindowLongPtrW(windowHandle, GWLP_USERDATA)
		);

		if (message == WM_NCCREATE)
		{
			const auto* creationData =
				reinterpret_cast<const CREATESTRUCTW*>(lParam);
			window = static_cast<Window*>(creationData->lpCreateParams);

			SetWindowLongPtrW(
				windowHandle,
				GWLP_USERDATA,
				reinterpret_cast<LONG_PTR>(window)
			);
			window->m_WindowHandle = windowHandle;
		}

		if (window != nullptr)
		{
			return window->HandleMessage(message, wParam, lParam);
		}

		return DefWindowProcW(windowHandle, message, wParam, lParam);
	}

	LRESULT Window::HandleMessage(
		UINT message,
		WPARAM wParam,
		LPARAM lParam)
	{
		switch (message)
		{
		case WM_CLOSE:
		{
			WindowCloseEvent event;
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}

			m_ShouldClose = true;
			return 0;
		}

		case WM_DESTROY:
			m_ShouldClose = true;
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
		{
			m_WindowSize = { LOWORD(lParam), HIWORD(lParam) };
			m_IsMinimized =
				wParam == SIZE_MINIMIZED ||
				m_WindowSize.X == 0 ||
				m_WindowSize.Y == 0;

			WindowResizeEvent event(
				m_WindowSize.X,
				m_WindowSize.Y
			);
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return 0;
		}

		case WM_SETFOCUS:
		{
			WindowFocusEvent event;
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return 0;
		}

		case WM_KILLFOCUS:
		{
			Input::Reset();

			WindowLostFocusEvent event;
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return 0;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			const KeyCode key = TranslateKeyCode(wParam, lParam);
			const bool isRepeat = (lParam & (1LL << 30)) != 0;
			Input::SetKeyState(key, true);

			KeyPressedEvent event(key, isRepeat);
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return message == WM_SYSKEYDOWN
				? DefWindowProcW(m_WindowHandle, message, wParam, lParam)
				: 0;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			const KeyCode key = TranslateKeyCode(wParam, lParam);
			Input::SetKeyState(key, false);

			KeyReleasedEvent event(key);
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return message == WM_SYSKEYUP
				? DefWindowProcW(m_WindowHandle, message, wParam, lParam)
				: 0;
		}

		case WM_MOUSEMOVE:
		{
			const Vector2i position
			{
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam)
			};
			const Vector2i delta = Input::SetMousePosition(position);

			MouseMovedEvent event(
				position.X,
				position.Y,
				delta.X,
				delta.Y
			);
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return 0;
		}

		case WM_LBUTTONDOWN:
			SetCapture(m_WindowHandle);
			Input::SetMouseButtonState(MouseButton::Left, true);
			if (m_EventCallback)
			{
				MouseButtonPressedEvent event(MouseButton::Left);
				m_EventCallback(event);
			}
			return 0;

		case WM_LBUTTONUP:
			Input::SetMouseButtonState(MouseButton::Left, false);
			if (m_EventCallback)
			{
				MouseButtonReleasedEvent event(MouseButton::Left);
				m_EventCallback(event);
			}
			if (!IsAnyMouseButtonDown())
			{
				ReleaseCapture();
			}
			return 0;

		case WM_RBUTTONDOWN:
			SetCapture(m_WindowHandle);
			Input::SetMouseButtonState(MouseButton::Right, true);
			if (m_EventCallback)
			{
				MouseButtonPressedEvent event(MouseButton::Right);
				m_EventCallback(event);
			}
			return 0;

		case WM_RBUTTONUP:
			Input::SetMouseButtonState(MouseButton::Right, false);
			if (m_EventCallback)
			{
				MouseButtonReleasedEvent event(MouseButton::Right);
				m_EventCallback(event);
			}
			if (!IsAnyMouseButtonDown())
			{
				ReleaseCapture();
			}
			return 0;

		case WM_MBUTTONDOWN:
			SetCapture(m_WindowHandle);
			Input::SetMouseButtonState(MouseButton::Middle, true);
			if (m_EventCallback)
			{
				MouseButtonPressedEvent event(MouseButton::Middle);
				m_EventCallback(event);
			}
			return 0;

		case WM_MBUTTONUP:
			Input::SetMouseButtonState(MouseButton::Middle, false);
			if (m_EventCallback)
			{
				MouseButtonReleasedEvent event(MouseButton::Middle);
				m_EventCallback(event);
			}
			if (!IsAnyMouseButtonDown())
			{
				ReleaseCapture();
			}
			return 0;

		case WM_XBUTTONDOWN:
		{
			const MouseButton button = TranslateXButton(wParam);
			SetCapture(m_WindowHandle);
			Input::SetMouseButtonState(button, true);
			if (m_EventCallback)
			{
				MouseButtonPressedEvent event(button);
				m_EventCallback(event);
			}
			return TRUE;
		}

		case WM_XBUTTONUP:
		{
			const MouseButton button = TranslateXButton(wParam);
			Input::SetMouseButtonState(button, false);
			if (m_EventCallback)
			{
				MouseButtonReleasedEvent event(button);
				m_EventCallback(event);
			}
			if (!IsAnyMouseButtonDown())
			{
				ReleaseCapture();
			}
			return TRUE;
		}

		case WM_MOUSEWHEEL:
		{
			const float offset =
				static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) /
				static_cast<float>(WHEEL_DELTA);
			Input::AddMouseScroll({ 0.0f, offset });

			MouseScrolledEvent event(0.0f, offset);
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return 0;
		}

		case WM_MOUSEHWHEEL:
		{
			const float offset =
				static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) /
				static_cast<float>(WHEEL_DELTA);
			Input::AddMouseScroll({ offset, 0.0f });

			MouseScrolledEvent event(offset, 0.0f);
			if (m_EventCallback)
			{
				m_EventCallback(event);
			}
			return 0;
		}
		}

		return DefWindowProcW(m_WindowHandle, message, wParam, lParam);
	}
}
