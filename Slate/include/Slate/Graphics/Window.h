#pragma once

#include "Slate/Input/Event.h"
#include "Slate/Math/Vector.h"

#include <Windows.h>

#include <functional>
#include <string>
#include <utility>

namespace Slate
{
	struct WindowInformation
	{
		WindowInformation() = default;
		WindowInformation(
			std::wstring name,
			Vector2iu windowSize,
			HINSTANCE instance,
			PWSTR commandLine,
			int showCommand)
			: Name(std::move(name)),
				WindowSize(windowSize),
				InstanceHandle(instance),
				CommandLine(commandLine),
				ShowCommand(showCommand)
		{}

		std::wstring Name = L"Slate Application";
		Vector2iu WindowSize{ 1280, 720 };

	private:
		HINSTANCE InstanceHandle{};
		PWSTR CommandLine{};
		int ShowCommand{};

		friend class Window;
	};

	class Window
	{
	public:
		using EventCallback = std::function<void(Event&)>;

		Window() = default;
		~Window() = default;

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		void SetEventCallback(EventCallback callback);
		void Create(const WindowInformation& info);
		void Destroy();

		void ProcessEvents();
		void WaitForEvents() const;

		bool ShouldClose() const { return m_ShouldClose; }
		bool IsMinimized() const { return m_IsMinimized; }

		HWND GetHandle() const { return m_WindowHandle; }

		Vector2iu GetWindowSize() const { return m_WindowSize; }

	private:

		static LRESULT CALLBACK WindowProcedure(
			HWND hwnd,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);

		LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

	private:

		const std::wstring c_WindowClassName = L"SlateWindowClass";

		WindowInformation m_WindowInfo{};
		HWND m_WindowHandle = nullptr;

		bool m_ShouldClose = false;
		bool m_IsMinimized = false;

		Vector2iu m_WindowSize{ 0, 0 };

		EventCallback m_EventCallback;
	};
}
