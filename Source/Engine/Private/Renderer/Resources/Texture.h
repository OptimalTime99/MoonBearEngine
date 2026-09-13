/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : 이미지 파일과 ShaderResourceView를 소유하는 Texture 자원
 */

#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <filesystem>

namespace Engine
{
    class FTexture final
    {
    public:
        FTexture() = default;
        ~FTexture() = default;

        FTexture(const FTexture&) = delete;
        FTexture& operator=(const FTexture&) = delete;

        bool Initialize(ID3D11Device& Device,
                        const std::filesystem::path& FilePath);

        void Bind(ID3D11DeviceContext& DeviceContext, std::uint32_t Slot) const;

        [[nodiscard]] std::uint32_t GetWidth() const noexcept;

        [[nodiscard]] std::uint32_t GetHeight() const noexcept;

    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResourceView;

        std::uint32_t Width = 0;
        std::uint32_t Height = 0;
    };
}
