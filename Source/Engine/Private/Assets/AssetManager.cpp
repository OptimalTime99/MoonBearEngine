/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : 렌더링 자원의 생성, 로딩, 이름 기반 캐시 구현
 */

#include "Assets/AssetManager.h"

#include <Engine/Core/Log.h>

#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Texture.h"

namespace Engine
{
    FAssetManager::FAssetManager(ID3D11Device& InDevice)
        : Device{InDevice}
    {
    }

    FAssetManager::~FAssetManager() = default;

    FMesh* FAssetManager::CreateMesh(const std::string& Name,
                                     const std::span<const FVertex> Vertices,
                                     const std::span<const std::uint16_t>
                                     Indices)
    {
        if (FMesh* ExistingMesh = FindMesh(Name))
        {
            return ExistingMesh;
        }

        auto Mesh = std::make_unique<FMesh>();

        if (!Mesh->Initialize(Device, Vertices, Indices))
        {
            return nullptr;
        }

        FMesh* MeshPointer = Mesh.get();

        Meshes.emplace(Name, std::move(Mesh));

        return MeshPointer;
    }

    FShader* FAssetManager::LoadShader(const std::string& Name,
                                       const std::filesystem::path& FilePath)
    {
        if (FShader* ExistingShader = FindShader(Name))
        {
            return ExistingShader;
        }

        auto Shader = std::make_unique<FShader>();

        if (!Shader->Initialize(Device, FilePath, "VSMain", "PSMain"))
        {
            return nullptr;
        }

        FShader* ShaderPointer = Shader.get();

        Shaders.emplace(Name, std::move(Shader));

        return ShaderPointer;
    }

    FTexture* FAssetManager::LoadTexture(const std::string& Name,
                                         const std::filesystem::path& FilePath)
    {
        if (FTexture* ExistingTexture = FindTexture(Name))
        {
            return ExistingTexture;
        }

        auto Texture = std::make_unique<FTexture>();

        if (!Texture->Initialize(Device, FilePath))
        {
            return nullptr;
        }

        FTexture* TexturePointer = Texture.get();

        Textures.emplace(Name, std::move(Texture));

        return TexturePointer;
    }

    FMesh* FAssetManager::FindMesh(const std::string& Name) const
    {
        const auto Iterator = Meshes.find(Name);

        return Iterator != Meshes.end()
                   ? Iterator->second.get()
                   : nullptr;
    }

    FShader* FAssetManager::FindShader(const std::string& Name) const
    {
        const auto Iterator = Shaders.find(Name);

        return Iterator != Shaders.end()
                   ? Iterator->second.get()
                   : nullptr;
    }

    FTexture* FAssetManager::FindTexture(const std::string& Name) const
    {
        const auto Iterator = Textures.find(Name);

        return Iterator != Textures.end()
                   ? Iterator->second.get()
                   : nullptr;
    }
}
