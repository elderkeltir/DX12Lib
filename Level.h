#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <DirectXMath.h>
#include <vector>
#include "simple_object_pool.h"
#include "LevelEntity.h"

class FreeCamera;
class RenderModel;

class Level {
public:
    struct LevelLight : protected pro_game_containers::object {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 dir;
        DirectX::XMFLOAT3 power;
        uint32_t id;
        enum class LightType { lt_ambient, lt_direct, lt_point, lt_spot } type;
    };
public:
    Level();
    ~Level();
    void Load(const std::wstring &name);
    void Update(float dt);
    void Render();

    std::weak_ptr<FreeCamera> GetCamera() { return m_camera; }
    const std::filesystem::path& GetLevelsDir() const;
    const std::filesystem::path& GetEntitiesDir() const;
    LevelEntity& GetEntityById(uint32_t id) { return m_entites[id]; }
    uint32_t GetEntityCount() const { return m_entites.size(); }

private:
    std::wstring m_name;
    pro_game_containers::simple_object_pool<LevelEntity, 256> m_entites;
    pro_game_containers::simple_object_pool<LevelLight, 16> m_lights;
    std::shared_ptr<FreeCamera> m_camera;
    std::filesystem::path m_levels_dir;
    std::filesystem::path m_entities_dir;
};