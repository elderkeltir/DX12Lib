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
class GpuResource;

class Level {
public:
    struct LevelLight {
        LevelLight() = default;
        enum class LightType { lt_ambient, lt_direct, lt_point, lt_spot } type;
        LevelLight(const DirectX::XMFLOAT3 &dir_, const DirectX::XMFLOAT3 &color_, LevelLight::LightType light_type) :
        dir(dir_),
        color(color_),
        type(light_type)
        {}
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 dir;
        DirectX::XMFLOAT3 color;
        uint32_t id;
    };
public:
    Level();
    ~Level();
    void Load(const std::wstring &name);
    void Update(float dt);
    void Render(ComPtr<ID3D12GraphicsCommandList6>& command_list);

    std::weak_ptr<FreeCamera> GetCamera() { return m_camera; }
    const std::filesystem::path& GetLevelsDir() const;
    const std::filesystem::path& GetEntitiesDir() const;
    // LevelEntity& GetEntityById(uint32_t id) { return m_entites[id]; }
    // uint32_t GetEntityCount() const { return m_entites.size(); }

private:
    static const uint32_t lights_num = 16;
    std::wstring m_name;
    pro_game_containers::simple_object_pool<LevelEntity, 256> m_entites;
    pro_game_containers::simple_object_pool<LevelLight, lights_num> m_lights;
    std::unique_ptr<GpuResource> m_lights_res;
    std::shared_ptr<FreeCamera> m_camera;
    std::filesystem::path m_levels_dir;
    std::filesystem::path m_entities_dir;
};