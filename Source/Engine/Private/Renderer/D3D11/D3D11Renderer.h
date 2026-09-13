/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : DirectX 11 렌더러 선언
 * 렌더링 장치와 프레임 공통 자원을 관리하고 Asset을 이용해 오브젝트를 렌더링
 */

#pragma once

#include "Renderer/Camera.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <cstdint>
#include <memory>

namespace Engine
{
    class FAssetManager;
    class FMesh;
    class FShader;
    class FTexture;
    class FWindowsWindow;

    class FD3D11Renderer final
    {
    public:
        FD3D11Renderer();
        ~FD3D11Renderer();

        FD3D11Renderer(const FD3D11Renderer&) = delete;

        FD3D11Renderer& operator=(const FD3D11Renderer&) = delete;

        bool Initialize(const FWindowsWindow& Window);

        bool Resize(std::uint32_t Width, std::uint32_t Height);

        bool Render(float DeltaTime);

    private:
        bool CreateDeviceAndSwapChain(const FWindowsWindow& Window);

        bool CreateRenderTarget(std::uint32_t Width, std::uint32_t Height);

        bool CreateDepthStencil(std::uint32_t Width, std::uint32_t Height);

        bool CreateTransformBuffer();
        bool CreateSamplerState();
        bool LoadAssets();

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> Device;

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilTexture;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;

        Microsoft::WRL::ComPtr<ID3D11Buffer> TransformBuffer;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;

        /*
         * AssetManager가 Device보다 먼저 제거되도록
         * Device보다 뒤에 선언한다.
         */
        std::unique_ptr<FAssetManager> AssetManager;

        FMesh* CubeMesh = nullptr;
        FShader* BasicShader = nullptr;
        FTexture* CheckerTexture = nullptr;

        FCamera Camera;

        float RotationRadians = 0.0f;

        float ClearColor[4]{0.02f, 0.03f, 0.08f, 1.0f};
    };
}
