/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.13
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : Windows Imaging Component를 이용한 Texture 로딩 구현
 */

#include "Renderer/Resources/Texture.h"

#include <Engine/Core/Assert.h>
#include <Engine/Core/Log.h>

#include <objbase.h>
#include <wincodec.h>

#include <cstdint>
#include <limits>
#include <vector>

namespace
{
    class FScopedComInitializer final
    {
    public:
        FScopedComInitializer()
            : Result{CoInitializeEx(nullptr, COINIT_MULTITHREADED)}
        {
        }

        ~FScopedComInitializer()
        {
            if (SUCCEEDED(Result))
            {
                CoUninitialize();
            }
        }

        [[nodiscard]] bool IsAvailable() const noexcept
        {
            /*
             * RPC_E_CHANGED_MODE은 현재 스레드가 이미 다른 방식으로
             * COM 초기화됐다는 의미이므로 COM 자체는 사용할 수 있다.
             */
            return SUCCEEDED(Result) || Result == RPC_E_CHANGED_MODE;
        }

    private:
        HRESULT Result;
    };
}

namespace Engine
{
    bool FTexture::Initialize(ID3D11Device& Device,
                              const std::filesystem::path& FilePath)
    {
        FScopedComInitializer ComInitializer;

        if (!ComInitializer.IsAvailable())
        {
            FLog::Error("Failed to initialize COM for texture loading.");

            return false;
        }

        Microsoft::WRL::ComPtr<IWICImagingFactory> ImagingFactory;

        HRESULT Result = CoCreateInstance(
            CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(ImagingFactory.GetAddressOf()));

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the WIC imaging factory.");

            return false;
        }

        Microsoft::WRL::ComPtr<IWICBitmapDecoder> Decoder;

        Result = ImagingFactory->CreateDecoderFromFilename(
            FilePath.c_str(), nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnLoad, Decoder.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to open the texture file.");

            return false;
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> Frame;

        Result = Decoder->GetFrame(0, Frame.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to decode the texture frame.");

            return false;
        }

        Microsoft::WRL::ComPtr<IWICFormatConverter> Converter;

        Result = ImagingFactory->CreateFormatConverter(
            Converter.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the texture format converter.");

            return false;
        }

        Result = Converter->Initialize(Frame.Get(),
                                       GUID_WICPixelFormat32bppRGBA,
                                       WICBitmapDitherTypeNone, nullptr, 0.0,
                                       WICBitmapPaletteTypeCustom);

        if (FAILED(Result))
        {
            FLog::Error("Failed to convert the texture to RGBA.");

            return false;
        }

        UINT LoadedWidth = 0;
        UINT LoadedHeight = 0;

        Result = Converter->GetSize(&LoadedWidth, &LoadedHeight);

        if (FAILED(Result) || LoadedWidth == 0 || LoadedHeight == 0)
        {
            FLog::Error("The texture has an invalid size.");

            return false;
        }

        const std::uint64_t RowPitch64 = static_cast<std::uint64_t>(LoadedWidth)
            * 4;

        const std::uint64_t ImageSize64 = RowPitch64 * LoadedHeight;

        if (RowPitch64 > std::numeric_limits<UINT>::max() || ImageSize64 >
            std::numeric_limits<UINT>::max())
        {
            FLog::Error("The texture is too large.");

            return false;
        }

        const UINT RowPitch = static_cast<UINT>(RowPitch64);

        const UINT ImageSize = static_cast<UINT>(ImageSize64);

        std::vector<std::uint8_t> PixelData(ImageSize);

        Result = Converter->CopyPixels(nullptr, RowPitch, ImageSize,
                                       PixelData.data());

        if (FAILED(Result))
        {
            FLog::Error("Failed to copy the texture pixels.");

            return false;
        }

        D3D11_TEXTURE2D_DESC TextureDescription{};
        TextureDescription.Width = LoadedWidth;
        TextureDescription.Height = LoadedHeight;
        TextureDescription.MipLevels = 1;
        TextureDescription.ArraySize = 1;
        TextureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        TextureDescription.SampleDesc.Count = 1;
        TextureDescription.Usage = D3D11_USAGE_IMMUTABLE;
        TextureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA TextureData{};
        TextureData.pSysMem = PixelData.data();
        TextureData.SysMemPitch = RowPitch;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture;

        Result = Device.CreateTexture2D(&TextureDescription, &TextureData,
                                        Texture.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the D3D11 texture.");

            return false;
        }

        Result = Device.CreateShaderResourceView(
            Texture.Get(), nullptr, ShaderResourceView.GetAddressOf());

        if (FAILED(Result))
        {
            FLog::Error("Failed to create the texture shader resource view.");

            return false;
        }

        Width = LoadedWidth;
        Height = LoadedHeight;

        return true;
    }

    void FTexture::Bind(ID3D11DeviceContext& DeviceContext,
                        const std::uint32_t Slot) const
    {
        ENGINE_ASSERT(ShaderResourceView != nullptr);

        ID3D11ShaderResourceView* Views[]{ShaderResourceView.Get()};

        DeviceContext.PSSetShaderResources(Slot, 1, Views);
    }

    std::uint32_t FTexture::GetWidth() const noexcept
    {
        return Width;
    }

    std::uint32_t FTexture::GetHeight() const noexcept
    {
        return Height;
    }
}
