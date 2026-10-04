/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : GameObject와 Component의 객체 및 Play 생명주기 구현
 */

#include <Engine/Scene/GameObject.h>

#include <utility>

namespace Engine
{
    FGameObject::FGameObject(std::string InName)
        : Name{std::move(InName)}
    {
    }

    FGameObject::~FGameObject()
    {
        Shutdown();
    }

    const std::string& FGameObject::GetName() const noexcept
    {
        return Name;
    }

    void FGameObject::SetName(std::string InName)
    {
        if (InName.empty())
        {
            return;
        }

        Name = std::move(InName);
    }

    FTransform& FGameObject::GetTransform() noexcept
    {
        return Transform;
    }

    const FTransform& FGameObject::GetTransform() const noexcept
    {
        return Transform;
    }

    bool FGameObject::IsActive() const noexcept
    {
        return bActive;
    }

    void FGameObject::SetActive(const bool bInActive) noexcept
    {
        bActive = bInActive;
    }

    bool FGameObject::Initialize()
    {
        if (bInitialized)
        {
            return true;
        }

        for (const auto& Component : Components)
        {
            if (!Component->OnInitialize())
            {
                /*
                 * 앞에서 초기화된 Component를 역순으로 정리한다.
                 */
                for (auto Iterator = Components.rbegin(); Iterator != Components
                     .rend(); ++Iterator)
                {
                    if ((*Iterator)->bInitialized)
                    {
                        (*Iterator)->OnShutdown();
                        (*Iterator)->bInitialized = false;
                    }
                }

                return false;
            }

            Component->bInitialized = true;
        }

        bInitialized = true;
        return true;
    }

    bool FGameObject::BeginPlay()
    {
        if (!bInitialized)
        {
            return false;
        }

        if (bPlaying)
        {
            return true;
        }

        for (const auto& Component : Components)
        {
            if (!Component->bInitialized)
            {
                continue;
            }

            if (!Component->OnBeginPlay())
            {
                for (auto Iterator = Components.rbegin(); Iterator != Components
                     .rend(); ++Iterator)
                {
                    if ((*Iterator)->bPlaying)
                    {
                        (*Iterator)->OnEndPlay();
                        (*Iterator)->bPlaying = false;
                    }
                }

                return false;
            }

            Component->bPlaying = true;
        }

        bPlaying = true;
        return true;
    }

    void FGameObject::Update(const float DeltaTime)
    {
        if (!bInitialized || !bPlaying || !bActive)
        {
            return;
        }

        /*
         * Update 중 Component가 추가되더라도 현재 프레임에서는
         * 기존 Component만 갱신한다.
         */
        const std::size_t ComponentCount = Components.size();

        for (std::size_t Index = 0; Index < ComponentCount; ++Index)
        {
            FComponent& Component = *Components[Index];

            if (Component.bInitialized && Component.bEnabled)
            {
                Component.OnUpdate(DeltaTime);
            }
        }
    }

    void FGameObject::EndPlay()
    {
        if (!bPlaying)
        {
            return;
        }

        for (auto Iterator = Components.rbegin(); Iterator != Components.rend();
             ++Iterator)
        {
            if ((*Iterator)->bPlaying)
            {
                (*Iterator)->OnEndPlay();
                (*Iterator)->bPlaying = false;
            }
        }

        bPlaying = false;
    }

    void FGameObject::Shutdown()
    {
        EndPlay();

        if (!bInitialized)
        {
            return;
        }

        for (auto Iterator = Components.rbegin(); Iterator != Components.rend();
             ++Iterator)
        {
            if ((*Iterator)->bInitialized)
            {
                (*Iterator)->OnShutdown();
                (*Iterator)->bInitialized = false;
            }
        }

        bInitialized = false;
    }
}
