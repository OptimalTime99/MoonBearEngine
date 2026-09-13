/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : 3D 공간을 바라보기 위한 카메라 행렬 관리
 * 위치와 시선 방향으로 View 행렬을 만들고 화면 크기로 Projection 행렬을 생성
 */

#pragma once

#include <DirectXMath.h>

#include <cstdint>

namespace Engine
{
    class FCamera final
    {
    public:
        FCamera();

        void SetViewportSize(std::uint32_t Width, std::uint32_t Height);

        [[nodiscard]] DirectX::XMMATRIX GetViewMatrix() const noexcept;

        [[nodiscard]] DirectX::XMMATRIX GetProjectionMatrix() const noexcept;

    private:
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Target;
        DirectX::XMFLOAT3 UpDirection;

        DirectX::XMFLOAT4X4 ProjectionMatrix;
    };
}
