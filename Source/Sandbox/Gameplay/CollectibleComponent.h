/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Player와의 XZ 평면 거리를 이용해
 * Collectible 획득을 판정하는 게임 전용 Component
 */

#pragma once

#include <Engine/Scene/Component.h>

namespace Engine
{
    class FGameObject;
}

namespace Sandbox
{
    class FGameState;

    class FCollectibleComponent final : public Engine::FComponent
    {
    public:
        FCollectibleComponent(Engine::FGameObject& Owner,
                              Engine::FGameObject& Player,
                              FGameState& GameState, float TriggerRadius);

    protected:
        void OnUpdate(float DeltaTime) override;

    private:
        /*
         * Player와 GameState는 Play 세션 동안
         * SandboxApplication과 Scene이 소유한다.
         */
        Engine::FGameObject& Player;
        FGameState& GameState;

        float TriggerRadius = 0.6f;
    };
}
