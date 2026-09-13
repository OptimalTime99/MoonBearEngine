/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : HLSL 컴파일과 D3D11 Shader 자원 생성 구현
 */

#include "Renderer/Resources/Shader.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

#include "Renderer/Resources/RenderTypes.h"

#include <d3dcompiler.h>

#include <array>
#include <cstddef>
#include <string>

namespace
{
    void LogCompileError(ID3DBlob* ErrorBlob)
    {
        if (ErrorBlob == nullptr)
        {
            return;
        }

        const auto* Message = static_cast<const char*>(ErrorBlob->
            GetBufferPointer());

        if (Message != nullptr)
        {
            Engine::FLog::Error(Message);
        }
    }
}

namespace Engine
{
    bool FShader::Initialize(ID3D11Device& Device,
                             const std::filesystem::path& FilePath,
                             const std::string_view VertexEntryPoint,
                             const std::string_view PixelEntryPoint)
    {
        UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
        CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        const std::string VertexEntry{VertexEntryPoint};

        const std::string PixelEntry{PixelEntryPoint};

        Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderBlob;

        Microsoft::WRL::ComPtr<ID3DBlob> ErrorBlob;

        HRESULT Result = D3DCompileFromFile(FilePath.c_str(), nullptr,
                                            D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                            VertexEntry.c_str(), "vs_5_0",
                                            CompileFlags, 0,
                                            VertexShaderBlob.GetAddressOf(),
                                            ErrorBlob.GetAddressOf());

        if (FAILED(Result))
        {
            LogCompileError(ErrorBlob.Get());

            FLog::Error("Failed to compile the vertex shader.");

            return false;
        }

        Result = Device.CreateVertexShader(VertexShaderBlob->GetBufferPointer(),
                                           VertexShaderBlob->GetBufferSize(),
                                           nullptr,
                                           VertexShader.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the vertex shader.");

            return false;
        }

        constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 3> InputElements{
            {
                {
                    "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                    static_cast<UINT>(offsetof(FVertex, Position)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0
                },
                {
                    "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
                    static_cast<UINT>(offsetof(FVertex, Color)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0
                },
                {
                    "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                    static_cast<UINT>(offsetof(FVertex, TextureCoordinate)),
                    D3D11_INPUT_PER_VERTEX_DATA, 0
                }
            }
        };

        Result = Device.CreateInputLayout(InputElements.data(),
                                          static_cast<UINT>(InputElements.
                                              size()),
                                          VertexShaderBlob->GetBufferPointer(),
                                          VertexShaderBlob->GetBufferSize(),
                                          InputLayout.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the input layout.");

            return false;
        }

        ErrorBlob.Reset();

        Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderBlob;

        Result = D3DCompileFromFile(FilePath.c_str(), nullptr,
                                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                    PixelEntry.c_str(), "ps_5_0", CompileFlags,
                                    0, PixelShaderBlob.GetAddressOf(),
                                    ErrorBlob.GetAddressOf());

        if (FAILED(Result))
        {
            LogCompileError(ErrorBlob.Get());

            FLog::Error("Failed to compile the pixel shader.");

            return false;
        }

        Result = Device.CreatePixelShader(PixelShaderBlob->GetBufferPointer(),
                                          PixelShaderBlob->GetBufferSize(),
                                          nullptr, PixelShader.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the pixel shader.");

            return false;
        }

        return true;
    }

    void FShader::Bind(ID3D11DeviceContext& DeviceContext) const
    {
        ENGINE_ASSERT(VertexShader != nullptr);
        ENGINE_ASSERT(PixelShader != nullptr);
        ENGINE_ASSERT(InputLayout != nullptr);

        DeviceContext.IASetInputLayout(InputLayout.Get());

        DeviceContext.VSSetShader(VertexShader.Get(), nullptr, 0);

        DeviceContext.PSSetShader(PixelShader.Get(), nullptr, 0);
    }
}
