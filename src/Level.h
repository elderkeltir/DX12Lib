#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <DirectXMath.h>
#include <vector>
#include "simple_object_pool.h"
#include "LevelEntity.h"
#include "LevelLight.h"

class FreeCamera;
class RenderModel;
class GpuResource;
class SkyBox;
class Plane;

class Level {
public:
    Level();
    ~Level();
    void Load(const std::wstring &name);
    void Update(float dt);
    void Render(ComPtr<ID3D12GraphicsCommandList6>& command_list);
    void RenderWater(ComPtr<ID3D12GraphicsCommandList6>& command_list);
    void BindLights(ComPtr<ID3D12GraphicsCommandList6>& command_list);

    std::weak_ptr<FreeCamera> GetCamera() { return m_camera; }
    const std::filesystem::path& GetLevelsDir() const;
    const std::filesystem::path& GetEntitiesDir() const;

private:
    void RenderEntity(ComPtr<ID3D12GraphicsCommandList6>& command_list, LevelEntity & ent, bool &is_scene_constants_set);
    static const uint32_t entities_num = 256;
    std::wstring m_name;
    pro_game_containers::simple_object_pool<LevelEntity, entities_num> m_entites;
    pro_game_containers::simple_object_pool<LevelLight, LightsNum> m_lights;
    std::unique_ptr<SkyBox> m_skybox_ent;
    std::unique_ptr<Plane> m_terrain;
    std::unique_ptr<Plane> m_water;
    std::unique_ptr<GpuResource> m_lights_res;
    std::shared_ptr<FreeCamera> m_camera;
    std::filesystem::path m_levels_dir;
    std::filesystem::path m_entities_dir; 
};