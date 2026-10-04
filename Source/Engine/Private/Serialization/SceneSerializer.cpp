/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Scene의 JSON 문자열 직렬화, 역직렬화와 파일 입출력 구현
 */

#include "Serialization/SceneSerializer.h"

#include <Engine/Core/Log.h>
#include <Engine/Scene/GameObject.h>
#include <Engine/Scene/MeshRendererComponent.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Transform.h>

#include <nlohmann/json.hpp>

#include <array>
#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace
{
    struct FSerializedGameObject
    {
        std::string Name;
        bool bActive = true;
        bool bMeshVisible = true;

        std::array<float, 3> Position{};
        std::array<float, 3> Rotation{};
        std::array<float, 3> Scale{1.0f, 1.0f, 1.0f};

        bool bHasMeshRenderer = false;
        std::string MeshName;
        std::string ShaderName;
        std::string TextureName;
    };

    bool ReadObjects(const nlohmann::json& Root,
                     std::vector<FSerializedGameObject>& OutObjects)
    {
        if (Root.at("version").get<int>() != 1)
        {
            Engine::FLog::Error("Unsupported scene version.");
            return false;
        }

        const auto& ObjectsJson = Root.at("gameObjects");

        if (!ObjectsJson.is_array())
        {
            Engine::FLog::Error("The scene object list is invalid.");
            return false;
        }

        OutObjects.clear();
        OutObjects.reserve(ObjectsJson.size());

        for (const auto& ObjectJson : ObjectsJson)
        {
            FSerializedGameObject Data;

            Data.Name = ObjectJson.at("name").get<std::string>();
            Data.bActive = ObjectJson.value("active", true);

            if (Data.Name.empty())
            {
                Engine::FLog::Error("A scene object has an empty name.");
                return false;
            }

            const auto& TransformJson = ObjectJson.at("transform");

            Data.Position = TransformJson.at("position").get<std::array<
                float, 3>>();

            Data.Rotation = TransformJson.at("rotation").get<std::array<
                float, 3>>();

            Data.Scale = TransformJson.at("scale").get<std::array<float, 3>>();

            if (ObjectJson.contains("meshRenderer"))
            {
                const auto& RendererJson = ObjectJson.at("meshRenderer");

                Data.bHasMeshRenderer = true;
                Data.bMeshVisible = RendererJson.value("visible", true);

                Data.MeshName = RendererJson.at("mesh").get<std::string>();

                Data.ShaderName = RendererJson.at("shader").get<std::string>();

                Data.TextureName = RendererJson.at("texture").get<
                    std::string>();
            }

            OutObjects.emplace_back(std::move(Data));
        }

        return true;
    }

    bool ApplyObjects(Engine::FScene& Scene,
                      const std::vector<FSerializedGameObject>& Objects)
    {
        Scene.Clear();

        for (const FSerializedGameObject& Data : Objects)
        {
            Engine::FGameObject& GameObject = Scene.CreateGameObject(Data.Name);

            GameObject.SetActive(Data.bActive);

            GameObject.GetTransform().SetPosition(
                Data.Position[0], Data.Position[1], Data.Position[2]);

            GameObject.GetTransform().SetRotationDegrees(
                Data.Rotation[0], Data.Rotation[1], Data.Rotation[2]);

            GameObject.GetTransform().SetScale(Data.Scale[0], Data.Scale[1],
                                               Data.Scale[2]);

            if (Data.bHasMeshRenderer)
            {
                auto* MeshRenderer = GameObject.AddComponent<
                    Engine::FMeshRendererComponent>(
                    Data.MeshName, Data.ShaderName, Data.TextureName);

                if (MeshRenderer == nullptr)
                {
                    Engine::FLog::Error("Failed to restore a mesh renderer.");

                    return false;
                }

                MeshRenderer->SetVisible(Data.bMeshVisible);
            }
        }

        return true;
    }
}

namespace Engine
{
    std::string FSceneSerializer::SerializeToString(const FScene& Scene)
    {
        try
        {
            nlohmann::json Root;

            Root["version"] = 1;
            Root["gameObjects"] = nlohmann::json::array();

            for (const auto& GameObjectPointer : Scene.GetGameObjects())
            {
                const FGameObject& GameObject = *GameObjectPointer;

                const FTransform& Transform = GameObject.GetTransform();

                const auto& Position = Transform.GetPosition();
                const auto& Rotation = Transform.GetRotationDegrees();
                const auto& Scale = Transform.GetScale();

                nlohmann::json ObjectJson{
                    {"name", GameObject.GetName()},
                    {"active", GameObject.IsActive()},
                    {
                        "transform",
                        {
                            {"position", {Position.x, Position.y, Position.z}},
                            {"rotation", {Rotation.x, Rotation.y, Rotation.z}},
                            {"scale", {Scale.x, Scale.y, Scale.z}}
                        }
                    }
                };

                if (const auto* MeshRenderer = GameObject.GetComponent<
                    FMeshRendererComponent>())
                {
                    ObjectJson["meshRenderer"] = {
                        {"mesh", MeshRenderer->GetMeshName()},
                        {"shader", MeshRenderer->GetShaderName()},
                        {"texture", MeshRenderer->GetTextureName()},
                        {"visible", MeshRenderer->IsVisible()}
                    };
                }

                Root["gameObjects"].push_back(std::move(ObjectJson));
            }

            return Root.dump(4);
        }
        catch (const std::exception&)
        {
            FLog::Error("Failed to serialize the scene.");
            return {};
        }
    }

    bool FSceneSerializer::DeserializeFromString(
        FScene& Scene, const std::string_view JsonText)
    {
        try
        {
            if (JsonText.empty())
            {
                FLog::Error("The scene JSON text is empty.");
                return false;
            }

            const nlohmann::json Root = nlohmann::json::parse(
                JsonText.begin(), JsonText.end());

            std::vector<FSerializedGameObject> Objects;

            /*
             * Scene을 지우기 전에 JSON 전체를 먼저 검증한다.
             */
            if (!ReadObjects(Root, Objects))
            {
                return false;
            }

            return ApplyObjects(Scene, Objects);
        }
        catch (const std::exception&)
        {
            FLog::Error("The scene JSON is invalid.");
            return false;
        }
    }

    bool FSceneSerializer::Save(const FScene& Scene,
                                const std::filesystem::path& FilePath)
    {
        const std::string JsonText = SerializeToString(Scene);

        if (JsonText.empty())
        {
            return false;
        }

        try
        {
            const auto ParentPath = FilePath.parent_path();

            if (!ParentPath.empty())
            {
                std::filesystem::create_directories(ParentPath);
            }

            std::ofstream OutputFile{FilePath};

            if (!OutputFile.is_open())
            {
                FLog::Error("Failed to open the scene file for writing.");

                return false;
            }

            OutputFile << JsonText;

            if (!OutputFile.good())
            {
                FLog::Error("Failed to write the scene file.");

                return false;
            }

            FLog::Info("Scene saved.");
            return true;
        }
        catch (const std::exception&)
        {
            FLog::Error("An exception occurred while saving the scene.");

            return false;
        }
    }

    bool FSceneSerializer::Load(FScene& Scene,
                                const std::filesystem::path& FilePath)
    {
        try
        {
            std::ifstream InputFile{FilePath};

            if (!InputFile.is_open())
            {
                FLog::Error("Failed to open the scene file.");
                return false;
            }

            const std::string JsonText{
                std::istreambuf_iterator<char>{InputFile},
                std::istreambuf_iterator<char>{}
            };

            if (!DeserializeFromString(Scene, JsonText))
            {
                return false;
            }

            FLog::Info("Scene loaded.");
            return true;
        }
        catch (const std::exception&)
        {
            FLog::Error("An exception occurred while loading the scene.");

            return false;
        }
    }
}
