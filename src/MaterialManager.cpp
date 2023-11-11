#include "MaterialManager.h"
#include "ConstantBufferManager.h"
#include "RenderHelper.h"
#include "defines.h"
#include "Frontend.h"

extern Frontend* gFrontend;

MaterialManager::~MaterialManager() = default;

uint32_t MaterialManager::CreateMaterial(float metallic, float roughness, float reflectivity) {
    for (uint32_t i = 0; i < m_materials.size(); i++){
        if (cmpf(metallic, m_materials[i].metallic) && cmpf(roughness, m_materials[i].roughness) && cmpf(reflectivity, m_materials[i].reflectivity)){
            return i;
        }
    }

    Material m{ metallic, roughness, reflectivity };
    return m_materials.push_back(m);
}

void MaterialManager::LoadMaterials() {
    m_materials_res.reset(CreateGpuResource(gFrontend->GetBackendType()));
    uint32_t cb_size = calc_cb_size(materials_num * sizeof(Material));
    HeapType h_type = HeapType(HeapType::ht_default | HeapType::ht_buff_uniform_buffer);
    m_materials_res->CreateBuffer(h_type, cb_size, ResourceState::rs_resource_state_vertex_and_constant_buffer, std::wstring(L"materials_buffer"));
    CBVdesc desc;
    desc.size_in_bytes = cb_size;
    m_materials_res->Create_CBV(desc);
}

void MaterialManager::BindMaterials(ICommandList* command_list) {
    ConstantBufferManager::SyncCpuDataToCB(command_list, m_materials_res.get(), m_materials.data(), (materials_num * sizeof(Material)), bi_materials_cb);
}