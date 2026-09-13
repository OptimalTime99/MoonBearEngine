/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : 렌더링 자원에서 공통으로 사용하는 데이터 형식
 */

#pragma once

#include <DirectXMath.h>

namespace Engine
{
    struct FVertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT2 TextureCoordinate;
    };

    struct alignas(16) FTransformBuffer
    {
        DirectX::XMFLOAT4X4 WorldMatrix;
        DirectX::XMFLOAT4X4 ViewMatrix;
        DirectX::XMFLOAT4X4 ProjectionMatrix;
    };

    static_assert(sizeof(FTransformBuffer) % 16 == 0,
                  "Constant buffer size must be a multiple of 16 bytes.");
}
