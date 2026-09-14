/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.09.14
 *
 * 파일 개요 : Win32 메시지를 외부 시스템에 전달하기 위한 인터페이스
 */

#pragma once

#include <Windows.h>

namespace Engine
{
    class IWindowsMessageHandler
    {
    public:
        virtual ~IWindowsMessageHandler() = default;

        virtual bool ProcessWindowMessage(HWND WindowHandle, UINT Message,
                                          WPARAM WParam, LPARAM LParam,
                                          LRESULT& OutResult) = 0;
    };
}
