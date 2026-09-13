/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Engine이 실행할 게임 Application의 생명주기 인터페이스
 */

#pragma once

namespace Engine
{
    class FScene;

    class IApplication
    {
    public:
        virtual ~IApplication() = default;

        virtual bool OnInitialize(FScene& Scene) = 0;

        virtual void OnUpdate(float DeltaTime, FScene& Scene) = 0;

        virtual void OnShutdown(FScene& Scene) = 0;
    };
}
