#include "Slate/Graphics/Window.h"

#include <stdexcept>

void Slate::Window::Create(const WindowInformation& info)
{
    m_WindowInfo = info;

	m_Width = info.Width;
	m_Height = info.Height;

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
        (long)m_WindowInfo.Width,
        (long)m_WindowInfo.Height
    };

    AdjustWindowRect(
        &windowRectangle,
        WS_OVERLAPPEDWINDOW,
        FALSE
    );

    const int completeWidth = windowRectangle.right - windowRectangle.left;
    const int completeHeight = windowRectangle.bottom - windowRectangle.top;

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
        nullptr
    );

    if (m_WindowHandle == nullptr)
    {
        throw std::runtime_error("Failed to create the window.");
    }

    ShowWindow(m_WindowHandle, SW_SHOW);
    UpdateWindow(m_WindowHandle);
}

void Slate::Window::Destroy()
{
	
}

void Slate::Window::ProcessEvents()
{
    MSG message{};

    while (PeekMessageW(
        &message,
        nullptr,
        0,
        0,
        PM_REMOVE))
    {

        switch (message.message)
        {
        case WM_SIZE:
        {
            m_Width = LOWORD(message.lParam);
            m_Height = HIWORD(message.lParam);
            m_WasResized = true;
            break;
        }
        case WM_QUIT:
        {
            m_ShouldClose = true;
            return;
        }
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

}

LRESULT CALLBACK Slate::Window::WindowProcedure(
    HWND windowHandle,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
    {
        DestroyWindow(windowHandle);
        return 0;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    default:
    {
        return DefWindowProcW(
            windowHandle,
            message,
            wParam,
            lParam
        );
    }
    }
}
