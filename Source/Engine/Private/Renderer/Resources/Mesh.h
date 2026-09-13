/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Vertex Buffer와 Index Buffer를 소유하는 Mesh 자원
 */

#pragma once

#include "Renderer/Resources/RenderTypes.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <span>

namespace Engine
{
    class FMesh final
    {
    public:
        FMesh() = default;
        ~FMesh() = default;

        FMesh(const FMesh&) = delete;
        FMesh& operator=(const FMesh&) = delete;

        bool Initialize(ID3D11Device& Device, std::span<const FVertex> Vertices,
                        std::span<const std::uint16_t> Indices);

        void Draw(ID3D11DeviceContext& DeviceContext) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

        std::uint32_t IndexCount = 0;
    };
}
