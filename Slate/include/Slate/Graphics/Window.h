#pragma once

#include "Slate/Input/Event.h"

#include <Windows.h>

#include <functional>
#include <string>

namespace Slate
{
	struct WindowInformation
	{
		WindowInformation() = default;
		WindowInformation(std::wstring name, unsigned int width, unsigned int height, HINSTANCE instance, PWSTR commandLine, int showCommand)
			: Name(name), Width(width), Height(height), InstanceHandle(instance), CommandLine(commandLine), ShowCommand(showCommand)
		{}

		std::wstring Name = L"Slate Application";
		// Client-area dimensions are measured in screen pixels.
		unsigned int Width = 1280;
		unsigned int Height = 720;

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

		// TODO: UPDATE THIS TO USE VECTOR 
		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }

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

		// TODO: UPDATE THIS TO USE VECTOR
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;

		EventCallback m_EventCallback;
	};
}
