/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : ImGui 초기화와 Hierarchy, Inspector,
 * Scene 저장 및 Edit/Play Mode 제어 UI 구현
 */

#include "Editor/Editor.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Renderer/D3D11/D3D11Renderer.h"
#include "Serialization/SceneSerializer.h"

#include <Engine/Core/Log.h>
#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/MeshRendererComponent.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Transform.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <array>
#include <cstdio>
#include <string>

/*
 * imgui_impl_win32.h는 Windows.h 의존성 전파를 막기 위해
 * Win32 메시지 처리 함수 선언을 노출하지 않는다.
 */
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

namespace Engine
{
    FEditor::~FEditor()
    {
        Shutdown();
    }

    bool FEditor::Initialize(FWindowsWindow& InWindow, FD3D11Renderer& Renderer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& InputOutput = ImGui::GetIO();

        InputOutput.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(InWindow.GetNativeHandle()))
        {
            FLog::Error("Failed to initialize the ImGui Win32 backend.");

            ImGui::DestroyContext();

            return false;
        }

        if (!ImGui_ImplDX11_Init(&Renderer.GetDevice(),
                                 &Renderer.GetDeviceContext()))
        {
            FLog::Error("Failed to initialize the ImGui DX11 backend.");

            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();

            return false;
        }

        Window = &InWindow;
        Window->SetMessageHandler(this);

        bInitialized = true;

        FLog::Info("Editor initialized.");

        return true;
    }

    void FEditor::Shutdown()
    {
        if (!bInitialized)
        {
            return;
        }

        if (Window != nullptr)
        {
            Window->SetMessageHandler(nullptr);
            Window = nullptr;
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        SelectedObject = nullptr;
        bInitialized = false;

        FLog::Info("Editor shut down.");
    }

    void FEditor::BeginFrame()
    {
        if (!bInitialized)
        {
            return;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    EEditorCommand FEditor::Draw(FScene& Scene, const EEngineMode Mode,
                                 const std::string_view PlayStatusText)
    {
        if (!bInitialized)
        {
            return EEditorCommand::None;
        }

        const EEditorCommand Command =
            DrawMainMenu(Scene, Mode, PlayStatusText);

        DrawHierarchy(Scene, Mode);
        DrawInspector(Scene, Mode);

        return Command;
    }

    void FEditor::ClearSelection() noexcept
    {
        SelectedObject = nullptr;
    }

    void FEditor::EndFrame()
    {
        if (!bInitialized)
        {
            return;
        }

        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    bool FEditor::ProcessWindowMessage(const HWND WindowHandle,
                                       const UINT Message, const WPARAM WParam,
                                       const LPARAM LParam, LRESULT& OutResult)
    {
        if (!bInitialized)
        {
            return false;
        }

        OutResult = ImGui_ImplWin32_WndProcHandler(
            WindowHandle, Message, WParam, LParam);

        return OutResult != 0;
    }

    EEditorCommand FEditor::DrawMainMenu(FScene& Scene, const EEngineMode Mode,
                                         const std::string_view PlayStatusText)
    {
        EEditorCommand Command = EEditorCommand::None;

        if (!ImGui::BeginMainMenuBar())
        {
            return Command;
        }

        const bool bIsPlaying = Mode == EEngineMode::Play;

        /*
         * Play 중에는 편집 Scene을 파일로 저장하거나
         * 다른 Scene으로 교체할 수 없다.
         */
        ImGui::BeginDisabled(bIsPlaying);

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                StatusMessage = FSceneSerializer::Save(Scene, ScenePath)
                                    ? "Scene saved."
                                    : "Failed to save scene.";
            }

            if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            {
                SelectedObject = nullptr;

                StatusMessage = FSceneSerializer::Load(Scene, ScenePath)
                                    ? "Scene loaded."
                                    : "Failed to load scene.";
            }

            ImGui::EndMenu();
        }

        ImGui::EndDisabled();

        ImGui::Separator();

        if (!bIsPlaying)
        {
            if (ImGui::Button("Play"))
            {
                Command = EEditorCommand::BeginPlay;
            }
        }
        else
        {
            if (ImGui::Button("Stop"))
            {
                Command = EEditorCommand::EndPlay;
            }
        }

        ImGui::Separator();

        if (!PlayStatusText.empty())
        {
            ImGui::TextUnformatted(PlayStatusText.data(),
                                   PlayStatusText.data() + PlayStatusText.
                                   size());
        }

        if (!StatusMessage.empty())
        {
            ImGui::Separator();

            ImGui::TextUnformatted(StatusMessage.c_str());
        }

        ImGui::EndMainMenuBar();

        return Command;
    }

    void FEditor::DrawHierarchy(FScene& Scene, const EEngineMode Mode)
    {
        ImGui::SetNextWindowSize(ImVec2{280.0f, 500.0f},
                                 ImGuiCond_FirstUseEver);

        ImGui::Begin("Hierarchy");

        const bool bIsPlaying = Mode == EEngineMode::Play;

        /*
         * Play 중에도 객체 선택은 가능하지만
         * 객체 생성은 불가능하다.
         */
        ImGui::BeginDisabled(bIsPlaying);

        if (ImGui::Button("Create Empty"))
        {
            SelectedObject = &Scene.CreateGameObject(
                "GameObject " + std::to_string(++NewObjectIndex));
        }

        ImGui::SameLine();

        if (ImGui::Button("Create Cube"))
        {
            SelectedObject = &CreateCube(Scene);
        }

        ImGui::EndDisabled();

        ImGui::Separator();

        for (const auto& GameObjectPointer : Scene.GetGameObjects())
        {
            FGameObject* GameObject = GameObjectPointer.get();

            ImGui::PushID(GameObject);

            const bool bSelected = SelectedObject == GameObject;

            if (ImGui::Selectable(GameObject->GetName().c_str(), bSelected))
            {
                SelectedObject = GameObject;
            }

            ImGui::PopID();
        }

        ImGui::End();
    }

    void FEditor::DrawInspector(FScene& Scene, const EEngineMode Mode)
    {
        ImGui::SetNextWindowSize(ImVec2{340.0f, 500.0f},
                                 ImGuiCond_FirstUseEver);

        ImGui::Begin("Inspector");

        if (SelectedObject == nullptr)
        {
            ImGui::TextUnformatted("Select a GameObject.");

            ImGui::End();

            return;
        }

        std::array<char, 256> NameBuffer{};

        std::snprintf(NameBuffer.data(), NameBuffer.size(), "%s",
                      SelectedObject->GetName().c_str());

        const bool bIsPlaying = Mode == EEngineMode::Play;

        /*
         * Play 중에는 Inspector 내용을 볼 수 있지만
         * 편집하거나 삭제할 수 없다.
         */
        ImGui::BeginDisabled(bIsPlaying);

        if (ImGui::InputText("Name", NameBuffer.data(), NameBuffer.size()))
        {
            SelectedObject->SetName(NameBuffer.data());
        }

        bool bActive = SelectedObject->IsActive();

        if (ImGui::Checkbox("Active", &bActive))
        {
            SelectedObject->SetActive(bActive);
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Transform");

        FTransform& Transform = SelectedObject->GetTransform();

        DirectX::XMFLOAT3 Position = Transform.GetPosition();

        DirectX::XMFLOAT3 Rotation = Transform.GetRotationDegrees();

        DirectX::XMFLOAT3 Scale = Transform.GetScale();

        if (ImGui::DragFloat3("Position", &Position.x, 0.05f))
        {
            Transform.SetPosition(Position.x, Position.y, Position.z);
        }

        if (ImGui::DragFloat3("Rotation", &Rotation.x, 0.25f))
        {
            Transform.SetRotationDegrees(Rotation.x, Rotation.y, Rotation.z);
        }

        if (ImGui::DragFloat3("Scale", &Scale.x, 0.02f, 0.01f, 100.0f))
        {
            Transform.SetScale(Scale.x, Scale.y, Scale.z);
        }

        if (const auto* MeshRenderer = SelectedObject->GetComponent<
            FMeshRendererComponent>())
        {
            ImGui::Separator();

            ImGui::TextUnformatted("Mesh Renderer");

            ImGui::Text("Mesh: %s", MeshRenderer->GetMeshName().c_str());

            ImGui::Text("Shader: %s", MeshRenderer->GetShaderName().c_str());

            ImGui::Text("Texture: %s", MeshRenderer->GetTextureName().c_str());
        }

        ImGui::Separator();

        if (ImGui::Button("Delete GameObject"))
        {
            FGameObject* ObjectToDelete = SelectedObject;

            SelectedObject = nullptr;

            Scene.DestroyGameObject(*ObjectToDelete);
        }

        ImGui::EndDisabled();
        ImGui::End();
    }

    FGameObject& FEditor::CreateCube(FScene& Scene)
    {
        FGameObject& Cube = Scene.CreateGameObject(
            "Cube " + std::to_string(++NewObjectIndex));

        Cube.AddComponent<FMeshRendererComponent>(
            "Cube", "BasicShader", "CheckerTexture");

        return Cube;
    }
}
