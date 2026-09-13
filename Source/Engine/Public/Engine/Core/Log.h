/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : 엔진과 게임에서 공통으로 사용하는 최소 로그 출력 기능
 * 현재 단계에서는 일반 정보와 오류를 Visual Studio 출력 창에 전달
 */

#pragma once

#include <string_view>

namespace Engine
{
    class FLog final
    {
    public:
        FLog() = delete;

        static void Info(std::string_view Message);
        static void Error(std::string_view Message);

    private:
        static void Write(std::string_view Prefix, std::string_view Message);
    };
}
