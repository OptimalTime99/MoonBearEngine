/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FTimer의 초기화와 프레임 시간 계산 구현
 * 비정상적으로 큰 DeltaTime이 게임 로직에 전달되지 않도록 상한을 적용
 */

#include "Core/Timer.h"

#include <algorithm>
#include <chrono>

namespace Engine
{
    void FTimer::Reset()
    {
        PreviousTime = FClock::now();
        DeltaTime = 0.0f;
    }

    void FTimer::Tick()
    {
        const FClock::time_point CurrentTime = FClock::now();

        const std::chrono::duration<float> ElapsedTime = CurrentTime -
            PreviousTime;

        PreviousTime = CurrentTime;

        /*
         * 디버거 중단이나 창 이동으로 프레임이 오래 정지한 경우에도
         * 한 프레임에 지나치게 큰 시간이 전달되지 않도록 제한한다.
         */
        DeltaTime = std::clamp(ElapsedTime.count(), 0.0f, 0.1f);
    }

    float FTimer::GetDeltaTime() const noexcept
    {
        return DeltaTime;
    }
}
