/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.14
 *
 * 파일 개요 : Scene의 GameObject 소유와 생명주기 구현
 */

#include <Engine/Scene/Scene.h>

#include <Engine/Core/Log.h>
#include <Engine/Scene/GameObject.h>

#include <algorithm>
#include <utility>

namespace Engine
{
    FScene::FScene() = default;

    FScene::~FScene()
    {
        Shutdown();
    }

    FGameObject& FScene::CreateGameObject(std::string Name)
    {
        auto GameObject = std::make_unique<FGameObject>(std::move(Name));

        FGameObject& Result = *GameObject;

        GameObjects.emplace_back(std::move(GameObject));

        /*
         * 실행 중 생성된 빈 GameObject는 즉시 활성화한다.
         * 이후 AddComponent()로 추가되는 Component도 즉시 초기화된다.
         */
        if (bInitialized)
        {
            Result.Initialize();
        }

        return Result;
    }

    bool FScene::DestroyGameObject(FGameObject& GameObject)
    {
        const auto Iterator = std::find_if(GameObjects.begin(),
                                           GameObjects.end(),
                                           [&GameObject](
                                           const std::unique_ptr<FGameObject>&
                                           Candidate)
                                           {
                                               return Candidate.get() == &
                                                   GameObject;
                                           });

        if (Iterator == GameObjects.end())
        {
            return false;
        }

        if (bInitialized)
        {
            (*Iterator)->Shutdown();
        }

        GameObjects.erase(Iterator);

        return true;
    }

    void FScene::Clear()
    {
        const bool bWasInitialized = bInitialized;

        Shutdown();

        GameObjects.clear();

        /*
         * 실행 중 Scene을 비운 경우 실행 상태를 유지한다.
         * 이후 생성되는 GameObject는 즉시 초기화된다.
         */
        bInitialized = bWasInitialized;
    }

    bool FScene::IsInitialized() const noexcept
    {
        return bInitialized;
    }

    FGameObject* FScene::FindGameObject(const std::string& Name) noexcept
    {
        for (const auto& GameObject : GameObjects)
        {
            if (GameObject->GetName() == Name)
            {
                return GameObject.get();
            }
        }

        return nullptr;
    }

    const FGameObject* FScene::FindGameObject(
        const std::string& Name) const noexcept
    {
        for (const auto& GameObject : GameObjects)
        {
            if (GameObject->GetName() == Name)
            {
                return GameObject.get();
            }
        }

        return nullptr;
    }

    std::span<const std::unique_ptr<FGameObject>>
    FScene::GetGameObjects() const noexcept
    {
        return {GameObjects.data(), GameObjects.size()};
    }

    bool FScene::Initialize()
    {
        if (bInitialized)
        {
            return true;
        }

        for (const auto& GameObject : GameObjects)
        {
            if (!GameObject->Initialize())
            {
                FLog::Error("Failed to initialize a game object.");

                Shutdown();
                return false;
            }
        }

        bInitialized = true;
        return true;
    }

    void FScene::Update(const float DeltaTime)
    {
        if (!bInitialized)
        {
            return;
        }

        /*
         * Update 도중 추가된 GameObject는
         * 다음 프레임부터 Update한다.
         */
        const std::size_t GameObjectCount = GameObjects.size();

        for (std::size_t Index = 0; Index < GameObjectCount; ++Index)
        {
            GameObjects[Index]->Update(DeltaTime);
        }
    }

    void FScene::Shutdown()
    {
        if (!bInitialized)
        {
            return;
        }

        for (auto Iterator = GameObjects.rbegin(); Iterator != GameObjects.
             rend(); ++Iterator)
        {
            (*Iterator)->Shutdown();
        }

        bInitialized = false;
    }
}
