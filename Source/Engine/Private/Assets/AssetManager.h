/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Mesh, Shader, Texture 자원의 생성과 캐시 관리
 */

#pragma once

#include "Renderer/Resources/RenderTypes.h"

#include <d3d11.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace Engine
{
    class FMesh;
    class FShader;
    class FTexture;

    class FAssetManager final
    {
    public:
        explicit FAssetManager(ID3D11Device& Device);

        ~FAssetManager();

        FAssetManager(const FAssetManager&) = delete;

        FAssetManager& operator=(const FAssetManager&) = delete;

        FMesh* CreateMesh(const std::string& Name,
                          std::span<const FVertex> Vertices,
                          std::span<const std::uint16_t> Indices);

        FShader* LoadShader(const std::string& Name,
                            const std::filesystem::path& FilePath);

        FTexture* LoadTexture(const std::string& Name,
                              const std::filesystem::path& FilePath);

        [[nodiscard]] FMesh* FindMesh(const std::string& Name) const;

        [[nodiscard]] FShader* FindShader(const std::string& Name) const;

        [[nodiscard]] FTexture* FindTexture(const std::string& Name) const;

    private:
        ID3D11Device& Device;

        std::unordered_map<std::string, std::unique_ptr<FMesh>> Meshes;

        std::unordered_map<std::string, std::unique_ptr<FShader>> Shaders;

        std::unordered_map<std::string, std::unique_ptr<FTexture>> Textures;
    };
}
