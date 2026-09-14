/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.14
 *
 * 클래스 개요 : Win32 창 생성과 Windows 메시지 처리를 담당하는 클래스
 * 키보드 입력, 창 종료, 클라이언트 영역 크기 변경 상태를 관리
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <cstdint>

namespace Engine
{
    class IWindowsMessageHandler;

    class FWindowsInput;

    class FWindowsWindow final
    {
    public:
        explicit FWindowsWindow(FWindowsInput& Input) noexcept;
        ~FWindowsWindow();

        FWindowsWindow(const FWindowsWindow&) = delete;
        FWindowsWindow& operator=(const FWindowsWindow&) = delete;
        FWindowsWindow(FWindowsWindow&&) = delete;
        FWindowsWindow& operator=(FWindowsWindow&&) = delete;

        [[nodiscard]] bool Initialize(const wchar_t* Title,
                                      std::uint32_t ClientWidth,
                                      std::uint32_t ClientHeight);

        void ProcessMessages();
        void RequestClose();

        [[nodiscard]] bool ShouldClose() const noexcept;

        [[nodiscard]] HWND GetNativeHandle() const noexcept;

        [[nodiscard]] std::uint32_t GetClientWidth() const noexcept;

        [[nodiscard]] std::uint32_t GetClientHeight() const noexcept;

        [[nodiscard]] bool ConsumeResizeRequest(std::uint32_t& OutWidth,
                                                std::uint32_t& OutHeight)
            noexcept;

        void SetMessageHandler(
            IWindowsMessageHandler* InMessageHandler) noexcept;

    private:
        static LRESULT CALLBACK WindowProcedure(HWND WindowHandle, UINT Message,
                                                WPARAM WParam, LPARAM LParam);

        LRESULT HandleMessage(HWND WindowHandle, UINT Message, WPARAM WParam,
                              LPARAM LParam);

        static constexpr const wchar_t* WindowClassName =
            L"MyEngineWindowClass";

        FWindowsInput& Input;

        HINSTANCE InstanceHandle = nullptr;
        HWND WindowHandle = nullptr;
        ATOM WindowClassAtom = 0;

        std::uint32_t ClientWidth = 0;
        std::uint32_t ClientHeight = 0;

        bool bShouldClose = false;
        bool bResizeRequested = false;

        IWindowsMessageHandler* MessageHandler = nullptr;
    };
}
