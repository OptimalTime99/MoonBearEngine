/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.11.01
 *
 * 파일 개요 : 수집 게임 Scene 구성, Runtime Component 연결,
 * 벽 충돌 대상 구성과 R 키 재시작을 구현
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
#include <vector>

namespace Sandbox
{
    FSandboxApplication::~FSandboxApplication() = default;

    bool FSandboxApplication::OnInitialize(Engine::FScene& Scene)
    {
        Engine::FLog::Info("Sandbox initialized.");

        CreateCube(Scene, "Player", 0.0f, 0.0f, 0.0f, 0.35f, 0.35f, 0.35f);

        CreateCube(Scene, "Collectible_1", -1.5f, 0.0f, 1.0f, 0.2f, 0.2f, 0.2f);

        CreateCube(Scene, "Collectible_2", 1.5f, 0.0f, 2.0f, 0.2f, 0.2f, 0.2f);

        CreateCube(Scene, "Collectible_3", -1.0f, 0.0f, 3.0f, 0.2f, 0.2f, 0.2f);

        CreateCube(Scene, "Goal", 2.5f, 0.0f, 3.0f, 0.5f, 0.5f, 0.5f);

        CreateCube(Scene, "Ground", 0.0f, -0.6f, 1.5f, 3.5f, 0.1f, 3.5f);

        /*
         * Ground의 XZ 범위는 다음과 같다.
         *
         * X: -3.5 ~ +3.5
         * Z: -2.0 ~ +5.0
         *
         * 벽은 Ground 안쪽 가장자리에 배치한다.
         */
        CreateCube(Scene, "Wall_West", -3.3f, 0.0f, 1.5f, 0.2f, 0.6f, 3.5f);

        CreateCube(Scene, "Wall_East", 3.3f, 0.0f, 1.5f, 0.2f, 0.6f, 3.5f);

        CreateCube(Scene, "Wall_South", 0.0f, 0.0f, -1.8f, 3.5f, 0.6f, 0.2f);

        CreateCube(Scene, "Wall_North", 0.0f, 0.0f, 4.8f, 3.5f, 0.6f, 0.2f);

        return true;
    }

    bool FSandboxApplication::OnBeginPlay(Engine::FScene& Scene
                                          , const Engine::IInput& Input)
    {
        Engine::FGameObject* Player = Scene.FindGameObject("Player");

        Engine::FGameObject* Collectible1 = Scene.FindGameObject
        (
            "Collectible_1"
        );

        Engine::FGameObject* Collectible2 = Scene.FindGameObject
        (
            "Collectible_2"
        );

        Engine::FGameObject* Collectible3 = Scene.FindGameObject
        (
            "Collectible_3"
        );

        Engine::FGameObject* Goal = Scene.FindGameObject("Goal");

        Engine::FGameObject* WallWest = Scene.FindGameObject("Wall_West");

        Engine::FGameObject* WallEast = Scene.FindGameObject("Wall_East");

        Engine::FGameObject* WallSouth = Scene.FindGameObject("Wall_South");

        Engine::FGameObject* WallNorth = Scene.FindGameObject("Wall_North");

        if (Player == nullptr || Collectible1 == nullptr || Collectible2 ==
            nullptr || Collectible3 == nullptr || Goal == nullptr || WallWest ==
            nullptr || WallEast == nullptr || WallSouth == nullptr || WallNorth
            == nullptr)
        {
            Engine::FLog::Error
            (
                "The gameplay scene is missing a required object."
            );

            return false;
        }

        GameState = std::make_unique<FGameState>(3);

        PlayInput = &Input;

        std::vector<Engine::FGameObject*> BlockingObjects{
            WallWest, WallEast, WallSouth, WallNorth
        };

        if (Player->AddComponent<FPlayerControllerComponent>
        (
            Input
            , *GameState
            , std::move(BlockingObjects)
            , 3.0f
        ) == nullptr)
        {
            Engine::FLog::Error("Failed to add the player controller.");

            return false;
        }

        if (Collectible1->AddComponent<FCollectibleComponent>
        (
            *Player
            , *GameState
            , 0.6f
        ) == nullptr)
        {
            Engine::FLog::Error("Failed to add Collectible_1 gameplay.");

            return false;
        }

        if (Collectible2->AddComponent<FCollectibleComponent>
        (
            *Player
            , *GameState
            , 0.6f
        ) == nullptr)
        {
            Engine::FLog::Error("Failed to add Collectible_2 gameplay.");

            return false;
        }

        if (Collectible3->AddComponent<FCollectibleComponent>
        (
            *Player
            , *GameState
            , 0.6f
        ) == nullptr)
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

    void FSandboxApplication::OnUpdate(float
                                       , Engine::FScene& Scene)
    {
        if (PlayInput == nullptr || GameState == nullptr)
        {
            return;
        }

        if (PlayInput->IsKeyPressed(Engine::EKey::R))
        {
            ResetGame(Scene);
        }
    }

    void FSandboxApplication::OnEndPlay(Engine::FScene&)
    {
        /*
         * Engine은 Runtime Component가 포함된 Scene을
         * 스냅샷으로 교체한 다음 OnEndPlay를 호출한다.
         *
         * 따라서 Runtime Component의 GameState 참조가 제거된
         * 이후 GameState를 파괴한다.
         */
        PlayInput = nullptr;
        GameState.reset();

        Engine::FLog::Info("Sandbox play session stopped.");
    }

    void FSandboxApplication::OnShutdown(Engine::FScene&)
    {
        PlayInput = nullptr;
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

        if (GameState->HasCollectedAll())
        {
            return "Exit open - reach the Goal!";
        }

        return "Collectibles: " + std::to_string(GameState->GetCollectedCount())
            + " / " + std::to_string(GameState->GetTotalCollectibleCount());
    }

    Engine::FGameObject& FSandboxApplication::CreateCube(
        Engine::FScene& Scene
        , const char* Name
        , const float PositionX
        , const float PositionY
        , const float PositionZ
        , const float ScaleX
        , const float ScaleY
        , const float ScaleZ)
    {
        Engine::FGameObject& GameObject = Scene.CreateGameObject(Name);

        GameObject.GetTransform().SetPosition(PositionX, PositionY, PositionZ);

        GameObject.GetTransform().SetScale(ScaleX, ScaleY, ScaleZ);

        GameObject.AddComponent<Engine::FMeshRendererComponent>
        (
            "Cube"
            , "BasicShader"
            , "CheckerTexture"
        );

        return GameObject;
    }

    void FSandboxApplication::ResetGame(Engine::FScene& Scene)
    {
        if (GameState == nullptr)
        {
            return;
        }

        Engine::FGameObject* Player = Scene.FindGameObject("Player");

        Engine::FGameObject* Collectible1 = Scene.FindGameObject
        (
            "Collectible_1"
        );

        Engine::FGameObject* Collectible2 = Scene.FindGameObject
        (
            "Collectible_2"
        );

        Engine::FGameObject* Collectible3 = Scene.FindGameObject
        (
            "Collectible_3"
        );

        Engine::FGameObject* Goal = Scene.FindGameObject("Goal");

        if (Player == nullptr || Collectible1 == nullptr || Collectible2 ==
            nullptr || Collectible3 == nullptr || Goal == nullptr)
        {
            Engine::FLog::Error("Failed to reset the gameplay scene.");

            return;
        }

        GameState->Reset();

        Player->GetTransform().SetPosition(0.0f, 0.0f, 0.0f);

        Collectible1->SetActive(true);
        Collectible2->SetActive(true);
        Collectible3->SetActive(true);
        Goal->SetActive(true);

        Engine::FLog::Info("Game restarted.");
    }
}
