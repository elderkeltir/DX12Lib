#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include <DirectXMath.h>

class FreeCamera;
class RenderMesh;

class Level {
public:
    struct LevelEntity {
        std::wstring model_name;
        const RenderMesh* mesh; // TODO: make pool of meshes and assign one from there?
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 rot;
        DirectX::XMFLOAT3 scale;
        uint32_t id;

        void Load(const std::wstring &name);
    };
    struct LevelLight {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 dir;
        DirectX::XMFLOAT3 power;
        enum class LightType { lt_ambient, lt_direct, lt_point, lt_spot } type;
    };
public:
    Level();
    ~Level();
    void Load(const std::wstring &name);
    std::weak_ptr<FreeCamera> GetCamera() { return m_camera; }

    const std::filesystem::path& GetLevelsDir() const;
    const std::filesystem::path& GetEntitiesDir() const;

private:
    std::wstring m_name;
    std::vector<LevelEntity> m_entites;
    std::vector<LevelLight> m_lights;
    std::shared_ptr<FreeCamera> m_camera;
    std::filesystem::path m_levels_dir;
    std::filesystem::path m_entities_dir;
};