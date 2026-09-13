/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Vertex Shader, Pixel Shader, Input Layout을 소유하는 Shader 자원
 */

#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <filesystem>
#include <string_view>

namespace Engine
{
    class FShader final
    {
    public:
        FShader() = default;
        ~FShader() = default;

        FShader(const FShader&) = delete;
        FShader& operator=(const FShader&) = delete;

        bool Initialize(ID3D11Device& Device,
                        const std::filesystem::path& FilePath,
                        std::string_view VertexEntryPoint,
                        std::string_view PixelEntryPoint);

        void Bind(ID3D11DeviceContext& DeviceContext) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;

        Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
    };
}
