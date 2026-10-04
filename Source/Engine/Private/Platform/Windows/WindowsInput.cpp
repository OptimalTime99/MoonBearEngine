/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Win32 키 상태 저장 및 Engine 키 입력으로의 변환 구현
 */

#include "Platform/Windows/WindowsInput.h"

namespace Engine
{
    void FWindowsInput::BeginFrame() noexcept
    {
        PreviousKeys = CurrentKeys;
    }

    void FWindowsInput::SetKeyState(const std::uint32_t KeyCode,
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

    bool FWindowsInput::IsKeyDown(const EKey Key) const noexcept
    {
        const std::uint32_t KeyCode = ToVirtualKey(Key);

        return KeyCode < CurrentKeys.size() && CurrentKeys[KeyCode];
    }

    bool FWindowsInput::IsKeyPressed(const EKey Key) const noexcept
    {
        const std::uint32_t KeyCode = ToVirtualKey(Key);

        return KeyCode < CurrentKeys.size() && CurrentKeys[KeyCode] && !
            PreviousKeys[KeyCode];
    }

    std::uint32_t FWindowsInput::ToVirtualKey(const EKey Key) noexcept
    {
        switch (Key)
        {
        case EKey::W:
            return 'W';

        case EKey::A:
            return 'A';

        case EKey::S:
            return 'S';

        case EKey::D:
            return 'D';

        case EKey::Escape:
            return 0x1B;

        default:
            return 0;
        }
    }
}
