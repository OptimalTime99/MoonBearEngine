/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : DirectX 11 초기화, 셰이더 컴파일과 삼각형 렌더링 구현
 * Flip Model SwapChain을 사용하고 창 크기 변경 시 RenderTarget을 재생성
 */

#include "Renderer/D3D11/D3D11Renderer.h"

#include "Platform/Windows/WindowsWindow.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

#include <d3dcompiler.h>

#include <array>
#include <cstddef>

namespace Engine
{
    namespace
    {
        struct FVertex
        {
            float Position[3];
            float Color[4];
        };

        constexpr std::array<FVertex, 3> TriangleVertices{
            FVertex{
                { 0.0f, 0.5f, 0.0f },
                { 1.0f, 0.0f, 0.0f, 1.0f }
            },
            FVertex{
                { 0.5f, -0.5f, 0.0f },
                { 0.0f, 1.0f, 0.0f, 1.0f }
            },
            FVertex{
                { -0.5f, -0.5f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 1.0f }
            }
        };

        constexpr float ClearColor[4]{
            0.08f,
            0.10f,
            0.15f,
            1.0f
        };

        [[nodiscard]]
        bool CompileShader(
            const wchar_t* ShaderPath,
            const char* EntryPoint,
            const char* Target,
            ID3DBlob** OutShaderBlob)
        {
            ENGINE_ASSERT(ShaderPath != nullptr);
            ENGINE_ASSERT(EntryPoint != nullptr);
            ENGINE_ASSERT(Target != nullptr);
            ENGINE_ASSERT(OutShaderBlob != nullptr);

            UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
            CompileFlags |= D3DCOMPILE_DEBUG;
            CompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

            Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;

            const HRESULT Result = ::D3DCompileFromFile(
                ShaderPath,
                nullptr,
                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                EntryPoint,
                Target,
                CompileFlags,
                0,
                OutShaderBlob,
                ErrorBlob.GetAddressOf());

            if (FAILED(Result))
            {
                if (ErrorBlob != nullptr)
                {
                    const auto* ErrorMessage =
                        static_cast<const char*>(
                            ErrorBlob->GetBufferPointer());

                    FLog::Error(ErrorMessage);
                }
                else
                {
                    FLog::Error("Failed to compile the shader.");
                }

                return false;
            }

            return true;
        }
    }

    FD3D11Renderer::~FD3D11Renderer()
    {
        if (DeviceContext != nullptr)
        {
            /*
             * Context가 잡고 있는 GPU 리소스 참조를 해제한 뒤
             * ComPtr 멤버들이 역순으로 안전하게 정리되도록 한다.
             */
            DeviceContext->ClearState();
            DeviceContext->Flush();
        }
    }

    bool FD3D11Renderer::Initialize(
        const FWindowsWindow& Window)
    {
        if (!CreateDevice())
        {
            return false;
        }

        if (!CreateSwapChain(Window))
        {
            return false;
        }

        if (!CreateRenderTarget(
            Window.GetClientWidth(),
            Window.GetClientHeight()))
        {
            return false;
        }

        if (!CreateTriangleResources())
        {
            return false;
        }

        FLog::Info("D3D11 renderer initialized.");

        return true;
    }

    bool FD3D11Renderer::CreateDevice()
    {
        UINT CreationFlags = 0;

#if defined(_DEBUG)
        CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT Result = ::D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            CreationFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            Device.GetAddressOf(),
            nullptr,
            DeviceContext.GetAddressOf());

#if defined(_DEBUG)
        if (Result == DXGI_ERROR_SDK_COMPONENT_MISSING)
        {
            /*
             * Graphics Tools가 설치되지 않은 환경에서도 실행할 수
             * 있도록 Debug Layer 없이 Device 생성을 다시 시도한다.
             */
            FLog::Info(
                "D3D11 Debug Layer is unavailable. "
                "Retrying without it.");

            CreationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

            Result = ::D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                CreationFlags,
                nullptr,
                0,
                D3D11_SDK_VERSION,
                Device.GetAddressOf(),
                nullptr,
                DeviceContext.GetAddressOf());
        }
#endif

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the D3D11 device.");
            return false;
        }

        return true;
    }

    bool FD3D11Renderer::CreateSwapChain(
        const FWindowsWindow& Window)
    {
        Microsoft::WRL::ComPtr<IDXGIDevice> DXGIDevice;

        HRESULT Result = Device.As(&DXGIDevice);

        if (FAILED(Result))
        {
            FLog::Error("Failed to query IDXGIDevice.");
            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIAdapter> Adapter;

        Result = DXGIDevice->GetAdapter(
            Adapter.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the DXGI adapter.");
            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIFactory2> Factory;

        Result = Adapter->GetParent(
            IID_PPV_ARGS(Factory.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the DXGI factory.");
            return false;
        }

        DXGI_SWAP_CHAIN_DESC1 SwapChainDescription{};
        SwapChainDescription.Width = 0;
        SwapChainDescription.Height = 0;
        SwapChainDescription.Format =
            DXGI_FORMAT_R8G8B8A8_UNORM;
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
            DXGI_ALPHA_MODE_IGNORE;

        Result = Factory->CreateSwapChainForHwnd(
            Device.Get(),
            Window.GetNativeHandle(),
            &SwapChainDescription,
            nullptr,
            nullptr,
            SwapChain.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the swap chain.");
            return false;
        }

        Result = Factory->MakeWindowAssociation(
            Window.GetNativeHandle(),
            DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(Result))
        {
            FLog::Error(
                "Failed to disable the default Alt+Enter behavior.");
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
        ENGINE_ASSERT(Width > 0);
        ENGINE_ASSERT(Height > 0);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;

        HRESULT Result = SwapChain->GetBuffer(
            0,
            IID_PPV_ARGS(BackBuffer.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error("Failed to get the swap chain back buffer.");
            return false;
        }

        Result = Device->CreateRenderTargetView(
            BackBuffer.Get(),
            nullptr,
            RenderTargetView.GetAddressOf());

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

        DeviceContext->RSSetViewports(
            1,
            &Viewport);

        return true;
    }

    bool FD3D11Renderer::CreateTriangleResources()
    {
        Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderBlob;

        if (!CompileShader(
            L"Shaders/Basic.hlsl",
            "VSMain",
            "vs_5_0",
            VertexShaderBlob.GetAddressOf()))
        {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderBlob;

        if (!CompileShader(
            L"Shaders/Basic.hlsl",
            "PSMain",
            "ps_5_0",
            PixelShaderBlob.GetAddressOf()))
        {
            return false;
        }

        HRESULT Result = Device->CreateVertexShader(
            VertexShaderBlob->GetBufferPointer(),
            VertexShaderBlob->GetBufferSize(),
            nullptr,
            VertexShader.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the vertex shader.");
            return false;
        }

        Result = Device->CreatePixelShader(
            PixelShaderBlob->GetBufferPointer(),
            PixelShaderBlob->GetBufferSize(),
            nullptr,
            PixelShader.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the pixel shader.");
            return false;
        }

        constexpr D3D11_INPUT_ELEMENT_DESC InputElements[]{
            {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32B32_FLOAT,
                0,
                offsetof(FVertex, Position),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },
            {
                "COLOR",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                offsetof(FVertex, Color),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };

        Result = Device->CreateInputLayout(
            InputElements,
            static_cast<UINT>(std::size(InputElements)),
            VertexShaderBlob->GetBufferPointer(),
            VertexShaderBlob->GetBufferSize(),
            InputLayout.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the input layout.");
            return false;
        }

        D3D11_BUFFER_DESC VertexBufferDescription{};
        VertexBufferDescription.ByteWidth =
            static_cast<UINT>(sizeof(TriangleVertices));
        VertexBufferDescription.Usage =
            D3D11_USAGE_IMMUTABLE;
        VertexBufferDescription.BindFlags =
            D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA VertexData{};
        VertexData.pSysMem = TriangleVertices.data();

        Result = Device->CreateBuffer(
            &VertexBufferDescription,
            &VertexData,
            VertexBuffer.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the vertex buffer.");
            return false;
        }

        return true;
    }

    bool FD3D11Renderer::Resize(
        const std::uint32_t Width,
        const std::uint32_t Height)
    {
        if (Width == 0 || Height == 0)
        {
            return true;
        }

        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(SwapChain != nullptr);

        /*
         * SwapChain BackBuffer의 크기를 바꾸기 전에 Context와
         * RenderTargetView가 가지고 있는 기존 참조를 해제한다.
         */
        DeviceContext->OMSetRenderTargets(
            0,
            nullptr,
            nullptr);

        RenderTargetView.Reset();

        const HRESULT Result = SwapChain->ResizeBuffers(
            0,
            Width,
            Height,
            DXGI_FORMAT_UNKNOWN,
            0);

        if (FAILED(Result))
        {
            FLog::Error("Failed to resize the swap chain.");
            return false;
        }

        return CreateRenderTarget(Width, Height);
    }

    bool FD3D11Renderer::Render()
    {
        ENGINE_ASSERT(DeviceContext != nullptr);
        ENGINE_ASSERT(RenderTargetView != nullptr);
        ENGINE_ASSERT(VertexBuffer != nullptr);
        ENGINE_ASSERT(VertexShader != nullptr);
        ENGINE_ASSERT(PixelShader != nullptr);
        ENGINE_ASSERT(InputLayout != nullptr);

        ID3D11RenderTargetView* RenderTargets[]{
            RenderTargetView.Get()
        };

        DeviceContext->OMSetRenderTargets(
            1,
            RenderTargets,
            nullptr);

        DeviceContext->ClearRenderTargetView(
            RenderTargetView.Get(),
            ClearColor);

        const UINT Stride = sizeof(FVertex);
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

        DeviceContext->IASetInputLayout(
            InputLayout.Get());

        DeviceContext->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        DeviceContext->VSSetShader(
            VertexShader.Get(),
            nullptr,
            0);

        DeviceContext->PSSetShader(
            PixelShader.Get(),
            nullptr,
            0);

        DeviceContext->Draw(
            static_cast<UINT>(TriangleVertices.size()),
            0);

        const HRESULT Result = SwapChain->Present(
            1,
            0);

        if (FAILED(Result))
        {
            FLog::Error("Failed to present the rendered frame.");
            return false;
        }

        return true;
    }
}