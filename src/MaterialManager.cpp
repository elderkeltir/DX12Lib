#include "MaterialManager.h"
#include "DXAppImplementation.h"
#include "GfxCommandQueue.h"
#include "ResourceDescriptor.h"
#include "DXHelper.h"

extern DXAppImplementation *gD3DApp;

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
    m_materials_res = std::make_unique<GpuResource>();
    uint32_t cb_size = calc_cb_size(materials_num * sizeof(Material));
    m_materials_res->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"materials_buffer"));
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
    desc.SizeInBytes = cb_size;
    m_materials_res->Create_CBV(desc);
}

void MaterialManager::BindMaterials(CommandList& command_list) {
    ConstantBufferManager::SyncCpuDataToCB(command_list, m_materials_res.get(), m_materials.data(), (materials_num * sizeof(Material)), bi_materials_cb);
}