/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : 작은 수집 게임의 기본 Scene을 구성하고
 * Play 세션의 Runtime Component와 GameState를 관리
 */

#pragma once

#include <Engine/Core/Application.h>

#include <memory>

#include "Gameplay/GameState.h"

namespace Engine
{
    class FGameObject;
    class FScene;
    class IInput;
}

namespace Sandbox
{
    class FSandboxApplication final : public Engine::IApplication
    {
    public:
        FSandboxApplication() = default;

        /*
         * FGameState는 이 헤더에서 전방 선언만 되어 있으므로
         * 소멸자는 GameState.h를 포함하는 cpp에서 정의한다.
         */
        ~FSandboxApplication() override;

        FSandboxApplication(const FSandboxApplication&) = delete;
        FSandboxApplication& operator=(const FSandboxApplication&) = delete;

        bool OnInitialize(Engine::FScene& Scene) override;

        bool OnBeginPlay(Engine::FScene& Scene,
                         const Engine::IInput& Input) override;

        void OnUpdate(float DeltaTime, Engine::FScene& Scene) override;

        void OnEndPlay(Engine::FScene& Scene) override;

        void OnShutdown(Engine::FScene& Scene) override;

        [[nodiscard]] std::string GetPlayStatusText() const override;

    private:
        static Engine::FGameObject& CreateCube(Engine::FScene& Scene,
                                               const char* Name,
                                               float PositionX, float PositionY,
                                               float PositionZ, float ScaleX,
                                               float ScaleY, float ScaleZ);

    private:
        /*
         * Play 세션 동안만 존재한다.
         * Runtime Component는 이 객체를 참조하므로 Scene의
         * Runtime Component가 제거된 후에 파괴해야 한다.
         */
        std::unique_ptr<FGameState> GameState;
    };
}
