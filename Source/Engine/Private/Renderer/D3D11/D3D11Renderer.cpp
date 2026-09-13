/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.07
 *
 * 파일 개요 : DirectX 11 렌더러 구현
 * D3D11 렌더링 자원을 생성하고 World/View/Projection 행렬을 이용해
 * 회전하는 3D 큐브를 렌더링
 */

#include "Renderer/D3D11/D3D11Renderer.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

#include "Platform/Windows/WindowsWindow.h"

#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "Assets/AssetManager.h"

#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/RenderTypes.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Texture.h"

#include <array>
#include <memory>
#include <cstdint>

namespace
{
    using Engine::FVertex;

    constexpr DirectX::XMFLOAT4 White{1.0f, 1.0f, 1.0f, 1.0f};

    constexpr std::array<FVertex, 24> CubeVertices{
        {
            // 앞면
            {{-1, -1, -1}, White, {0, 1}}, {{-1, 1, -1}, White, {0, 0}},
            {{1, 1, -1}, White, {1, 0}}, {{1, -1, -1}, White, {1, 1}},

            // 뒷면
            {{1, -1, 1}, White, {0, 1}}, {{1, 1, 1}, White, {0, 0}},
            {{-1, 1, 1}, White, {1, 0}}, {{-1, -1, 1}, White, {1, 1}},

            // 왼쪽 면
            {{-1, -1, 1}, White, {0, 1}}, {{-1, 1, 1}, White, {0, 0}},
            {{-1, 1, -1}, White, {1, 0}}, {{-1, -1, -1}, White, {1, 1}},

            // 오른쪽 면
            {{1, -1, -1}, White, {0, 1}}, {{1, 1, -1}, White, {0, 0}},
            {{1, 1, 1}, White, {1, 0}}, {{1, -1, 1}, White, {1, 1}},

            // 위쪽 면
            {{-1, 1, -1}, White, {0, 1}}, {{-1, 1, 1}, White, {0, 0}},
            {{1, 1, 1}, White, {1, 0}}, {{1, 1, -1}, White, {1, 1}},

            // 아래쪽 면
            {{-1, -1, 1}, White, {0, 1}}, {{-1, -1, -1}, White, {0, 0}},
            {{1, -1, -1}, White, {1, 0}}, {{1, -1, 1}, White, {1, 1}}
        }
    };

    constexpr std::array<std::uint16_t, 36> CubeIndices{
        {
            0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14,
            12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23
        }
    };
}

namespace Engine
{
    FD3D11Renderer::FD3D11Renderer() = default;
    FD3D11Renderer::~FD3D11Renderer() = default;

    bool FD3D11Renderer::Initialize(const FWindowsWindow& Window)
    {
        const std::uint32_t Width = Window.GetClientWidth();

        const std::uint32_t Height = Window.GetClientHeight();

        if (!CreateDeviceAndSwapChain(Window))
        {
            return false;
        }

        if (!CreateRenderTarget(Width, Height))
        {
            return false;
        }

        if (!CreateDepthStencil(Width, Height))
        {
            return false;
        }

        if (!CreateTransformBuffer())
        {
            return false;
        }

        if (!CreateSamplerState())
        {
            return false;
        }

        AssetManager = std::make_unique<FAssetManager>(*Device.Get());

        if (!LoadAssets())
        {
            return false;
        }

        Camera.SetViewportSize(Width, Height);

        FLog::Info("D3D11 renderer initialized.");

        return true;
    }

    bool FD3D11Renderer::CreateDeviceAndSwapChain(const FWindowsWindow& Window)
    {
        UINT DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        constexpr D3D_FEATURE_LEVEL RequestedFeatureLevels[]{
            D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0
        };

        D3D_FEATURE_LEVEL CreatedFeatureLevel{};

        HRESULT Result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                           nullptr, DeviceFlags,
                                           RequestedFeatureLevels,
                                           static_cast<UINT>(
                                               std::size(
                                                   RequestedFeatureLevels)),
                                           D3D11_SDK_VERSION,
                                           Device.GetAddressOf(),
                                           &CreatedFeatureLevel,
                                           DeviceContext.GetAddressOf());

#if defined(_DEBUG)
        /*
         * Windows Graphics Tools가 설치되지 않은 환경에서는
         * Debug Layer 없이 다시 장치를 생성한다.
         */
        if (FAILED(Result))
        {
            FLog::Info(
                "D3D11 Debug Layer is unavailable. " "Retrying without it.");

            Device.Reset();
            DeviceContext.Reset();

            DeviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

            Result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                       nullptr, DeviceFlags,
                                       RequestedFeatureLevels,
                                       static_cast<UINT>(
                                           std::size(RequestedFeatureLevels)),
                                       D3D11_SDK_VERSION, Device.GetAddressOf(),
                                       &CreatedFeatureLevel,
                                       DeviceContext.GetAddressOf());
        }
#endif

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the D3D11 device.");

            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIDevice> DxgiDevice;

        Result = Device.As(&DxgiDevice);

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the DXGI device.");

            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter;

        Result = DxgiDevice->GetAdapter(Adapter.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the DXGI adapter.");

            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIFactory2> Factory;

        Result = Adapter->GetParent(IID_PPV_ARGS(Factory.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the DXGI factory.");

            return false;
        }

        DXGI_SWAP_CHAIN_DESC1 SwapChainDescription{};
        SwapChainDescription.Width = Window.GetClientWidth();
        SwapChainDescription.Height = Window.GetClientHeight();
        SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        SwapChainDescription.Stereo = false;
        SwapChainDescription.SampleDesc.Count = 1;
        SwapChainDescription.SampleDesc.Quality = 0;
        SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDescription.BufferCount = 2;
        SwapChainDescription.Scaling = DXGI_SCALING_STRETCH;
        SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        SwapChainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        SwapChainDescription.Flags = 0;

        Result = Factory->CreateSwapChainForHwnd(
            Device.Get(), Window.GetNativeHandle(), &SwapChainDescription,
            nullptr, nullptr, SwapChain.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the swap chain.");

            return false;
        }

        /*
         * DXGI가 Alt+Enter 입력을 처리해
         * 임의로 전체 화면 상태를 변경하지 않도록 한다.
         */
        Result = Factory->MakeWindowAssociation(Window.GetNativeHandle(),
                                                DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(Result))
        {
            FLog::Error("Failed to configure the window association.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateRenderTarget(const std::uint32_t Width,
                                            const std::uint32_t Height)
    {
        ENGINE_ASSERT(Device != nullptr);
        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(SwapChain != nullptr);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;

        HRESULT Result = SwapChain->GetBuffer(
            0, IID_PPV_ARGS(BackBuffer.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the swap chain back buffer.");

            return false;
        }

        Result = Device->CreateRenderTargetView(BackBuffer.Get(), nullptr,
                                                RenderTargetView.
                                                GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the render target view.");

            return false;
        }

        D3D11_VIEWPORT Viewport{};
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        Viewport.Width = static_cast<float>(Width);
        Viewport.Height = static_cast<float>(Height);
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;

        DeviceContext->RSSetViewports(1, &Viewport);

        return true;
    }

    bool FD3D11Renderer::CreateDepthStencil(const std::uint32_t Width,
                                            const std::uint32_t Height)
    {
        ENGINE_ASSERT(Device != nullptr);

        if (Width == 0 || Height == 0)
        {
            return true;
        }

        D3D11_TEXTURE2D_DESC TextureDescription{};
        TextureDescription.Width = Width;
        TextureDescription.Height = Height;
        TextureDescription.MipLevels = 1;
        TextureDescription.ArraySize = 1;
        TextureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        TextureDescription.SampleDesc.Count = 1;
        TextureDescription.SampleDesc.Quality = 0;
        TextureDescription.Usage = D3D11_USAGE_DEFAULT;
        TextureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        TextureDescription.CPUAccessFlags = 0;
        TextureDescription.MiscFlags = 0;

        HRESULT Result = Device->CreateTexture2D(
            &TextureDescription, nullptr, DepthStencilTexture.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the depth stencil texture.");

            return false;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC ViewDescription{};

        ViewDescription.Format = TextureDescription.Format;
        ViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        ViewDescription.Texture2D.MipSlice = 0;

        Result = Device->CreateDepthStencilView(DepthStencilTexture.Get(),
                                                &ViewDescription,
                                                DepthStencilView.
                                                GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the depth stencil view.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateTransformBuffer()
    {
        D3D11_BUFFER_DESC Description{};
        Description.ByteWidth = sizeof(FTransformBuffer);
        Description.Usage = D3D11_USAGE_DYNAMIC;
        Description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        Description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        const HRESULT Result = Device->CreateBuffer(
            &Description, nullptr, TransformBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the transform buffer.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateSamplerState()
    {
        D3D11_SAMPLER_DESC Description{};
        Description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        Description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        Description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        Description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        Description.ComparisonFunc = D3D11_COMPARISON_NEVER;
        Description.MinLOD = 0.0f;
        Description.MaxLOD = D3D11_FLOAT32_MAX;

        const HRESULT Result = Device->CreateSamplerState(
            &Description, SamplerState.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the sampler state.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::LoadAssets()
    {
        ENGINE_ASSERT(AssetManager != nullptr);

        CubeMesh = AssetManager->CreateMesh("Cube", CubeVertices, CubeIndices);

        if (CubeMesh == nullptr)
        {
            FLog::Error("Failed to create the cube mesh.");

            return false;
        }

        BasicShader = AssetManager->LoadShader("BasicShader",
                                               L"Shaders/Basic.hlsl");

        if (BasicShader == nullptr)
        {
            FLog::Error("Failed to load the basic shader.");

            return false;
        }

        CheckerTexture = AssetManager->LoadTexture(
            "CheckerTexture", L"Assets/Textures/Checker.png");

        if (CheckerTexture == nullptr)
        {
            FLog::Error("Failed to load the checker texture.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::Resize(const std::uint32_t Width,
                                const std::uint32_t Height)
    {
        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(SwapChain != nullptr);

        /*
         * 창이 최소화되면 Width 또는 Height가 0이 된다.
         * 이 크기로는 SwapChain과 Depth Buffer를 만들 수 없다.
         */
        if (Width == 0 || Height == 0)
        {
            return true;
        }

        /*
         * ResizeBuffers를 호출하기 전에 Context가 가진 참조와
         * 현재 BackBuffer 및 Depth Buffer 참조를 해제한다.
         */
        DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

        RenderTargetView.Reset();
        DepthStencilView.Reset();
        DepthStencilTexture.Reset();

        const HRESULT Result = SwapChain->ResizeBuffers(
            0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);

        if (FAILED(Result))
        {
            FLog::Error("Failed to resize the swap chain.");

            return false;
        }

        if (!CreateRenderTarget(Width, Height))
        {
            return false;
        }

        if (!CreateDepthStencil(Width, Height))
        {
            return false;
        }

        /*
         * 화면 비율이 달라졌으므로
         * Projection 행렬의 Aspect Ratio도 갱신한다.
         */
        Camera.SetViewportSize(Width, Height);

        return true;
    }

    bool FD3D11Renderer::Render(const float DeltaTime)
    {
        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(RenderTargetView != nullptr);
        ENGINE_ASSERT(DepthStencilView != nullptr);
        ENGINE_ASSERT(TransformBuffer != nullptr);
        ENGINE_ASSERT(SamplerState != nullptr);
        ENGINE_ASSERT(CubeMesh != nullptr);
        ENGINE_ASSERT(BasicShader != nullptr);
        ENGINE_ASSERT(CheckerTexture != nullptr);

        RotationRadians += DeltaTime * DirectX::XMConvertToRadians(45.0f);

        if (RotationRadians >= DirectX::XM_2PI)
        {
            RotationRadians -= DirectX::XM_2PI;
        }

        const DirectX::XMMATRIX WorldMatrix =
            DirectX::XMMatrixRotationX(RotationRadians * 0.5f) *
            DirectX::XMMatrixRotationY(RotationRadians);

        const DirectX::XMMATRIX ViewMatrix = Camera.GetViewMatrix();

        const DirectX::XMMATRIX ProjectionMatrix = Camera.GetProjectionMatrix();

        D3D11_MAPPED_SUBRESOURCE MappedResource{};

        HRESULT Result = DeviceContext->Map(TransformBuffer.Get(), 0,
                                            D3D11_MAP_WRITE_DISCARD, 0,
                                            &MappedResource);

        if (FAILED(Result))
        {
            FLog::Error("Failed to map the transform buffer.");

            return false;
        }

        auto* TransformData = static_cast<FTransformBuffer*>(MappedResource.
            pData);

        DirectX::XMStoreFloat4x4(&TransformData->WorldMatrix,
                                 DirectX::XMMatrixTranspose(WorldMatrix));

        DirectX::XMStoreFloat4x4(&TransformData->ViewMatrix,
                                 DirectX::XMMatrixTranspose(ViewMatrix));

        DirectX::XMStoreFloat4x4(&TransformData->ProjectionMatrix,
                                 DirectX::XMMatrixTranspose(ProjectionMatrix));

        DeviceContext->Unmap(TransformBuffer.Get(), 0);

        ID3D11RenderTargetView* RenderTargets[]{RenderTargetView.Get()};

        DeviceContext->OMSetRenderTargets(1, RenderTargets,
                                          DepthStencilView.Get());

        DeviceContext->ClearRenderTargetView(
            RenderTargetView.Get(), ClearColor);

        DeviceContext->ClearDepthStencilView(DepthStencilView.Get(),
                                             D3D11_CLEAR_DEPTH |
                                             D3D11_CLEAR_STENCIL, 1.0f, 0);

        BasicShader->Bind(*DeviceContext.Get());

        ID3D11Buffer* TransformBuffers[]{TransformBuffer.Get()};

        DeviceContext->VSSetConstantBuffers(0, 1, TransformBuffers);

        CheckerTexture->Bind(*DeviceContext.Get(), 0);

        ID3D11SamplerState* Samplers[]{SamplerState.Get()};

        DeviceContext->PSSetSamplers(0, 1, Samplers);

        CubeMesh->Draw(*DeviceContext.Get());

        Result = SwapChain->Present(1, 0);

        if (FAILED(Result))
        {
            FLog::Error("Failed to present the rendered frame.");

            return false;
        }

        return true;
    }
}
