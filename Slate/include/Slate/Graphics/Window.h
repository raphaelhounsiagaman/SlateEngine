#pragma once

#include <Windows.h>

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
		Window() = default;
		~Window() = default;

		void Create(const WindowInformation& info);
		void Destroy();

		void ProcessEvents();

		bool ShouldClose() const { return m_ShouldClose; }

		bool WasResized() const { return m_WasResized; }
		void ClearResizedFlag() { m_WasResized = false; }

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

	private:

		const std::wstring c_WindowClassName = L"SlateWindowClass";

		WindowInformation m_WindowInfo{};
		HWND m_WindowHandle = nullptr;

		bool m_ShouldClose = false;
		bool m_WasResized = false;

		// TODO: UPDATE THIS TO USE VECTOR
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;

	};
}