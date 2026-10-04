/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Component의 공통 상태와 기본 생명주기 구현
 */

#include <Engine/Scene/Component.h>

namespace Engine
{
    FComponent::FComponent(FGameObject& InOwner)
        : Owner{InOwner}
    {
    }

    FGameObject& FComponent::GetOwner() noexcept
    {
        return Owner;
    }

    const FGameObject& FComponent::GetOwner() const noexcept
    {
        return Owner;
    }

    bool FComponent::IsEnabled() const noexcept
    {
        return bEnabled;
    }

    void FComponent::SetEnabled(const bool bInEnabled) noexcept
    {
        bEnabled = bInEnabled;
    }

    bool FComponent::OnInitialize()
    {
        return true;
    }

    bool FComponent::OnBeginPlay()
    {
        return true;
    }

    void FComponent::OnUpdate(float)
    {
    }

    void FComponent::OnEndPlay()
    {
    }

    void FComponent::OnShutdown()
    {
    }
}
