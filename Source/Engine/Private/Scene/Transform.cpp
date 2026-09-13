/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Transform 값 변경과 World 행렬 계산 구현
 */

#include <Engine/Scene/Transform.h>

namespace Engine
{
    FTransform::FTransform()
    {
        DirectX::XMStoreFloat4x4(&CachedWorldMatrix,
                                 DirectX::XMMatrixIdentity());
    }

    void FTransform::SetPosition(const float X, const float Y,
                                 const float Z) noexcept
    {
        Position = {X, Y, Z};
        MarkDirty();
    }

    void FTransform::SetRotationDegrees(const float Pitch, const float Yaw,
                                        const float Roll) noexcept
    {
        RotationDegrees = {Pitch, Yaw, Roll};

        MarkDirty();
    }

    void FTransform::AddRotationDegrees(const float Pitch, const float Yaw,
                                        const float Roll) noexcept
    {
        RotationDegrees.x += Pitch;
        RotationDegrees.y += Yaw;
        RotationDegrees.z += Roll;

        MarkDirty();
    }

    void FTransform::SetScale(const float X, const float Y,
                              const float Z) noexcept
    {
        Scale = {X, Y, Z};
        MarkDirty();
    }

    const DirectX::XMFLOAT3& FTransform::GetPosition() const noexcept
    {
        return Position;
    }

    const DirectX::XMFLOAT3& FTransform::GetRotationDegrees() const noexcept
    {
        return RotationDegrees;
    }

    const DirectX::XMFLOAT3& FTransform::GetScale() const noexcept
    {
        return Scale;
    }

    DirectX::XMMATRIX FTransform::GetWorldMatrix() const noexcept
    {
        if (bWorldMatrixDirty)
        {
            const DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(
                Scale.x, Scale.y, Scale.z);

            const DirectX::XMMATRIX RotationMatrix =
                DirectX::XMMatrixRotationRollPitchYaw(
                    DirectX::XMConvertToRadians(RotationDegrees.x),
                    DirectX::XMConvertToRadians(RotationDegrees.y),
                    DirectX::XMConvertToRadians(RotationDegrees.z));

            const DirectX::XMMATRIX TranslationMatrix =
                DirectX::XMMatrixTranslation(Position.x, Position.y,
                                             Position.z);

            /*
             * DirectXMath의 행 벡터 규칙에 따라
             * Scale → Rotation → Translation 순서로 결합한다.
             */
            const DirectX::XMMATRIX WorldMatrix = ScaleMatrix * RotationMatrix *
                TranslationMatrix;

            DirectX::XMStoreFloat4x4(&CachedWorldMatrix, WorldMatrix);

            bWorldMatrixDirty = false;
        }

        return DirectX::XMLoadFloat4x4(&CachedWorldMatrix);
    }

    void FTransform::MarkDirty() noexcept
    {
        bWorldMatrixDirty = true;
    }
}
