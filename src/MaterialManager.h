#pragma once

#include <directx/d3d12.h>
#include <cstdint>
#include <memory>
#include "simple_object_pool.h"
#include "IGpuResource.h"

class ICommandList;

class MaterialManager {
public:
    struct Material {
        float metallic;
        float roughness;
        float reflectivity;
        float padding2;
    };
public:
    ~MaterialManager();
    uint32_t CreateMaterial(float metallic, float roughness, float reflectivity);
    Material& GetMaterial(uint32_t id) { return m_materials[id]; }
    void LoadMaterials();
    void BindMaterials(ICommandList* command_list);

private:
    static const uint32_t materials_num = 64;
    pro_game_containers::simple_object_pool<Material, materials_num> m_materials;
    std::unique_ptr<IGpuResource> m_materials_res;
};