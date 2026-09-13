/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : MeshRendererComponent의 Asset 정보와 상태 구현
 */

#include <Engine/Scene/MeshRendererComponent.h>

#include <utility>

namespace Engine
{
    FMeshRendererComponent::FMeshRendererComponent(
        FGameObject& Owner, std::string InMeshName, std::string InShaderName,
        std::string InTextureName)
        : FComponent{Owner}, MeshName{std::move(InMeshName)},
          ShaderName{std::move(InShaderName)},
          TextureName{std::move(InTextureName)}
    {
    }

    const std::string& FMeshRendererComponent::GetMeshName() const noexcept
    {
        return MeshName;
    }

    const std::string& FMeshRendererComponent::GetShaderName() const noexcept
    {
        return ShaderName;
    }

    const std::string& FMeshRendererComponent::GetTextureName() const noexcept
    {
        return TextureName;
    }

    bool FMeshRendererComponent::IsVisible() const noexcept
    {
        return bVisible;
    }

    void FMeshRendererComponent::SetVisible(const bool bInVisible) noexcept
    {
        bVisible = bInVisible;
    }

    bool FMeshRendererComponent::OnInitialize()
    {
        return !MeshName.empty() && !ShaderName.empty() && !TextureName.empty();
    }
}
