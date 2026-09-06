/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FSandboxApplication의 생명주기 함수 구현
 * 초기화와 종료 순서를 검사하며 프레임 업데이트 진입점을 제공
 */

#include "SandboxApplication.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

bool FSandboxApplication::OnInitialize()
{
    ENGINE_ASSERT(!bIsInitialized);

    bIsInitialized = true;

    Engine::FLog::Info("Sandbox initialized.");

    return true;
}

void FSandboxApplication::OnUpdate(float)
{
    /*
     * 현재 단계에는 실행할 게임 로직이 없다.
     * DeltaTime은 3단계 이후 오브젝트 이동을 구현할 때 사용한다.
     */
}

void FSandboxApplication::OnShutdown()
{
    ENGINE_ASSERT(bIsInitialized);

    Engine::FLog::Info("Sandbox shutting down.");

    bIsInitialized = false;
}