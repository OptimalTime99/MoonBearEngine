/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.09.14
 *
 * 파일 개요 : ImGui 기반 최소 Scene Editor
 */

#pragma once

#include "Platform/Windows/WindowsMessageHandler.h"

#include <cstdint>
#include <filesystem>
#include <string>

namespace Engine
{
    class FD3D11Renderer;
    class FGameObject;
    class FScene;
    class FWindowsWindow;

    class FEditor final : public IWindowsMessageHandler
    {
    public:
        FEditor() = default;
        ~FEditor();

        FEditor(const FEditor&) = delete;
        FEditor& operator=(const FEditor&) = delete;

        bool Initialize(FWindowsWindow& Window, FD3D11Renderer& Renderer);

        void Shutdown();

        void BeginFrame();
        void Draw(FScene& Scene);
        void EndFrame();

        bool ProcessWindowMessage(HWND WindowHandle, UINT Message,
                                  WPARAM WParam, LPARAM LParam,
                                  LRESULT& OutResult) override;

    private:
        void DrawMainMenu(FScene& Scene);
        void DrawHierarchy(FScene& Scene);
        void DrawInspector(FScene& Scene);

        FGameObject& CreateCube(FScene& Scene);

    private:
        FWindowsWindow* Window = nullptr;
        FGameObject* SelectedObject = nullptr;

        std::filesystem::path ScenePath{L"Assets/Scenes/Main.scene.json"};

        std::string StatusMessage;

        std::uint32_t NewObjectIndex = 0;

        bool bInitialized = false;
    };
}
