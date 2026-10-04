/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : WASD로 Player를 XZ 평면에서 이동시키는 Component
 */

#pragma once

#include <Engine/Scene/Component.h>

namespace Engine
{
    class IInput;
}

namespace Sandbox
{
    class FPlayerControllerComponent final : public Engine::FComponent
    {
    public:
        FPlayerControllerComponent(Engine::FGameObject& Owner,
                                   const Engine::IInput& Input,
                                   float MoveSpeed);

    protected:
        void OnUpdate(float DeltaTime) override;

    private:
        const Engine::IInput& Input;
        float MoveSpeed = 3.0f;
    };
}
