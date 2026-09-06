/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FSandboxApplication의 생명주기 함수 구현
 * 중복 초기화와 잘못된 종료 호출을 검사하고 현재 상태를 로그로 출력
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

void FSandboxApplication::OnShutdown()
{
    ENGINE_ASSERT(bIsInitialized);

    Engine::FLog::Info("Sandbox shutting down.");

    bIsInitialized = false;
}