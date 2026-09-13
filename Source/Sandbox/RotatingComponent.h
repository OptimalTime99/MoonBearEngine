/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : 소유 GameObject를 일정한 각속도로 회전시키는 테스트 Component
 */

#pragma once

#include <Engine/Scene/Component.h>

#include <DirectXMath.h>

namespace Sandbox
{
    class FRotatingComponent final : public Engine::FComponent
    {
    public:
        FRotatingComponent(Engine::FGameObject& Owner, float PitchSpeed,
                           float YawSpeed, float RollSpeed);

    protected:
        void OnUpdate(float DeltaTime) override;

    private:
        DirectX::XMFLOAT3 RotationSpeed;
    };
}
