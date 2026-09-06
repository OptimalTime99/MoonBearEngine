/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FEngine의 실행 흐름 구현
 * Application 초기화 결과를 검사하고 정상 초기화된 경우에만 종료 함수 호출
 */

#include <Engine/Core/Engine.h>

#include <Engine/Core/Application.h>
#include <Engine/Core/Log.h>

#include <cstdlib>

namespace Engine
{
    int FEngine::Run(IApplication& Application)
    {
        FLog::Info("Engine starting.");

        if (!Application.OnInitialize())
        {
            FLog::Error("Application initialization failed.");
            return EXIT_FAILURE;
        }

        /*
         * 현재 단계에서는 Application의 생명주기만 검증한다.
         * Update 루프는 Win32 메시지 처리가 추가되는 2단계에서 구현한다.
         */
        Application.OnShutdown();

        FLog::Info("Engine stopped.");

        return EXIT_SUCCESS;
    }
}