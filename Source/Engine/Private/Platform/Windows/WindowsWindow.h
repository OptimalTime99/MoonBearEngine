/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : Win32 창 생성과 Windows 메시지 처리를 담당하는 클래스
 * 키보드 메시지를 입력 시스템에 전달하고 창 종료 상태를 관리
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

        [[nodiscard]]
        bool Initialize(
            const wchar_t* Title,
            std::uint32_t ClientWidth,
            std::uint32_t ClientHeight);

        void ProcessMessages();
        void RequestClose();

        [[nodiscard]]
        bool ShouldClose() const noexcept;

    private:
        static LRESULT CALLBACK WindowProcedure(
            HWND WindowHandle,
            UINT Message,
            WPARAM WParam,
            LPARAM LParam);

        LRESULT HandleMessage(
            HWND WindowHandle,
            UINT Message,
            WPARAM WParam,
            LPARAM LParam);

        static constexpr const wchar_t* WindowClassName =
            L"MyEngineWindowClass";

        FWindowsInput& Input;

        HINSTANCE InstanceHandle = nullptr;
        HWND WindowHandle = nullptr;
        ATOM WindowClassAtom = 0;

        bool bShouldClose = false;
    };
}