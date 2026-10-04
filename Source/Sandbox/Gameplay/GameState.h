/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.10.04
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Collectible 개수와 승리 여부를 보관하는 Play 세션 상태
 */

#pragma once

namespace Sandbox
{
    class FGameState final
    {
    public:
        explicit FGameState(const int InTotalCollectibleCount)
            : TotalCollectibleCount{InTotalCollectibleCount}
        {
        }

        void Collect() noexcept
        {
            if (CollectedCount < TotalCollectibleCount)
            {
                ++CollectedCount;
            }
        }

        void Win() noexcept
        {
            if (HasCollectedAll())
            {
                bWon = true;
            }
        }

        [[nodiscard]] int GetCollectedCount() const noexcept
        {
            return CollectedCount;
        }

        [[nodiscard]] int GetTotalCollectibleCount() const noexcept
        {
            return TotalCollectibleCount;
        }

        [[nodiscard]] bool HasCollectedAll() const noexcept
        {
            return CollectedCount == TotalCollectibleCount;
        }

        [[nodiscard]] bool HasWon() const noexcept
        {
            return bWon;
        }

    private:
        int CollectedCount = 0;
        int TotalCollectibleCount = 0;
        bool bWon = false;
    };
}
