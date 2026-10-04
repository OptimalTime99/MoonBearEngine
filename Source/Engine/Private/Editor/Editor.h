/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : ImGui 기반 Scene Editor와
 * Edit/Play Mode 사용자 명령 관리
 */

#pragma once

#include "Platform/Windows/WindowsMessageHandler.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace Engine
{
    enum class EEngineMode
    {
        Edit,
        Play
    };

    enum class EEditorCommand
    {
        None,
        BeginPlay,
        EndPlay
    };

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

        [[nodiscard]] EEditorCommand Draw(FScene& Scene, EEngineMode Mode,
                                          std::string_view PlayStatusText);

        void ClearSelection() noexcept;

        void EndFrame();

        bool ProcessWindowMessage(HWND WindowHandle, UINT Message,
                                  WPARAM WParam, LPARAM LParam,
                                  LRESULT& OutResult) override;

    private:
        [[nodiscard]] EEditorCommand DrawMainMenu(
            FScene& Scene, EEngineMode Mode, std::string_view PlayStatusText);

        void DrawHierarchy(FScene& Scene, EEngineMode Mode);

        void DrawInspector(FScene& Scene, EEngineMode Mode);

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
