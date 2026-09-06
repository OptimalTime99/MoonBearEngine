/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 클래스 개요 : DirectX 11을 이용해 화면과 기본 삼각형을 렌더링하는 클래스
 * Device, SwapChain, RenderTarget과 현재 필요한 GPU 리소스의 수명을 관리
 */

#pragma once

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
        ~FD3D11Renderer();

        FD3D11Renderer(const FD3D11Renderer&) = delete;
        FD3D11Renderer& operator=(const FD3D11Renderer&) = delete;
        FD3D11Renderer(FD3D11Renderer&&) = delete;
        FD3D11Renderer& operator=(FD3D11Renderer&&) = delete;

        [[nodiscard]]
        bool Initialize(const FWindowsWindow& Window);

        [[nodiscard]]
        bool Resize(
            std::uint32_t Width,
            std::uint32_t Height);

        [[nodiscard]]
        bool Render();

    private:
        [[nodiscard]]
        bool CreateDevice();

        [[nodiscard]]
        bool CreateSwapChain(const FWindowsWindow& Window);

        [[nodiscard]]
        bool CreateRenderTarget(
            std::uint32_t Width,
            std::uint32_t Height);

        [[nodiscard]]
        bool CreateTriangleResources();

        Microsoft::WRL::ComPtr<ID3D11Device> Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>
            RenderTargetView;

        Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
    };
}