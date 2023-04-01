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
class IGpuResource;
class SkyBox;
class Plane;
class ICommandList;
class Sun;

class Level {
public:
    Level();
    ~Level();
    void Load(const std::wstring &name);
    void Update(float dt);
    void Render(ICommandList* command_list);
    void RenderWater(ICommandList* command_list);
    void RenderShadowMap(ICommandList* command_list);
    void BindLights(ICommandList* command_list);

    std::weak_ptr<FreeCamera> GetCamera() { return m_camera; }
    const std::filesystem::path& GetLevelsDir() const;
    const std::filesystem::path& GetEntitiesDir() const;

    const LevelLight& GetSunParams() const { return m_lights[0]; }
    IGpuResource& GetSunShadowMap();

private:
    void RenderEntity(ICommandList* command_list, LevelEntity & ent, bool &is_scene_constants_set);
    static const uint32_t entities_num = 256;
    std::wstring m_name;
    pro_game_containers::simple_object_pool<LevelEntity, entities_num> m_entites;
    pro_game_containers::simple_object_pool<LevelLight, LightsNum> m_lights;
    std::unique_ptr<SkyBox> m_skybox_ent;
    std::unique_ptr<Plane> m_terrain;
    std::unique_ptr<Plane> m_water;
    std::unique_ptr<IGpuResource> m_lights_res;
    std::shared_ptr<FreeCamera> m_camera;
    std::unique_ptr<Sun> m_sun;
    std::filesystem::path m_levels_dir;
    std::filesystem::path m_entities_dir; 
};