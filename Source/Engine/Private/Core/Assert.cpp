/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : Assert 실패 메시지 생성과 프로그램 중단 처리
 * 실패한 표현식, 파일 경로, 줄 번호를 기록한 뒤 잘못된 실행을 종료
 */

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

#include <cstdlib>
#include <string>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace Engine::Detail
{
    [[noreturn]]
    void HandleAssertionFailure(
        const char* Expression,
        const char* File,
        const int Line)
    {
        std::string Message = "Assertion failed: ";
        Message.append(Expression);
        Message.append("\nFile: ");
        Message.append(File);
        Message.append("\nLine: ");
        Message.append(std::to_string(Line));

        FLog::Error(Message);

#if defined(_MSC_VER)
        /*
         * Visual Studio 디버거가 연결되어 있으면
         * Assert가 실패한 호출 위치에서 실행을 중단한다.
         */
        __debugbreak();
#endif

        /*
         * 디버거에서 계속 실행하더라도
         * 잘못된 상태로 프로그램이 진행되지 않게 종료한다.
         */
        std::abort();
    }
}