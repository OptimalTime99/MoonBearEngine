/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : 엔진 내부 전제 조건을 검증하는 Debug 전용 Assert 기능
 * 실패한 조건과 파일 위치를 출력하고 잘못된 상태의 실행을 중단
 */

#pragma once

namespace Engine::Detail
{
    [[noreturn]]
    void HandleAssertionFailure(
        const char* Expression,
        const char* File,
        int Line);
}

#if defined(_DEBUG)

#define ENGINE_ASSERT(Expression)                         \
    do                                                    \
    {                                                     \
        if (!(Expression))                                \
        {                                                 \
            ::Engine::Detail::HandleAssertionFailure(     \
                #Expression,                              \
                __FILE__,                                 \
                __LINE__);                                \
        }                                                 \
    } while (false)

#else

#define ENGINE_ASSERT(Expression) \
    ((void)sizeof(Expression))

#endif