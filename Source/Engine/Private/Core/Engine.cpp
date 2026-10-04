/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Engine 실행 흐름, Edit/Play Mode 전환,
 * Application 및 Scene 생명주기와 Scene 복원을 관리
 */

#include <Engine/Core/Engine.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Log.h>
#include <Engine/Input/Input.h>
#include <Engine/Scene/Scene.h>

#include "Core/Timer.h"
#include "Editor/Editor.h"
#include "Platform/Windows/WindowsInput.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Renderer/D3D11/D3D11Renderer.h"
#include "Serialization/SceneSerializer.h"

#include <cstdint>
#include <cstdlib>
#include <string>

namespace Engine
{
    int FEngine::Run(IApplication& Application)
    {
        FLog::Info("Engine starting.");

        FWindowsInput Input;
        FWindowsWindow Window{Input};

        if (!Window.Initialize(L"MoonBearEngine Sandbox", 1280, 720))
        {
            FLog::Error("Window initialization failed.");

            return EXIT_FAILURE;
        }

        FD3D11Renderer Renderer;

        if (!Renderer.Initialize(Window))
        {
            FLog::Error("Renderer initialization failed.");

            return EXIT_FAILURE;
        }

        FEditor Editor;

        if (!Editor.Initialize(Window, Renderer))
        {
            FLog::Error("Editor initialization failed.");

            return EXIT_FAILURE;
        }

        FScene Scene;

        if (!Application.OnInitialize(Scene))
        {
            FLog::Error("Application initialization failed.");

            Application.OnShutdown(Scene);
            Editor.Shutdown();

            return EXIT_FAILURE;
        }

        if (!Scene.Initialize())
        {
            FLog::Error("Scene initialization failed.");

            Application.OnShutdown(Scene);
            Editor.Shutdown();

            return EXIT_FAILURE;
        }

        /*
         * 실행 직후에는 항상 Edit Mode다.
         * Play Mode 상태는 Run() 안에서만 관리한다.
         */
        EEngineMode EngineMode = EEngineMode::Edit;

        /*
         * 디스크 임시 파일 대신 Play 직전 Scene을
         * JSON 문자열로 보관한다.
         */
        std::string EditorSceneSnapshot;

        /*
         * Play 진입:
         * 1. Edit Scene을 문자열로 저장한다.
         * 2. Sandbox가 Runtime Component를 붙인다.
         * 3. Scene 전체에 BeginPlay를 전달한다.
         */
        const auto BeginPlaySession = [&]() -> bool
        {
            if (EngineMode != EEngineMode::Edit)
            {
                return true;
            }

            EditorSceneSnapshot = FSceneSerializer::SerializeToString(Scene);

            if (EditorSceneSnapshot.empty())
            {
                FLog::Error("Failed to capture the editor scene.");

                return false;
            }

            if (!Application.OnBeginPlay(Scene, Input))
            {
                FLog::Error("Application failed to prepare play mode.");

                /*
                 * 일부 Runtime Component만 추가됐을 수 있으므로
                 * 스냅샷으로 Scene을 다시 만든다.
                 *
                 * Runtime Component가 GameState를 참조하므로
                 * Scene 복원을 먼저 하고 GameState를 제거한다.
                 */
                Editor.ClearSelection();

                const bool bRestored = FSceneSerializer::DeserializeFromString(
                    Scene, EditorSceneSnapshot);

                Application.OnEndPlay(Scene);
                EditorSceneSnapshot.clear();

                return bRestored;
            }

            if (!Scene.BeginPlay())
            {
                FLog::Error("Scene failed to begin play.");

                /*
                 * Scene::BeginPlay()는 앞에서 시작된 객체를
                 * 자체적으로 EndPlay한다.
                 */
                Scene.EndPlay();
                Editor.ClearSelection();

                const bool bRestored = FSceneSerializer::DeserializeFromString(
                    Scene, EditorSceneSnapshot);

                Application.OnEndPlay(Scene);
                EditorSceneSnapshot.clear();

                return bRestored;
            }

            EngineMode = EEngineMode::Play;

            FLog::Info("Play mode started.");

            return true;
        };

        /*
         * Play 종료:
         * 1. Component에 EndPlay를 전달한다.
         * 2. Runtime Component가 포함된 Scene을 제거한다.
         * 3. Play 직전 JSON으로 Scene을 복원한다.
         * 4. Runtime Component 제거 후 GameState를 파괴한다.
         */
        const auto EndPlaySession = [&]() -> bool
        {
            if (EngineMode != EEngineMode::Play)
            {
                return true;
            }

            Scene.EndPlay();

            /*
             * 역직렬화하면 모든 기존 FGameObject 주소가
             * 무효가 되므로 Editor 선택을 먼저 해제한다.
             */
            Editor.ClearSelection();

            const bool bRestored = FSceneSerializer::DeserializeFromString(
                Scene, EditorSceneSnapshot);

            /*
             * Runtime Component가 제거된 다음
             * Sandbox의 GameState를 파괴한다.
             */
            Application.OnEndPlay(Scene);

            EditorSceneSnapshot.clear();
            EngineMode = EEngineMode::Edit;

            if (!bRestored)
            {
                FLog::Error("Failed to restore the editor scene.");

                return false;
            }

            FLog::Info("Play mode stopped.");

            return true;
        };

        FTimer Timer;
        Timer.Reset();

        bool bHasRuntimeError = false;

        while (!Window.ShouldClose())
        {
            Input.BeginFrame();
            Window.ProcessMessages();

            if (Window.ShouldClose())
            {
                break;
            }

            if (Input.IsKeyPressed(EKey::Escape))
            {
                Window.RequestClose();
                break;
            }

            std::uint32_t NewWidth = 0;
            std::uint32_t NewHeight = 0;

            if (Window.ConsumeResizeRequest(NewWidth, NewHeight))
            {
                if (!Renderer.Resize(NewWidth, NewHeight))
                {
                    FLog::Error("Renderer resize failed.");

                    bHasRuntimeError = true;
                    break;
                }
            }

            Timer.Tick();

            const float DeltaTime = Timer.GetDeltaTime();

            /*
             * Edit Mode에서는 Application과 Scene의
             * 게임 Update를 실행하지 않는다.
             */
            if (EngineMode == EEngineMode::Play)
            {
                Application.OnUpdate(DeltaTime, Scene);

                Scene.Update(DeltaTime);
            }

            Editor.BeginFrame();

            const EEditorCommand EditorCommand = Editor.Draw(
                Scene, EngineMode, Application.GetPlayStatusText());

            if (EditorCommand == EEditorCommand::BeginPlay)
            {
                if (!BeginPlaySession())
                {
                    FLog::Error("Failed to enter play mode.");

                    bHasRuntimeError = true;
                    break;
                }
            }
            else if (EditorCommand == EEditorCommand::EndPlay)
            {
                if (!EndPlaySession())
                {
                    bHasRuntimeError = true;
                    break;
                }
            }

            Renderer.BeginFrame();

            const bool bSceneRendered = Renderer.RenderScene(Scene);

            /*
             * Scene 렌더링 결과 위에 Editor UI를 그린다.
             */
            Editor.EndFrame();

            if (!bSceneRendered)
            {
                FLog::Error("Scene rendering failed.");

                bHasRuntimeError = true;
                break;
            }

            if (!Renderer.EndFrame())
            {
                FLog::Error("Frame presentation failed.");

                bHasRuntimeError = true;
                break;
            }
        }

        /*
         * Play 중 창이 닫힌 경우에도 일반 Stop과 같은 순서로
         * Runtime Component를 제거하고 GameState를 정리한다.
         */
        if (EngineMode == EEngineMode::Play)
        {
            if (!EndPlaySession())
            {
                bHasRuntimeError = true;
            }
        }

        Application.OnShutdown(Scene);
        Scene.Shutdown();
        Editor.Shutdown();

        FLog::Info("Engine stopped.");

        return bHasRuntimeError
                   ? EXIT_FAILURE
                   : EXIT_SUCCESS;
    }
}
