/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : 자체 엔진의 동작을 검증하기 위한 테스트 애플리케이션
 * 초기화, 프레임 업데이트, 종료 시점의 게임 동작을 정의
 */

#pragma once

#include <Engine/Core/Application.h>

class FSandboxApplication final : public Engine::IApplication
{
public:
    [[nodiscard]] bool OnInitialize() override;

    void OnUpdate(float DeltaTime) override;
    void OnShutdown() override;

private:
    bool bIsInitialized = false;
};
