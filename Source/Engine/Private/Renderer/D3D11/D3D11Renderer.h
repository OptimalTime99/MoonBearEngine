/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.07
 *
 * 파일 개요 : DirectX 11 렌더러 선언
 * D3D11 장치, SwapChain, RenderTarget, Depth Buffer,
 * Shader, Vertex/Index Buffer와 Constant Buffer의 수명을 관리
 */

#pragma once

#include "Renderer/Camera.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <cstdint>

namespace Engine
{
    class FWindowsWindow;

    class FD3D11Renderer final
    {
    public:
        FD3D11Renderer() = default;
        ~FD3D11Renderer() = default;

        FD3D11Renderer(const FD3D11Renderer&) = delete;

        FD3D11Renderer& operator=(const FD3D11Renderer&) = delete;

        FD3D11Renderer(FD3D11Renderer&&) = delete;

        FD3D11Renderer& operator=(FD3D11Renderer&&) = delete;

        bool Initialize(const FWindowsWindow& Window);

        bool Resize(std::uint32_t Width, std::uint32_t Height);

        bool Render(float DeltaTime);

    private:
        bool CreateDeviceAndSwapChain(const FWindowsWindow& Window);

        bool CreateRenderTarget(std::uint32_t Width, std::uint32_t Height);

        bool CreateDepthStencil(std::uint32_t Width, std::uint32_t Height);

        bool CreateShaders();

        bool CreateCubeResources();

        bool CreateTransformBuffer();

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> Device;

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilTexture;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

        Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;

        Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;

        Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;

        Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

        Microsoft::WRL::ComPtr<ID3D11Buffer> TransformBuffer;

        FCamera Camera;

        std::uint32_t IndexCount = 0;

        float RotationRadians = 0.0f;

        float ClearColor[4]{0.02f, 0.03f, 0.08f, 1.0f};
    };
}
