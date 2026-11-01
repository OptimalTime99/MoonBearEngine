/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.11.01
 *
 * 파일 개요 : XZ 게임 공간을 위에서 내려다보는
 * 고정 직교 탑다운 카메라 구현
 */

#include "Renderer/Camera.h"

namespace Engine
{
    FCamera::FCamera()
        : Position{0.0f, 10.0f, 1.5f}, Target{0.0f, 0.0f, 1.5f},
          UpDirection{0.0f, 0.0f, 1.0f}, ProjectionMatrix{}
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

        /*
         * 세로 9 단위가 항상 보이게 하고,
         * 가로 범위는 창의 종횡비에 따라 결정한다.
         *
         * Perspective가 아닌 Orthographic을 사용하므로
         * 화면 위쪽과 아래쪽의 물체 크기가 동일하다.
         */
        constexpr float VisibleHeight = 9.0f;

        const float VisibleWidth = VisibleHeight * AspectRatio;

        constexpr float NearPlane = 0.1f;
        constexpr float FarPlane = 100.0f;

        const DirectX::XMMATRIX Projection = DirectX::XMMatrixOrthographicLH(
            VisibleWidth, VisibleHeight, NearPlane, FarPlane);

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
