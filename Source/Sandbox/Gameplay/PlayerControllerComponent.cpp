/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.11.01
 *
 * 파일 개요 : DeltaTime과 대각선 보정을 적용한 Player 이동,
 * XZ 평면 AABB 충돌과 축 분리 이동을 구현
 */

#include "PlayerControllerComponent.h"

#include "GameState.h"

#include <Engine/Input/Input.h>
#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/Transform.h>

#include <cmath>
#include <utility>

namespace Sandbox
{
    FPlayerControllerComponent::FPlayerControllerComponent(
        Engine::FGameObject& Owner
        , const Engine::IInput& InInput
        , FGameState& InGameState
        , std::vector<Engine::FGameObject*> InBlockingObjects
        , const float InMoveSpeed)
        : FComponent{Owner}
        , Input{InInput}
        , GameState{InGameState}
        , BlockingObjects{std::move(InBlockingObjects)}
        , MoveSpeed{InMoveSpeed}
    {
    }

    void FPlayerControllerComponent::OnUpdate(const float DeltaTime)
    {
        /*
         * 게임을 완료하면 플레이어 이동을 중단한다.
         * R 키에 의한 Reset 후에는 다시 움직일 수 있다.
         */
        if (GameState.HasWon())
        {
            return;
        }

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

        /*
         * W+D처럼 두 키를 동시에 눌러도 직선 이동보다
         * 빠르게 움직이지 않도록 방향 벡터를 정규화한다.
         */
        const float InverseLength = 1.0f / std::sqrt(LengthSquared);

        DirectionX *= InverseLength;
        DirectionZ *= InverseLength;

        Engine::FTransform& Transform = GetOwner().GetTransform();

        DirectX::XMFLOAT3 Position = Transform.GetPosition();

        const float MovementDistance = MoveSpeed * DeltaTime;

        /*
         * X축과 Z축을 따로 검사한다.
         *
         * 두 축을 한 번에 취소하면 대각선으로 벽에 닿았을 때
         * 플레이어가 완전히 멈춘다. 축을 분리하면 충돌하지 않은
         * 방향의 이동은 유지되어 벽을 따라 미끄러질 수 있다.
         */
        const float CandidateX = Position.x + DirectionX * MovementDistance;

        if (!IsBlocked(CandidateX, Position.z))
        {
            Position.x = CandidateX;
        }

        const float CandidateZ = Position.z + DirectionZ * MovementDistance;

        if (!IsBlocked(Position.x, CandidateZ))
        {
            Position.z = CandidateZ;
        }

        Transform.SetPosition(Position.x, Position.y, Position.z);
    }

    bool FPlayerControllerComponent::IsBlocked(const float PositionX
                                               , const float PositionZ) const
        noexcept
    {
        const auto& PlayerScale = GetOwner().GetTransform().GetScale();

        /*
         * 현재 Cube Mesh의 로컬 좌표 범위가 -1에서 +1이므로
         * Transform Scale의 절댓값을 AABB 반크기로 사용한다.
         */
        const float PlayerHalfWidth = std::abs(PlayerScale.x);
        const float PlayerHalfDepth = std::abs(PlayerScale.z);

        for (const Engine::FGameObject* BlockingObject : BlockingObjects)
        {
            if (BlockingObject == nullptr || !BlockingObject->IsActive())
            {
                continue;
            }

            const auto& BlockingTransform = BlockingObject->GetTransform();

            const auto& BlockingPosition = BlockingTransform.GetPosition();

            const auto& BlockingScale = BlockingTransform.GetScale();

            const float BlockingHalfWidth = std::abs(BlockingScale.x);

            const float BlockingHalfDepth = std::abs(BlockingScale.z);

            const bool bOverlapsX = std::abs(PositionX - BlockingPosition.x) <
                PlayerHalfWidth + BlockingHalfWidth;

            const bool bOverlapsZ = std::abs(PositionZ - BlockingPosition.z) <
                PlayerHalfDepth + BlockingHalfDepth;

            if (bOverlapsX && bOverlapsZ)
            {
                return true;
            }
        }

        return false;
    }
}
