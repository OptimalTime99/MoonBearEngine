/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : Win32 창 등록, 생성, 메시지 처리와 자원 정리 구현
 * Windows 메시지를 FWindowsWindow 객체와 연결하고 크기 변경을 기록
 */

#include "Platform/Windows/WindowsWindow.h"

#include "Platform/Windows/WindowsInput.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

namespace Engine
{
    FWindowsWindow::FWindowsWindow(FWindowsInput& InInput) noexcept
        : Input(InInput)
    {
    }

    FWindowsWindow::~FWindowsWindow()
    {
        if (WindowHandle != nullptr)
        {
            ::DestroyWindow(WindowHandle);
            WindowHandle = nullptr;
        }

        if (WindowClassAtom != 0 && InstanceHandle != nullptr)
        {
            ::UnregisterClassW(WindowClassName, InstanceHandle);

            WindowClassAtom = 0;
        }
    }

    bool FWindowsWindow::Initialize(const wchar_t* Title,
                                    const std::uint32_t InClientWidth,
                                    const std::uint32_t InClientHeight)
    {
        ENGINE_ASSERT(Title != nullptr);
        ENGINE_ASSERT(InClientWidth > 0);
        ENGINE_ASSERT(InClientHeight > 0);
        ENGINE_ASSERT(WindowHandle == nullptr);

        InstanceHandle = ::GetModuleHandleW(nullptr);

        if (InstanceHandle == nullptr)
        {
            FLog::Error("Failed to get the application instance.");
            return false;
        }

        WNDCLASSEXW WindowClass{};
        WindowClass.cbSize = sizeof(WNDCLASSEXW);
        WindowClass.style = CS_HREDRAW | CS_VREDRAW;
        WindowClass.lpfnWndProc = &FWindowsWindow::WindowProcedure;
        WindowClass.hInstance = InstanceHandle;
        WindowClass.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
        WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<
            INT_PTR>(COLOR_WINDOW + 1));
        WindowClass.lpszClassName = WindowClassName;

        WindowClassAtom = ::RegisterClassExW(&WindowClass);

        if (WindowClassAtom == 0)
        {
            FLog::Error("Failed to register the window class.");
            return false;
        }

        constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW;

        RECT WindowRectangle{
            0, 0, static_cast<LONG>(InClientWidth),
            static_cast<LONG>(InClientHeight)
        };

        if (!::AdjustWindowRect(&WindowRectangle, WindowStyle, FALSE))
        {
            FLog::Error("Failed to calculate the window size.");
            return false;
        }

        const int WindowWidth = WindowRectangle.right - WindowRectangle.left;

        const int WindowHeight = WindowRectangle.bottom - WindowRectangle.top;

        WindowHandle = ::CreateWindowExW(0, WindowClassName, Title, WindowStyle,
                                         CW_USEDEFAULT, CW_USEDEFAULT,
                                         WindowWidth, WindowHeight, nullptr,
                                         nullptr, InstanceHandle, this);

        if (WindowHandle == nullptr)
        {
            FLog::Error("Failed to create the window.");
            return false;
        }

        ::ShowWindow(WindowHandle, SW_SHOW);
        ::UpdateWindow(WindowHandle);

        RECT ClientRectangle{};

        if (!::GetClientRect(WindowHandle, &ClientRectangle))
        {
            FLog::Error("Failed to get the client area.");
            return false;
        }

        ClientWidth = static_cast<std::uint32_t>(ClientRectangle.right -
            ClientRectangle.left);

        ClientHeight = static_cast<std::uint32_t>(ClientRectangle.bottom -
            ClientRectangle.top);

        /*
         * 창 생성 과정에서 발생한 WM_SIZE는 현재 크기에 이미 반영됐다.
         * 렌더러는 이 크기로 처음 생성되므로 별도 Resize가 필요하지 않다.
         */
        bResizeRequested = false;

        FLog::Info("Window initialized.");

        return true;
    }

    void FWindowsWindow::ProcessMessages()
    {
        MSG Message{};

        while (::PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE))
        {
            if (Message.message == WM_QUIT)
            {
                bShouldClose = true;
                continue;
            }

            ::TranslateMessage(&Message);
            ::DispatchMessageW(&Message);
        }
    }

    void FWindowsWindow::RequestClose()
    {
        bShouldClose = true;

        if (WindowHandle != nullptr)
        {
            ::DestroyWindow(WindowHandle);
        }
    }

    bool FWindowsWindow::ShouldClose() const noexcept
    {
        return bShouldClose;
    }

    HWND FWindowsWindow::GetNativeHandle() const noexcept
    {
        return WindowHandle;
    }

    std::uint32_t FWindowsWindow::GetClientWidth() const noexcept
    {
        return ClientWidth;
    }

    std::uint32_t FWindowsWindow::GetClientHeight() const noexcept
    {
        return ClientHeight;
    }

    bool FWindowsWindow::ConsumeResizeRequest(std::uint32_t& OutWidth,
                                              std::uint32_t& OutHeight) noexcept
    {
        if (!bResizeRequested)
        {
            return false;
        }

        OutWidth = ClientWidth;
        OutHeight = ClientHeight;
        bResizeRequested = false;

        return true;
    }

    LRESULT CALLBACK FWindowsWindow::WindowProcedure(
        const HWND InWindowHandle, const UINT Message, const WPARAM WParam,
        const LPARAM LParam)
    {
        FWindowsWindow* Window = nullptr;

        if (Message == WM_NCCREATE)
        {
            const auto* CreateData = reinterpret_cast<CREATESTRUCTW*>(LParam);

            Window = static_cast<FWindowsWindow*>(CreateData->lpCreateParams);

            ::SetWindowLongPtrW(InWindowHandle, GWLP_USERDATA,
                                reinterpret_cast<LONG_PTR>(Window));

            Window->WindowHandle = InWindowHandle;
        }
        else
        {
            Window = reinterpret_cast<FWindowsWindow*>(::GetWindowLongPtrW(
                InWindowHandle, GWLP_USERDATA));
        }

        if (Window != nullptr)
        {
            return Window->HandleMessage(InWindowHandle, Message, WParam,
                                         LParam);
        }

        return ::DefWindowProcW(InWindowHandle, Message, WParam, LParam);
    }

    LRESULT FWindowsWindow::HandleMessage(const HWND InWindowHandle,
                                          const UINT Message,
                                          const WPARAM WParam,
                                          const LPARAM LParam)
    {
        switch (Message)
        {
        case WM_CLOSE:
            RequestClose();
            return 0;

        case WM_DESTROY:
            bShouldClose = true;
            ::PostQuitMessage(0);
            return 0;

        case WM_NCDESTROY:
            /*
             * 운영체제가 창을 완전히 제거한 뒤에는 해당 HWND를
             * 다시 DestroyWindow에 전달하지 않도록 상태를 비운다.
             */
            ::SetWindowLongPtrW(InWindowHandle, GWLP_USERDATA, 0);

            WindowHandle = nullptr;

            return ::DefWindowProcW(InWindowHandle, Message, WParam, LParam);

        case WM_SIZE:
            {
                if (WParam == SIZE_MINIMIZED)
                {
                    return 0;
                }

                const std::uint32_t NewWidth = static_cast<std::uint32_t>(
                    LOWORD(LParam));

                const std::uint32_t NewHeight = static_cast<std::uint32_t>(
                    HIWORD(LParam));

                if (NewWidth == 0 || NewHeight == 0)
                {
                    return 0;
                }

                ClientWidth = NewWidth;
                ClientHeight = NewHeight;
                bResizeRequested = true;

                return 0;
            }

        case WM_KEYDOWN:
            Input.SetKeyState(static_cast<std::uint32_t>(WParam), true);
            return 0;

        case WM_KEYUP:
            Input.SetKeyState(static_cast<std::uint32_t>(WParam), false);
            return 0;

        case WM_KILLFOCUS:
            /*
             * 키를 누른 상태에서 포커스를 잃으면 WM_KEYUP을 받지
             * 못할 수 있으므로 모든 키 상태를 해제한다.
             */
            Input.Reset();
            return 0;

        default:
            break;
        }

        return ::DefWindowProcW(InWindowHandle, Message, WParam, LParam);
    }
}
