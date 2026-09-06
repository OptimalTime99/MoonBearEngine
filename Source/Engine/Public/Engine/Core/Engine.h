/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : 엔진의 실행 흐름과 Application 생명주기를 관리하는 진입 클래스
 * 현재 단계에서는 Application 초기화와 종료 순서만 관리
 */

#pragma once

namespace Engine
{
    class IApplication;

    class FEngine final
    {
    public:
        [[nodiscard]]
        int Run(IApplication& Application);
    };
}