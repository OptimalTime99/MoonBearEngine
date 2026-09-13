/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.07
 *
 * 파일 개요 : FEngine의 실행 흐름과 Application 생명주기 구현
 * Window, Input, Timer, D3D11 Renderer를 생성하고 게임 루프를 실행
 */

#include <Engine/Core/Engine.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Log.h>

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

        /*
         * 생성의 역순으로 Renderer가 Window보다 먼저 제거되도록
         * Window 다음에 Renderer를 생성한다.
         */
        FD3D11Renderer Renderer;

        if (!Renderer.Initialize(Window))
        {
            FLog::Error("Renderer initialization failed.");

            return EXIT_FAILURE;
        }

        if (!Application.OnInitialize())
        {
            FLog::Error("Application initialization failed.");

            return EXIT_FAILURE;
        }

        FTimer Timer;
        Timer.Reset();

        bool HasRuntimeError = false;

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

                    HasRuntimeError = true;
                    break;
                }
            }

            Timer.Tick();

            const float DeltaTime = Timer.GetDeltaTime();

            Application.OnUpdate(DeltaTime);

            if (!Renderer.Render(DeltaTime))
            {
                FLog::Error("Rendering failed.");

                HasRuntimeError = true;
                break;
            }
        }

        Application.OnShutdown();

        FLog::Info("Engine stopped.");

        return HasRuntimeError
                   ? EXIT_FAILURE
                   : EXIT_SUCCESS;
    }
}
