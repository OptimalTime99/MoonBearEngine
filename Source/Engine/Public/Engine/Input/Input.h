/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Platform 구현과 Sandbox 사이의 키보드 입력 인터페이스
 * Sandbox가 Win32 Virtual Key나 Windows 헤더에 의존하지 않게 한다.
 */

#pragma once

namespace Engine
{
    enum class EKey
    {
        W,
        A,
        S,
        D,
        Escape
    };

    class IInput
    {
    public:
        virtual ~IInput() = default;

        [[nodiscard]] virtual bool IsKeyDown(EKey Key) const noexcept = 0;

        [[nodiscard]] virtual bool IsKeyPressed(EKey Key) const noexcept = 0;
    };
}
