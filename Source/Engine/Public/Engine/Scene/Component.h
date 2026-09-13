/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : GameObject에 부착되는 Component의 공통 인터페이스
 */

#pragma once

namespace Engine
{
    class FGameObject;

    class FComponent
    {
    public:
        virtual ~FComponent() = default;

        FComponent(const FComponent&) = delete;
        FComponent& operator=(const FComponent&) = delete;

        [[nodiscard]] FGameObject& GetOwner() noexcept;

        [[nodiscard]] const FGameObject& GetOwner() const noexcept;

        [[nodiscard]] bool IsEnabled() const noexcept;

        void SetEnabled(bool bInEnabled) noexcept;

    protected:
        explicit FComponent(FGameObject& InOwner);

        virtual bool OnInitialize();
        virtual void OnUpdate(float DeltaTime);
        virtual void OnShutdown();

    private:
        friend class FGameObject;

        FGameObject& Owner;

        bool bEnabled = true;
        bool bInitialized = false;
    };
}
