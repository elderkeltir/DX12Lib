#pragma once

#include <directx/d3d12.h>
#include <cstdint>
#include <memory>
#include "simple_object_pool.h"
#include "GpuResource.h"

class MaterialManager {
public:
    struct Material {
        float metallic;
        float roughness;
        float padding1;
        float padding2;
    };
public:
    ~MaterialManager();
    uint32_t CreateMaterial(float metallic, float roughness);
    Material& GetMaterial(uint32_t id) { return m_materials[id]; }
    void LoadMaterials();
    void BindMaterials(ComPtr<ID3D12GraphicsCommandList6>& command_list);

private:
    static const uint32_t materials_num = 64;
    pro_game_containers::simple_object_pool<Material, materials_num> m_materials;
    std::unique_ptr<GpuResource> m_materials_res;
};