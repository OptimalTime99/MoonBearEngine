/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : MoonBearEngine을 사용하는 테스트 Application
 */

#pragma once

#include <Engine/Core/Application.h>

namespace Engine
{
    class FGameObject;
    class FScene;
}

namespace Sandbox
{
    class FSandboxApplication final : public Engine::IApplication
    {
    public:
        bool OnInitialize(Engine::FScene& Scene) override;

        void OnUpdate(float DeltaTime, Engine::FScene& Scene) override;

        void OnShutdown(Engine::FScene& Scene) override;

    private:
        /*
         * Scene이 소유하며 Sandbox는 소유하지 않는다.
         */
        Engine::FGameObject* CubeObject = nullptr;
    };
}
