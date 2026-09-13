/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : 테스트 Scene과 GameObject 구성
 */

#include "SandboxApplication.h"

#include "RotatingComponent.h"

#include <Engine/Core/Log.h>
#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/MeshRendererComponent.h>
#include <Engine/Scene/Scene.h>

namespace Sandbox
{
    bool FSandboxApplication::OnInitialize(Engine::FScene& Scene)
    {
        Engine::FLog::Info("Sandbox initialized.");

        CubeObject = &Scene.CreateGameObject("RotatingCube");

        CubeObject->GetTransform().SetPosition(0.0f, 0.0f, 0.0f);

        CubeObject->GetTransform().SetScale(1.0f, 1.0f, 1.0f);

        const auto* MeshRenderer = CubeObject->AddComponent<
            Engine::FMeshRendererComponent>("Cube", "BasicShader",
                                            "CheckerTexture");

        if (MeshRenderer == nullptr)
        {
            Engine::FLog::Error("Failed to add the mesh renderer component.");

            return false;
        }

        const auto* RotatingComponent = CubeObject->AddComponent<
            FRotatingComponent>(20.0f, 45.0f, 0.0f);

        if (RotatingComponent == nullptr)
        {
            Engine::FLog::Error("Failed to add the rotating component.");

            return false;
        }

        return true;
    }

    void FSandboxApplication::OnUpdate(float, Engine::FScene&)
    {
        /*
         * 회전은 FRotatingComponent가 처리한다.
         */
    }

    void FSandboxApplication::OnShutdown(Engine::FScene&)
    {
        CubeObject = nullptr;

        Engine::FLog::Info("Sandbox shutting down.");
    }
}
