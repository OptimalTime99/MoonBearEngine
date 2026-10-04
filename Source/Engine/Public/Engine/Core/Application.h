/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Engine이 실행할 Application의 객체 및 Play 생명주기 인터페이스
 */

#pragma once

#include <string>

namespace Engine
{
    class FScene;
    class IInput;

    class IApplication
    {
    public:
        virtual ~IApplication() = default;

        virtual bool OnInitialize(FScene& Scene) = 0;

        virtual bool OnBeginPlay(FScene& Scene, const IInput& Input) = 0;

        virtual void OnUpdate(float DeltaTime, FScene& Scene) = 0;

        virtual void OnEndPlay(FScene& Scene) = 0;

        virtual void OnShutdown(FScene& Scene) = 0;

        [[nodiscard]] virtual std::string GetPlayStatusText() const = 0;
    };
}
