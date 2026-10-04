/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : DeltaTime과 대각선 보정을 적용한 Player 이동 구현
 */

#include "PlayerControllerComponent.h"

#include <Engine/Input/Input.h>
#include <Engine/Scene/GameObject.h>

#include <cmath>

namespace Sandbox
{
    FPlayerControllerComponent::FPlayerControllerComponent(
        Engine::FGameObject& Owner, const Engine::IInput& InInput,
        const float InMoveSpeed)
        : FComponent{Owner}, Input{InInput}, MoveSpeed{InMoveSpeed}
    {
    }

    void FPlayerControllerComponent::OnUpdate(const float DeltaTime)
    {
        float DirectionX = 0.0f;
        float DirectionZ = 0.0f;

        if (Input.IsKeyDown(Engine::EKey::W))
        {
            DirectionZ += 1.0f;
        }

        if (Input.IsKeyDown(Engine::EKey::S))
        {
            DirectionZ -= 1.0f;
        }

        if (Input.IsKeyDown(Engine::EKey::A))
        {
            DirectionX -= 1.0f;
        }

        if (Input.IsKeyDown(Engine::EKey::D))
        {
            DirectionX += 1.0f;
        }

        const float LengthSquared = DirectionX * DirectionX + DirectionZ *
            DirectionZ;

        if (LengthSquared <= 0.0f)
        {
            return;
        }

        const float InverseLength = 1.0f / std::sqrt(LengthSquared);

        DirectionX *= InverseLength;
        DirectionZ *= InverseLength;

        Engine::FTransform& Transform = GetOwner().GetTransform();

        const auto Position = Transform.GetPosition();

        Transform.SetPosition(Position.x + DirectionX * MoveSpeed * DeltaTime,
                              Position.y,
                              Position.z + DirectionZ * MoveSpeed * DeltaTime);
    }
}
