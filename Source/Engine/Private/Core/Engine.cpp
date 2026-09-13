/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Engine 실행 흐름과 Application 및 Scene 생명주기 구현
 */

#include <Engine/Core/Engine.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Log.h>
#include <Engine/Scene/Scene.h>

#include "Core/Timer.h"
#include "Platform/Windows/WindowsInput.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Renderer/D3D11/D3D11Renderer.h"

#include <cstdint>
#include <cstdlib>

namespace Engine
{
    int FEngine::Run(IApplication& Application)
    {
        FLog::Info("Engine starting.");

        FWindowsInput Input;
        FWindowsWindow Window{Input};

        if (!Window.Initialize(L"MyEngine Sandbox", 1280, 720))
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

        FScene Scene;

        if (!Application.OnInitialize(Scene))
        {
            FLog::Error("Application initialization failed.");

            Application.OnShutdown(Scene);
            return EXIT_FAILURE;
        }

        if (!Scene.Initialize())
        {
            FLog::Error("Scene initialization failed.");

            Application.OnShutdown(Scene);
            return EXIT_FAILURE;
        }

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

            if (Input.IsEscapePressed())
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
             * 게임 로직이 Scene을 변경한 뒤
             * Component를 갱신한다.
             */
            Application.OnUpdate(DeltaTime, Scene);

            Scene.Update(DeltaTime);

            if (!Renderer.Render(Scene))
            {
                FLog::Error("Rendering failed.");

                bHasRuntimeError = true;
                break;
            }
        }

        Application.OnShutdown(Scene);
        Scene.Shutdown();

        FLog::Info("Engine stopped.");

        return bHasRuntimeError
                   ? EXIT_FAILURE
                   : EXIT_SUCCESS;
    }
}
