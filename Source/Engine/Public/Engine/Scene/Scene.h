/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : GameObject의 생성, 소유, 갱신, 종료를 관리하는 Scene
 */

#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

namespace Engine
{
    class FGameObject;

    class FScene final
    {
    public:
        FScene();
        ~FScene();

        FScene(const FScene&) = delete;
        FScene& operator=(const FScene&) = delete;

        FGameObject& CreateGameObject(std::string Name);

        [[nodiscard]] FGameObject* FindGameObject(
            const std::string& Name) noexcept;

        [[nodiscard]] const FGameObject* FindGameObject(
            const std::string& Name) const noexcept;

        [[nodiscard]] std::span<const std::unique_ptr<FGameObject>>
        GetGameObjects() const noexcept;

        bool Initialize();
        void Update(float DeltaTime);
        void Shutdown();

    private:
        std::vector<std::unique_ptr<FGameObject>> GameObjects;

        bool bInitialized = false;
    };
}
