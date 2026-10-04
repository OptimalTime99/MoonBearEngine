/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : 모든 Collectible 획득 여부와 Player의 Goal 진입을
 * 함께 검사해 게임 완료 상태를 설정
 */

#include "GoalComponent.h"

#include "GameState.h"

#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/Transform.h>

namespace Sandbox
{
    FGoalComponent::FGoalComponent(Engine::FGameObject& Owner,
                                   Engine::FGameObject& InPlayer,
                                   FGameState& InGameState,
                                   const float InTriggerRadius)
        : FComponent{Owner}, Player{InPlayer}, GameState{InGameState},
          TriggerRadius{InTriggerRadius}
    {
    }

    void FGoalComponent::OnUpdate(float)
    {
        if (!GameState.HasCollectedAll() || GameState.HasWon())
        {
            return;
        }

        const auto& PlayerPosition = Player.GetTransform().GetPosition();

        const auto& GoalPosition = GetOwner().GetTransform().GetPosition();

        const float DeltaX = PlayerPosition.x - GoalPosition.x;

        const float DeltaZ = PlayerPosition.z - GoalPosition.z;

        const float DistanceSquared = DeltaX * DeltaX + DeltaZ * DeltaZ;

        const float TriggerRadiusSquared = TriggerRadius * TriggerRadius;

        if (DistanceSquared <= TriggerRadiusSquared)
        {
            GameState.Win();
        }
    }
}
