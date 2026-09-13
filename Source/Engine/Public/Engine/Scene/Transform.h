/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : GameObject의 위치, 회전, 크기와 World 행렬 관리
 */

#pragma once

#include <DirectXMath.h>

namespace Engine
{
    class FTransform final
    {
    public:
        FTransform();

        void SetPosition(float X, float Y, float Z) noexcept;

        void SetRotationDegrees(float Pitch, float Yaw, float Roll) noexcept;

        void AddRotationDegrees(float Pitch, float Yaw, float Roll) noexcept;

        void SetScale(float X, float Y, float Z) noexcept;

        [[nodiscard]] const DirectX::XMFLOAT3& GetPosition() const noexcept;

        [[nodiscard]] const DirectX::XMFLOAT3&
        GetRotationDegrees() const noexcept;

        [[nodiscard]] const DirectX::XMFLOAT3& GetScale() const noexcept;

        [[nodiscard]] DirectX::XMMATRIX GetWorldMatrix() const noexcept;

    private:
        void MarkDirty() noexcept;

    private:
        DirectX::XMFLOAT3 Position{0.0f, 0.0f, 0.0f};

        DirectX::XMFLOAT3 RotationDegrees{0.0f, 0.0f, 0.0f};

        DirectX::XMFLOAT3 Scale{1.0f, 1.0f, 1.0f};

        mutable DirectX::XMFLOAT4X4 CachedWorldMatrix{};

        mutable bool bWorldMatrixDirty = true;
    };
}
