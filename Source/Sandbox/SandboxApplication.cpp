/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : 작은 수집 게임의 기본 Scene 구성과
 * Play 세션 Runtime Component 연결 구현
 */

#include "SandboxApplication.h"

#include "Gameplay/CollectibleComponent.h"
#include "Gameplay/GameState.h"
#include "Gameplay/GoalComponent.h"
#include "Gameplay/PlayerControllerComponent.h"

#include <Engine/Core/Log.h>
#include <Engine/Input/Input.h>
#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/MeshRendererComponent.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Transform.h>

#include <memory>
#include <string>

namespace Sandbox
{
    FSandboxApplication::~FSandboxApplication() = default;

    bool FSandboxApplication::OnInitialize(Engine::FScene& Scene)
    {
        Engine::FLog::Info("Sandbox initialized.");

        /*
         * 현재 Main.scene.json에는 RotatingCube 하나만 있고
         * 엔진 시작 시 Scene 파일을 자동으로 읽지 않는다.
         *
         * 따라서 이번 단계에서는 코드로 기본 게임 Scene을 만든다.
         * Editor에서 배치를 수정한 뒤 Main.scene.json으로 저장할 수 있다.
         */
        CreateCube(Scene, "Player", 0.0f, 0.0f, 0.0f, 0.35f, 0.35f, 0.35f);

        CreateCube(Scene, "Collectible_1", -1.5f, 0.0f, 1.0f, 0.2f, 0.2f, 0.2f);

        CreateCube(Scene, "Collectible_2", 1.5f, 0.0f, 2.0f, 0.2f, 0.2f, 0.2f);

        CreateCube(Scene, "Collectible_3", -1.0f, 0.0f, 3.0f, 0.2f, 0.2f, 0.2f);

        CreateCube(Scene, "Goal", 2.5f, 0.0f, 3.0f, 0.5f, 0.5f, 0.5f);

        CreateCube(Scene, "Ground", 0.0f, -0.6f, 1.5f, 3.5f, 0.1f, 3.5f);

        return true;
    }

    bool FSandboxApplication::OnBeginPlay(Engine::FScene& Scene,
                                          const Engine::IInput& Input)
    {
        /*
         * Scene에서 게임 규칙에 필요한 객체를 이름으로 찾는다.
         * 사용자가 이름을 변경하거나 삭제했다면 Play를 시작하지 않는다.
         */
        Engine::FGameObject* Player = Scene.FindGameObject("Player");

        Engine::FGameObject* Collectible1 = Scene.FindGameObject(
            "Collectible_1");

        Engine::FGameObject* Collectible2 = Scene.FindGameObject(
            "Collectible_2");

        Engine::FGameObject* Collectible3 = Scene.FindGameObject(
            "Collectible_3");

        Engine::FGameObject* Goal = Scene.FindGameObject("Goal");

        if (Player == nullptr || Collectible1 == nullptr || Collectible2 ==
            nullptr || Collectible3 == nullptr || Goal == nullptr)
        {
            Engine::FLog::Error(
                "The gameplay scene is missing Player, "
                "Collectible_1, Collectible_2, " "Collectible_3, or Goal.");

            return false;
        }

        /*
         * Play를 반복할 때 이전 세션 상태가 남아 있으면 안 된다.
         */
        GameState = std::make_unique<FGameState>(3);

        if (Player->AddComponent<FPlayerControllerComponent>(Input, 3.0f) ==
            nullptr)
        {
            Engine::FLog::Error("Failed to add the player controller.");

            return false;
        }

        if (Collectible1->AddComponent<FCollectibleComponent>(
            *Player, *GameState, 0.6f) == nullptr)
        {
            Engine::FLog::Error("Failed to add Collectible_1 gameplay.");

            return false;
        }

        if (Collectible2->AddComponent<FCollectibleComponent>(
            *Player, *GameState, 0.6f) == nullptr)
        {
            Engine::FLog::Error("Failed to add Collectible_2 gameplay.");

            return false;
        }

        if (Collectible3->AddComponent<FCollectibleComponent>(
            *Player, *GameState, 0.6f) == nullptr)
        {
            Engine::FLog::Error("Failed to add Collectible_3 gameplay.");

            return false;
        }

        if (Goal->AddComponent<FGoalComponent>(*Player, *GameState, 0.9f) ==
            nullptr)
        {
            Engine::FLog::Error("Failed to add the goal gameplay.");

            return false;
        }

        Engine::FLog::Info("Sandbox play session prepared.");

        return true;
    }

    void FSandboxApplication::OnUpdate(float, Engine::FScene&)
    {
        /*
         * Player 이동과 게임 규칙은 Runtime Component가 처리한다.
         * Application 자체에서 매 프레임 처리할 작업은 없다.
         */
    }

    void FSandboxApplication::OnEndPlay(Engine::FScene&)
    {
        /*
         * Engine은 Runtime Component가 포함된 Scene을 먼저
         * 스냅샷으로 교체한다.
         *
         * Runtime Component가 제거된 후 GameState를 파괴해야
         * Component가 가리키는 참조의 수명이 안전하다.
         */
        GameState.reset();

        Engine::FLog::Info("Sandbox play session stopped.");
    }

    void FSandboxApplication::OnShutdown(Engine::FScene&)
    {
        GameState.reset();

        Engine::FLog::Info("Sandbox shutting down.");
    }

    std::string FSandboxApplication::GetPlayStatusText() const
    {
        if (GameState == nullptr)
        {
            return "Edit Mode";
        }

        if (GameState->HasWon())
        {
            return "Clear!";
        }

        return "Collectibles: " + std::to_string(GameState->GetCollectedCount())
            + " / " + std::to_string(GameState->GetTotalCollectibleCount());
    }

    Engine::FGameObject& FSandboxApplication::CreateCube(
        Engine::FScene& Scene, const char* Name, const float PositionX,
        const float PositionY, const float PositionZ, const float ScaleX,
        const float ScaleY, const float ScaleZ)
    {
        Engine::FGameObject& GameObject = Scene.CreateGameObject(Name);

        GameObject.GetTransform().SetPosition(PositionX, PositionY, PositionZ);

        GameObject.GetTransform().SetScale(ScaleX, ScaleY, ScaleZ);

        GameObject.AddComponent<Engine::FMeshRendererComponent>(
            "Cube", "BasicShader", "CheckerTexture");

        return GameObject;
    }
}
