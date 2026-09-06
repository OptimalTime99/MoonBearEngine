/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FLog의 Windows Debug 출력 구현
 * 로그 레벨 접두사와 메시지를 결합하여 Visual Studio 출력 창에 전달
 */

#include <Engine/Core/Log.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <string>

namespace Engine
{
    void FLog::Info(const std::string_view Message)
    {
        Write("[Engine][Info] ", Message);
    }

    void FLog::Error(const std::string_view Message)
    {
        Write("[Engine][Error] ", Message);
    }

    void FLog::Write(
        const std::string_view Prefix,
        const std::string_view Message)
    {
        std::string Output;
        Output.reserve(Prefix.size() + Message.size() + 1);

        Output.append(Prefix);
        Output.append(Message);
        Output.push_back('\n');

        /*
         * OutputDebugStringA는 전달된 문자열을 보관하지 않는다.
         * 따라서 호출이 끝날 때까지 유효한 지역 문자열을 전달할 수 있다.
         */
        ::OutputDebugStringA(Output.c_str());
    }
}