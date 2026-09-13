/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : Win32 키보드 메시지를 프레임 단위 상태로 보관하는 입력 클래스
 * 현재 단계에서는 ESC 키가 새롭게 눌린 시점만 판별
 */

#pragma once

#include <array>
#include <cstdint>

namespace Engine
{
    class FWindowsInput final
    {
    public:
        void BeginFrame() noexcept;
        void SetKeyState(std::uint32_t KeyCode, bool bIsDown) noexcept;
        void Reset() noexcept;

        [[nodiscard]] bool IsEscapePressed() const noexcept;

    private:
        static constexpr std::size_t KeyCount = 256;

        std::array<bool, KeyCount> CurrentKeys{};
        std::array<bool, KeyCount> PreviousKeys{};
    };
}
