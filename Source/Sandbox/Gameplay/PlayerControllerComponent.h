/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.11.01
 *
 * 파일 개요 : WASD로 Player를 XZ 평면에서 이동시키고
 * 벽 GameObject와의 AABB 충돌을 처리하는 Component
 */

#pragma once

#include <Engine/Scene/Component.h>

#include <vector>

namespace Engine
{
    class FGameObject;
    class IInput;
}

namespace Sandbox
{
    class FGameState;

    class FPlayerControllerComponent final : public Engine::FComponent
    {
    public:
        FPlayerControllerComponent(Engine::FGameObject& Owner,
                                   const Engine::IInput& Input,
                                   FGameState& GameState,
                                   std::vector<Engine::FGameObject*>
                                   BlockingObjects, float MoveSpeed);

    protected:
        void OnUpdate(float DeltaTime) override;

    private:
        [[nodiscard]] bool IsBlocked(float PositionX,
                                     float PositionZ) const noexcept;

    private:
        const Engine::IInput& Input;
        FGameState& GameState;

        /*
         * 벽 객체는 Play 세션 동안 Scene이 소유한다.
         * Runtime Component는 비소유 포인터만 보관한다.
         */
        std::vector<Engine::FGameObject*> BlockingObjects;

        float MoveSpeed = 3.0f;
    };
}
