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

#include <array>
#include <cstdint>

namespace
{
    struct FVertex
    {
        float Position[3];
        float Color[4];
    };

    /*
     * D3D11 Constant Buffer의 크기는
     * 반드시 16바이트의 배수여야 한다.
     */
    struct alignas(16) FTransformBuffer
    {
        DirectX::XMFLOAT4X4 WorldMatrix;
        DirectX::XMFLOAT4X4 ViewMatrix;
        DirectX::XMFLOAT4X4 ProjectionMatrix;
    };

    static_assert(
        sizeof(FTransformBuffer) % 16 == 0,
        "Constant buffer size must be a multiple of 16 bytes.");

    constexpr std::array<FVertex, 8> CubeVertices
    {
        FVertex{
            { -1.0f, -1.0f, -1.0f },
            { 1.0f, 0.0f, 0.0f, 1.0f }
        },
        FVertex{
            { -1.0f,  1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f, 1.0f }
        },
        FVertex{
            {  1.0f,  1.0f, -1.0f },
            { 0.0f, 0.0f, 1.0f, 1.0f }
        },
        FVertex{
            {  1.0f, -1.0f, -1.0f },
            { 1.0f, 1.0f, 0.0f, 1.0f }
        },
        FVertex{
            { -1.0f, -1.0f,  1.0f },
            { 1.0f, 0.0f, 1.0f, 1.0f }
        },
        FVertex{
            { -1.0f,  1.0f,  1.0f },
            { 0.0f, 1.0f, 1.0f, 1.0f }
        },
        FVertex{
            {  1.0f,  1.0f,  1.0f },
            { 1.0f, 1.0f, 1.0f, 1.0f }
        },
        FVertex{
            {  1.0f, -1.0f,  1.0f },
            { 0.3f, 0.3f, 0.3f, 1.0f }
        }
    };

    constexpr std::array<std::uint16_t, 36> CubeIndices
    {
        // 앞면
        0, 1, 2,
        0, 2, 3,

        // 뒷면
        4, 6, 5,
        4, 7, 6,

        // 왼쪽 면
        4, 5, 1,
        4, 1, 0,

        // 오른쪽 면
        3, 2, 6,
        3, 6, 7,

        // 위쪽 면
        1, 5, 6,
        1, 6, 2,

        // 아래쪽 면
        4, 0, 3,
        4, 3, 7
    };

    void LogShaderCompileError(
        ID3DBlob* ErrorBlob)
    {
        if (ErrorBlob == nullptr)
        {
            return;
        }

        const auto* ErrorMessage =
            static_cast<const char*>(
                ErrorBlob->GetBufferPointer());

        if (ErrorMessage != nullptr)
        {
            Engine::FLog::Error(ErrorMessage);
        }
    }
}

namespace Engine
{
    bool FD3D11Renderer::Initialize(
        const FWindowsWindow& Window)
    {
        const std::uint32_t Width =
            Window.GetClientWidth();

        const std::uint32_t Height =
            Window.GetClientHeight();

        if (!CreateDeviceAndSwapChain(Window))
        {
            return false;
        }

        if (!CreateRenderTarget(
            Width,
            Height))
        {
            return false;
        }

        if (!CreateDepthStencil(
            Width,
            Height))
        {
            return false;
        }

        if (!CreateShaders())
        {
            return false;
        }

        if (!CreateCubeResources())
        {
            return false;
        }

        if (!CreateTransformBuffer())
        {
            return false;
        }

        Camera.SetViewportSize(
            Width,
            Height);

        FLog::Info(
            "D3D11 renderer initialized.");

        return true;
    }

    bool FD3D11Renderer::CreateDeviceAndSwapChain(
        const FWindowsWindow& Window)
    {
        UINT DeviceFlags =
            D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        DeviceFlags |=
            D3D11_CREATE_DEVICE_DEBUG;
#endif

        constexpr D3D_FEATURE_LEVEL RequestedFeatureLevels[]{
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        D3D_FEATURE_LEVEL CreatedFeatureLevel{};

        HRESULT Result = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            DeviceFlags,
            RequestedFeatureLevels,
            static_cast<UINT>(
                std::size(RequestedFeatureLevels)),
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
                "D3D11 Debug Layer is unavailable. "
                "Retrying without it.");

            Device.Reset();
            DeviceContext.Reset();

            DeviceFlags &=
                ~D3D11_CREATE_DEVICE_DEBUG;

            Result = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                DeviceFlags,
                RequestedFeatureLevels,
                static_cast<UINT>(
                    std::size(RequestedFeatureLevels)),
                D3D11_SDK_VERSION,
                Device.GetAddressOf(),
                &CreatedFeatureLevel,
                DeviceContext.GetAddressOf());
        }
#endif

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the D3D11 device.");

            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIDevice>
            DxgiDevice;

        Result = Device.As(&DxgiDevice);

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to get the DXGI device.");

            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIAdapter>
            Adapter;

        Result = DxgiDevice->GetAdapter(
            Adapter.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to get the DXGI adapter.");

            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIFactory2>
            Factory;

        Result = Adapter->GetParent(
            IID_PPV_ARGS(
                Factory.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to get the DXGI factory.");

            return false;
        }

        DXGI_SWAP_CHAIN_DESC1 SwapChainDescription{};
        SwapChainDescription.Width =
            Window.GetClientWidth();
        SwapChainDescription.Height =
            Window.GetClientHeight();
        SwapChainDescription.Format =
            DXGI_FORMAT_R8G8B8A8_UNORM;
        SwapChainDescription.Stereo = false;
        SwapChainDescription.SampleDesc.Count = 1;
        SwapChainDescription.SampleDesc.Quality = 0;
        SwapChainDescription.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDescription.BufferCount = 2;
        SwapChainDescription.Scaling =
            DXGI_SCALING_STRETCH;
        SwapChainDescription.SwapEffect =
            DXGI_SWAP_EFFECT_FLIP_DISCARD;
        SwapChainDescription.AlphaMode =
            DXGI_ALPHA_MODE_UNSPECIFIED;
        SwapChainDescription.Flags = 0;

        Result = Factory->CreateSwapChainForHwnd(
            Device.Get(),
            Window.GetNativeHandle(),
            &SwapChainDescription,
            nullptr,
            nullptr,
            SwapChain.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the swap chain.");

            return false;
        }

        /*
         * DXGI가 Alt+Enter 입력을 처리해
         * 임의로 전체 화면 상태를 변경하지 않도록 한다.
         */
        Result = Factory->MakeWindowAssociation(
            Window.GetNativeHandle(),
            DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to configure the window association.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateRenderTarget(
        const std::uint32_t Width,
        const std::uint32_t Height)
    {
        ENGINE_ASSERT(Device != nullptr);
        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(SwapChain != nullptr);

        Microsoft::WRL::ComPtr<ID3D11Texture2D>
            BackBuffer;

        HRESULT Result = SwapChain->GetBuffer(
            0,
            IID_PPV_ARGS(
                BackBuffer.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to get the swap chain back buffer.");

            return false;
        }

        Result = Device->CreateRenderTargetView(
            BackBuffer.Get(),
            nullptr,
            RenderTargetView.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the render target view.");

            return false;
        }

        D3D11_VIEWPORT Viewport{};
        Viewport.TopLeftX = 0.0f;
        Viewport.TopLeftY = 0.0f;
        Viewport.Width =
            static_cast<float>(Width);
        Viewport.Height =
            static_cast<float>(Height);
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;

        DeviceContext->RSSetViewports(
            1,
            &Viewport);

        return true;
    }

    bool FD3D11Renderer::CreateDepthStencil(
        const std::uint32_t Width,
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
        TextureDescription.Format =
            DXGI_FORMAT_D24_UNORM_S8_UINT;
        TextureDescription.SampleDesc.Count = 1;
        TextureDescription.SampleDesc.Quality = 0;
        TextureDescription.Usage =
            D3D11_USAGE_DEFAULT;
        TextureDescription.BindFlags =
            D3D11_BIND_DEPTH_STENCIL;
        TextureDescription.CPUAccessFlags = 0;
        TextureDescription.MiscFlags = 0;

        HRESULT Result = Device->CreateTexture2D(
            &TextureDescription,
            nullptr,
            DepthStencilTexture.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the depth stencil texture.");

            return false;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC
            ViewDescription{};

        ViewDescription.Format =
            TextureDescription.Format;
        ViewDescription.ViewDimension =
            D3D11_DSV_DIMENSION_TEXTURE2D;
        ViewDescription.Texture2D.MipSlice = 0;

        Result = Device->CreateDepthStencilView(
            DepthStencilTexture.Get(),
            &ViewDescription,
            DepthStencilView.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the depth stencil view.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateShaders()
    {
        ENGINE_ASSERT(Device != nullptr);

        UINT ShaderFlags =
            D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
        ShaderFlags |=
            D3DCOMPILE_DEBUG |
            D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        ShaderFlags |=
            D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob>
            VertexShaderBlob;

        Microsoft::WRL::ComPtr<ID3DBlob>
            VertexShaderErrorBlob;

        HRESULT Result = D3DCompileFromFile(
            L"Shaders/Basic.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VSMain",
            "vs_5_0",
            ShaderFlags,
            0,
            VertexShaderBlob.GetAddressOf(),
            VertexShaderErrorBlob.GetAddressOf());

        if (FAILED(Result))
        {
            LogShaderCompileError(
                VertexShaderErrorBlob.Get());

            FLog::Error(
                "Failed to compile the vertex shader.");

            return false;
        }

        Result = Device->CreateVertexShader(
            VertexShaderBlob->GetBufferPointer(),
            VertexShaderBlob->GetBufferSize(),
            nullptr,
            VertexShader.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the vertex shader.");

            return false;
        }

        constexpr D3D11_INPUT_ELEMENT_DESC
            InputElements[]
        {
            {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                0,
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                12,
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };

        Result = Device->CreateInputLayout(
            InputElements,
            static_cast<UINT>(
                std::size(InputElements)),
            VertexShaderBlob->GetBufferPointer(),
            VertexShaderBlob->GetBufferSize(),
            InputLayout.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the input layout.");

            return false;
        }

        Microsoft::WRL::ComPtr<ID3DBlob>
            PixelShaderBlob;

        Microsoft::WRL::ComPtr<ID3DBlob>
            PixelShaderErrorBlob;

        Result = D3DCompileFromFile(
            L"Shaders/Basic.hlsl",
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PSMain",
            "ps_5_0",
            ShaderFlags,
            0,
            PixelShaderBlob.GetAddressOf(),
            PixelShaderErrorBlob.GetAddressOf());

        if (FAILED(Result))
        {
            LogShaderCompileError(
                PixelShaderErrorBlob.Get());

            FLog::Error(
                "Failed to compile the pixel shader.");

            return false;
        }

        Result = Device->CreatePixelShader(
            PixelShaderBlob->GetBufferPointer(),
            PixelShaderBlob->GetBufferSize(),
            nullptr,
            PixelShader.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the pixel shader.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateCubeResources()
    {
        ENGINE_ASSERT(Device != nullptr);

        D3D11_BUFFER_DESC
            VertexBufferDescription{};

        VertexBufferDescription.ByteWidth =
            static_cast<UINT>(
                CubeVertices.size() *
                sizeof(FVertex));

        VertexBufferDescription.Usage =
            D3D11_USAGE_IMMUTABLE;

        VertexBufferDescription.BindFlags =
            D3D11_BIND_VERTEX_BUFFER;

        VertexBufferDescription.CPUAccessFlags = 0;
        VertexBufferDescription.MiscFlags = 0;
        VertexBufferDescription.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA VertexData{};
        VertexData.pSysMem =
            CubeVertices.data();

        HRESULT Result = Device->CreateBuffer(
            &VertexBufferDescription,
            &VertexData,
            VertexBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the cube vertex buffer.");

            return false;
        }

        D3D11_BUFFER_DESC
            IndexBufferDescription{};

        IndexBufferDescription.ByteWidth =
            static_cast<UINT>(
                CubeIndices.size() *
                sizeof(std::uint16_t));

        IndexBufferDescription.Usage =
            D3D11_USAGE_IMMUTABLE;

        IndexBufferDescription.BindFlags =
            D3D11_BIND_INDEX_BUFFER;

        IndexBufferDescription.CPUAccessFlags = 0;
        IndexBufferDescription.MiscFlags = 0;
        IndexBufferDescription.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA IndexData{};
        IndexData.pSysMem =
            CubeIndices.data();

        Result = Device->CreateBuffer(
            &IndexBufferDescription,
            &IndexData,
            IndexBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the cube index buffer.");

            return false;
        }

        IndexCount =
            static_cast<std::uint32_t>(
                CubeIndices.size());

        return true;
    }

    bool FD3D11Renderer::CreateTransformBuffer()
    {
        ENGINE_ASSERT(Device != nullptr);

        D3D11_BUFFER_DESC BufferDescription{};

        BufferDescription.ByteWidth =
            static_cast<UINT>(
                sizeof(FTransformBuffer));

        BufferDescription.Usage =
            D3D11_USAGE_DYNAMIC;

        BufferDescription.BindFlags =
            D3D11_BIND_CONSTANT_BUFFER;

        BufferDescription.CPUAccessFlags =
            D3D11_CPU_ACCESS_WRITE;

        BufferDescription.MiscFlags = 0;
        BufferDescription.StructureByteStride = 0;

        const HRESULT Result = Device->CreateBuffer(
            &BufferDescription,
            nullptr,
            TransformBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to create the transform constant buffer.");

            return false;
        }

        return true;
    }

    bool FD3D11Renderer::Resize(
        const std::uint32_t Width,
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
        DeviceContext->OMSetRenderTargets(
            0,
            nullptr,
            nullptr);

        RenderTargetView.Reset();
        DepthStencilView.Reset();
        DepthStencilTexture.Reset();

        const HRESULT Result = SwapChain->ResizeBuffers(
            0,
            Width,
            Height,
            DXGI_FORMAT_UNKNOWN,
            0);

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to resize the swap chain.");

            return false;
        }

        if (!CreateRenderTarget(
            Width,
            Height))
        {
            return false;
        }

        if (!CreateDepthStencil(
            Width,
            Height))
        {
            return false;
        }

        /*
         * 화면 비율이 달라졌으므로
         * Projection 행렬의 Aspect Ratio도 갱신한다.
         */
        Camera.SetViewportSize(
            Width,
            Height);

        return true;
    }

    bool FD3D11Renderer::Render(
        const float DeltaTime)
    {
        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(RenderTargetView != nullptr);
        ENGINE_ASSERT(DepthStencilView != nullptr);
        ENGINE_ASSERT(VertexBuffer != nullptr);
        ENGINE_ASSERT(IndexBuffer != nullptr);
        ENGINE_ASSERT(TransformBuffer != nullptr);
        ENGINE_ASSERT(VertexShader != nullptr);
        ENGINE_ASSERT(PixelShader != nullptr);
        ENGINE_ASSERT(InputLayout != nullptr);

        /*
         * 초당 45도씩 회전한다.
         * DeltaTime을 사용하므로 프레임 속도와 무관하게
         * 동일한 속도로 회전한다.
         */
        RotationRadians +=
            DeltaTime *
            DirectX::XMConvertToRadians(45.0f);

        if (RotationRadians >= DirectX::XM_2PI)
        {
            RotationRadians -= DirectX::XM_2PI;
        }

        const DirectX::XMMATRIX WorldMatrix =
            DirectX::XMMatrixRotationX(
                RotationRadians * 0.5f) *
            DirectX::XMMatrixRotationY(
                RotationRadians);

        const DirectX::XMMATRIX ViewMatrix =
            Camera.GetViewMatrix();

        const DirectX::XMMATRIX ProjectionMatrix =
            Camera.GetProjectionMatrix();

        /*
         * 매 프레임 변경되는 Transform 데이터를
         * Constant Buffer에 기록한다.
         */
        D3D11_MAPPED_SUBRESOURCE MappedResource{};

        HRESULT Result = DeviceContext->Map(
            TransformBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &MappedResource);

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to map the transform constant buffer.");

            return false;
        }

        auto* TransformData =
            static_cast<FTransformBuffer*>(
                MappedResource.pData);

        /*
         * DirectXMath와 HLSL의 행렬 저장 규칙을 맞추기 위해
         * GPU로 전달하기 전에 각 행렬을 전치한다.
         */
        DirectX::XMStoreFloat4x4(
            &TransformData->WorldMatrix,
            DirectX::XMMatrixTranspose(
                WorldMatrix));

        DirectX::XMStoreFloat4x4(
            &TransformData->ViewMatrix,
            DirectX::XMMatrixTranspose(
                ViewMatrix));

        DirectX::XMStoreFloat4x4(
            &TransformData->ProjectionMatrix,
            DirectX::XMMatrixTranspose(
                ProjectionMatrix));

        DeviceContext->Unmap(
            TransformBuffer.Get(),
            0);

        ID3D11RenderTargetView* RenderTargets[]{
            RenderTargetView.Get()
        };

        DeviceContext->OMSetRenderTargets(
            1,
            RenderTargets,
            DepthStencilView.Get());

        DeviceContext->ClearRenderTargetView(
            RenderTargetView.Get(),
            ClearColor);

        DeviceContext->ClearDepthStencilView(
            DepthStencilView.Get(),
            D3D11_CLEAR_DEPTH |
            D3D11_CLEAR_STENCIL,
            1.0f,
            0);

        const UINT Stride =
            sizeof(FVertex);

        const UINT Offset = 0;

        ID3D11Buffer* VertexBuffers[]{
            VertexBuffer.Get()
        };

        DeviceContext->IASetVertexBuffers(
            0,
            1,
            VertexBuffers,
            &Stride,
            &Offset);

        DeviceContext->IASetIndexBuffer(
            IndexBuffer.Get(),
            DXGI_FORMAT_R16_UINT,
            0);

        DeviceContext->IASetInputLayout(
            InputLayout.Get());

        DeviceContext->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        DeviceContext->VSSetShader(
            VertexShader.Get(),
            nullptr,
            0);

        ID3D11Buffer* ConstantBuffers[]{
            TransformBuffer.Get()
        };

        /*
         * HLSL의 register(b0)에
         * Transform Constant Buffer를 연결한다.
         */
        DeviceContext->VSSetConstantBuffers(
            0,
            1,
            ConstantBuffers);

        DeviceContext->PSSetShader(
            PixelShader.Get(),
            nullptr,
            0);

        DeviceContext->DrawIndexed(
            IndexCount,
            0,
            0);

        Result = SwapChain->Present(
            1,
            0);

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to present the rendered frame.");

            return false;
        }

        return true;
    }
}