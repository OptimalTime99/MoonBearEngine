/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.10.04
 *
 * 파일 개요 : Scene의 파일 및 메모리 JSON 직렬화 시스템
 */

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace Engine
{
    class FScene;

    class FSceneSerializer final
    {
    public:
        FSceneSerializer() = delete;

        [[nodiscard]] static std::string SerializeToString(const FScene& Scene);

        static bool DeserializeFromString(FScene& Scene,
                                          std::string_view JsonText);

        static bool Save(const FScene& Scene,
                         const std::filesystem::path& FilePath);

        static bool Load(FScene& Scene, const std::filesystem::path& FilePath);
    };
}
