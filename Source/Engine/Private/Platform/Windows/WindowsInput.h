/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Win32 키보드 메시지를 프레임 단위 상태로 보관하고
 * Engine Public 입력 인터페이스로 변환하는 Windows 입력 클래스
 */

#pragma once

#include <Engine/Input/Input.h>

#include <array>
#include <cstdint>

namespace Engine
{
    class FWindowsInput final : public IInput
    {
    public:
        void BeginFrame() noexcept;

        void SetKeyState(std::uint32_t KeyCode, bool bIsDown) noexcept;

        void Reset() noexcept;

        [[nodiscard]] bool IsKeyDown(EKey Key) const noexcept override;

        [[nodiscard]] bool IsKeyPressed(EKey Key) const noexcept override;

    private:
        [[nodiscard]] static std::uint32_t ToVirtualKey(EKey Key) noexcept;

    private:
        static constexpr std::size_t KeyCount = 256;

        std::array<bool, KeyCount> CurrentKeys{};
        std::array<bool, KeyCount> PreviousKeys{};
    };
}
