/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : GameObject가 사용할 Mesh, Shader, Texture Asset 정보 관리
 */

#pragma once

#include <Engine/Scene/Component.h>

#include <string>

namespace Engine
{
    class FMeshRendererComponent final : public FComponent
    {
    public:
        FMeshRendererComponent(FGameObject& Owner, std::string InMeshName,
                               std::string InShaderName,
                               std::string InTextureName);

        [[nodiscard]] const std::string& GetMeshName() const noexcept;

        [[nodiscard]] const std::string& GetShaderName() const noexcept;

        [[nodiscard]] const std::string& GetTextureName() const noexcept;

        [[nodiscard]] bool IsVisible() const noexcept;

        void SetVisible(bool bInVisible) noexcept;
        
        bool OnInitialize() override;

    private:
        std::string MeshName;
        std::string ShaderName;
        std::string TextureName;

        bool bVisible = true;
    };
}
