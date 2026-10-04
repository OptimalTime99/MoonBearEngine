/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.14
 *
 * 파일 개요 : Transform과 Component를 소유하는 Scene Object
 */

#pragma once

#include <Engine/Scene/Component.h>
#include <Engine/Scene/Transform.h>

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Engine
{
    class FGameObject final
    {
    public:
        explicit FGameObject(std::string InName);
        ~FGameObject();

        FGameObject(const FGameObject&) = delete;
        FGameObject& operator=(const FGameObject&) = delete;

        [[nodiscard]] const std::string& GetName() const noexcept;

        void SetName(std::string InName);

        [[nodiscard]] FTransform& GetTransform() noexcept;

        [[nodiscard]] const FTransform& GetTransform() const noexcept;

        [[nodiscard]] bool IsActive() const noexcept;

        void SetActive(bool bInActive) noexcept;

        template <typename TComponent, typename... TArguments> requires
            std::derived_from<TComponent, FComponent>
        TComponent* AddComponent(TArguments&&... Arguments)
        {
            auto NewComponent = std::make_unique<TComponent>(
                *this, std::forward<TArguments>(Arguments)...);

            TComponent* ComponentPointer = NewComponent.get();

            /*
             * 이미 실행 중인 GameObject에 추가된 Component는
             * 즉시 초기화한다.
             */
            if (bInitialized)
            {
                if (!ComponentPointer->OnInitialize())
                {
                    return nullptr;
                }

                ComponentPointer->bInitialized = true;

                if (bPlaying)
                {
                    if (!ComponentPointer->OnBeginPlay())
                    {
                        ComponentPointer->OnShutdown();
                        ComponentPointer->bInitialized = false;
                        return nullptr;
                    }

                    ComponentPointer->bPlaying = true;
                }
            }

            Components.emplace_back(std::move(NewComponent));

            return ComponentPointer;
        }

        template <typename TComponent> requires std::derived_from<
            TComponent, FComponent>
        [[nodiscard]] TComponent* GetComponent() noexcept
        {
            for (const auto& Component : Components)
            {
                if (auto* Result = dynamic_cast<TComponent*>(Component.get()))
                {
                    return Result;
                }
            }

            return nullptr;
        }

        template <typename TComponent> requires std::derived_from<
            TComponent, FComponent>
        [[nodiscard]] const TComponent* GetComponent() const noexcept
        {
            for (const auto& Component : Components)
            {
                if (const auto* Result = dynamic_cast<const TComponent*>(
                    Component.get()))
                {
                    return Result;
                }
            }

            return nullptr;
        }

    private:
        friend class FScene;

        bool Initialize();
        bool BeginPlay();
        void Update(float DeltaTime);
        void EndPlay();
        void Shutdown();

    private:
        std::string Name;
        FTransform Transform;

        std::vector<std::unique_ptr<FComponent>> Components;

        bool bActive = true;
        bool bInitialized = false;
        bool bPlaying = false;
    };
}
