/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : 게임 애플리케이션이 구현해야 하는 최소 생명주기 인터페이스
 * 초기화와 종료 시점을 Engine에서 호출할 수 있도록 정의
 */

#pragma once

namespace Engine
{
    class IApplication
    {
    public:
        virtual ~IApplication() = default;

        [[nodiscard]]
        virtual bool OnInitialize() = 0;

        virtual void OnShutdown() = 0;
    };
}