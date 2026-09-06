/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : Win32 키보드 상태 저장과 ESC 키 입력 판별 구현
 * 유효하지 않은 키 코드는 배열에 접근하지 않도록 검사
 */

#include "Platform/Windows/WindowsInput.h"

#include <algorithm>

namespace Engine
{
    namespace
    {
        constexpr std::uint32_t EscapeKeyCode = 0x1B;
    }

    void FWindowsInput::BeginFrame() noexcept
    {
        PreviousKeys = CurrentKeys;
    }

    void FWindowsInput::SetKeyState(
        const std::uint32_t KeyCode,
        const bool bIsDown) noexcept
    {
        if (KeyCode >= CurrentKeys.size())
        {
            return;
        }

        CurrentKeys[KeyCode] = bIsDown;
    }

    void FWindowsInput::Reset() noexcept
    {
        CurrentKeys.fill(false);
        PreviousKeys.fill(false);
    }

    bool FWindowsInput::IsEscapePressed() const noexcept
    {
        return CurrentKeys[EscapeKeyCode]
            && !PreviousKeys[EscapeKeyCode];
    }
}