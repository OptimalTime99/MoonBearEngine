/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FEngine의 실행 흐름과 Application 생명주기 구현
 * Window, 입력, Timer를 생성하고 창이 닫힐 때까지 게임 루프를 실행
 */

#include <Engine/Core/Engine.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Log.h>

#include "Core/Timer.h"
#include "Platform/Windows/WindowsInput.h"
#include "Platform/Windows/WindowsWindow.h"

#include <chrono>
#include <cstdlib>
#include <thread>

namespace Engine
{
    int FEngine::Run(IApplication& Application)
    {
        FLog::Info("Engine starting.");

        FWindowsInput Input;
        FWindowsWindow Window{ Input };

        if (!Window.Initialize(
            L"MoonBearEngine Sandbox",
            1280,
            720))
        {
            FLog::Error("Window initialization failed.");
            return EXIT_FAILURE;
        }

        if (!Application.OnInitialize())
        {
            FLog::Error("Application initialization failed.");
            return EXIT_FAILURE;
        }

        FTimer Timer;
        Timer.Reset();

        /*
         * 아직 Present 기반 VSync가 없으므로 빈 루프가 CPU를 모두
         * 사용하지 않도록 임시로 약 60FPS 간격을 적용한다.
         */
        constexpr auto TargetFrameDuration =
            std::chrono::nanoseconds{ 16'666'667 };

        while (!Window.ShouldClose())
        {
            const auto FrameStart =
                std::chrono::steady_clock::now();

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

            Timer.Tick();
            Application.OnUpdate(Timer.GetDeltaTime());

            std::this_thread::sleep_until(
                FrameStart + TargetFrameDuration);
        }

        Application.OnShutdown();

        FLog::Info("Engine stopped.");

        return EXIT_SUCCESS;
    }
}