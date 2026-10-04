/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Player와 Collectible의 XZ 평면 제곱 거리를 비교해
 * Collectible 획득과 비활성화를 처리
 */

#include "CollectibleComponent.h"

#include "GameState.h"

#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/Transform.h>

namespace Sandbox
{
    FCollectibleComponent::FCollectibleComponent(
        Engine::FGameObject& Owner, Engine::FGameObject& InPlayer,
        FGameState& InGameState, const float InTriggerRadius)
        : FComponent{Owner}, Player{InPlayer}, GameState{InGameState},
          TriggerRadius{InTriggerRadius}
    {
    }

    void FCollectibleComponent::OnUpdate(float)
    {
        const auto& PlayerPosition = Player.GetTransform().GetPosition();

        const auto& CollectiblePosition = GetOwner().GetTransform().
            GetPosition();

        const float DeltaX = PlayerPosition.x - CollectiblePosition.x;

        const float DeltaZ = PlayerPosition.z - CollectiblePosition.z;

        const float DistanceSquared = DeltaX * DeltaX + DeltaZ * DeltaZ;

        const float TriggerRadiusSquared = TriggerRadius * TriggerRadius;

        if (DistanceSquared > TriggerRadiusSquared)
        {
            return;
        }

        GameState.Collect();

        /*
         * GameObject를 비활성화하면 렌더링과 Update가 중단된다.
         * 따라서 같은 Collectible이 중복 집계되지 않는다.
         */
        GetOwner().SetActive(false);
    }
}
