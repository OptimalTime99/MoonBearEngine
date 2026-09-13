/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : DeltaTime을 이용한 GameObject 회전 구현
 */

#include "RotatingComponent.h"

#include <Engine/Scene/GameObject.h>

namespace Sandbox
{
    FRotatingComponent::FRotatingComponent(Engine::FGameObject& Owner,
                                           const float PitchSpeed,
                                           const float YawSpeed,
                                           const float RollSpeed)
        : FComponent{Owner}, RotationSpeed{PitchSpeed, YawSpeed, RollSpeed}
    {
    }

    void FRotatingComponent::OnUpdate(const float DeltaTime)
    {
        GetOwner().GetTransform().AddRotationDegrees(
            RotationSpeed.x * DeltaTime, RotationSpeed.y * DeltaTime,
            RotationSpeed.z * DeltaTime);
    }
}
