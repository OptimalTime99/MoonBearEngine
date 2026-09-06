/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : 프레임 사이의 경과 시간을 측정하는 엔진 내부 타이머
 * 시간 역행의 영향을 받지 않는 steady_clock을 사용해 DeltaTime을 계산
 */

#pragma once

#include <chrono>

namespace Engine
{
    class FTimer final
    {
    public:
        void Reset();
        void Tick();

        [[nodiscard]]
        float GetDeltaTime() const noexcept;

    private:
        using FClock = std::chrono::steady_clock;

        FClock::time_point PreviousTime{};
        float DeltaTime = 0.0f;
    };
}