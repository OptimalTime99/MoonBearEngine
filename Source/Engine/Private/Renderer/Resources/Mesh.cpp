/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Mesh의 Vertex/Index Buffer 생성 및 Draw 구현
 */

#include "Renderer/Resources/Mesh.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

#include <limits>

namespace Engine
{
    bool FMesh::Initialize(ID3D11Device& Device,
                           const std::span<const FVertex> Vertices,
                           const std::span<const std::uint16_t> Indices)
    {
        if (Vertices.empty() || Indices.empty())
        {
            FLog::Error("Cannot create an empty mesh.");

            return false;
        }

        if (Vertices.size_bytes() > std::numeric_limits<UINT>::max() || Indices.
            size_bytes() > std::numeric_limits<UINT>::max())
        {
            FLog::Error("Mesh buffer size exceeds the D3D11 limit.");

            return false;
        }

        D3D11_BUFFER_DESC VertexDescription{};
        VertexDescription.ByteWidth = static_cast<UINT>(Vertices.size_bytes());
        VertexDescription.Usage = D3D11_USAGE_IMMUTABLE;
        VertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA VertexData{};
        VertexData.pSysMem = Vertices.data();

        HRESULT Result = Device.CreateBuffer(&VertexDescription, &VertexData,
                                             VertexBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the mesh vertex buffer.");

            return false;
        }

        D3D11_BUFFER_DESC IndexDescription{};
        IndexDescription.ByteWidth = static_cast<UINT>(Indices.size_bytes());
        IndexDescription.Usage = D3D11_USAGE_IMMUTABLE;
        IndexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA IndexData{};
        IndexData.pSysMem = Indices.data();

        Result = Device.CreateBuffer(&IndexDescription, &IndexData,
                                     IndexBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the mesh index buffer.");

            return false;
        }

        IndexCount = static_cast<std::uint32_t>(Indices.size());

        return true;
    }

    void FMesh::Draw(ID3D11DeviceContext& DeviceContext) const
    {
        ENGINE_ASSERT(VertexBuffer != nullptr);
        ENGINE_ASSERT(IndexBuffer != nullptr);

        const UINT Stride = sizeof(FVertex);
        const UINT Offset = 0;

        ID3D11Buffer* VertexBuffers[]{VertexBuffer.Get()};

        DeviceContext.IASetVertexBuffers(0, 1, VertexBuffers, &Stride, &Offset);

        DeviceContext.IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R16_UINT,
                                       0);

        DeviceContext.IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        DeviceContext.DrawIndexed(IndexCount, 0, 0);
    }
}
