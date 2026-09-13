/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : FCamera의 View 및 Perspective Projection 행렬 구현
 */

#include "Renderer/Camera.h"

namespace Engine
{
    FCamera::FCamera()
        : Position{0.0f, 0.0f, -5.0f}, Target{0.0f, 0.0f, 0.0f},
          UpDirection{0.0f, 1.0f, 0.0f}, ProjectionMatrix{}
    {
        DirectX::XMStoreFloat4x4(&ProjectionMatrix,
                                 DirectX::XMMatrixIdentity());
    }

    void FCamera::SetViewportSize(const std::uint32_t Width,
                                  const std::uint32_t Height)
    {
        if (Width == 0 || Height == 0)
        {
            return;
        }

        const float AspectRatio = static_cast<float>(Width) / static_cast<float>
            (Height);

        constexpr float FieldOfView = DirectX::XMConvertToRadians(60.0f);

        constexpr float NearPlane = 0.1f;
        constexpr float FarPlane = 1000.0f;

        const DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(
            FieldOfView, AspectRatio, NearPlane, FarPlane);

        DirectX::XMStoreFloat4x4(&ProjectionMatrix, Projection);
    }

    DirectX::XMMATRIX FCamera::GetViewMatrix() const noexcept
    {
        const DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(
            &Position);

        const DirectX::XMVECTOR CameraTarget = DirectX::XMLoadFloat3(&Target);

        const DirectX::XMVECTOR CameraUp = DirectX::XMLoadFloat3(&UpDirection);

        return DirectX::XMMatrixLookAtLH(CameraPosition, CameraTarget,
                                         CameraUp);
    }

    DirectX::XMMATRIX FCamera::GetProjectionMatrix() const noexcept
    {
        return DirectX::XMLoadFloat4x4(&ProjectionMatrix);
    }
}
