/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : Sandbox 실행 파일의 Windows 진입점
 * Application과 Engine을 생성하고 Engine의 실행 흐름을 시작
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <Engine/Core/Engine.h>

#include "SandboxApplication.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    /*
     * 두 객체는 Run이 끝날 때까지만 필요하므로 스택에 생성한다.
     * 별도의 동적 할당이나 공유 소유권은 필요하지 않다.
     */
    Sandbox::FSandboxApplication Application;
    Engine::FEngine EngineInstance;

    return EngineInstance.Run(Application);
}
