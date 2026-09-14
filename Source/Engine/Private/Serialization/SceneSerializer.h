/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.14
 * 파일 수정일 : 26.09.14
 *
 * 파일 개요 : Scene을 JSON 파일로 저장하고 불러오는 직렬화 시스템
 */

#pragma once

#include <filesystem>

namespace Engine
{
    class FScene;

    class FSceneSerializer final
    {
    public:
        FSceneSerializer() = delete;

        static bool Save(const FScene& Scene,
                         const std::filesystem::path& FilePath);

        static bool Load(FScene& Scene, const std::filesystem::path& FilePath);
    };
}
