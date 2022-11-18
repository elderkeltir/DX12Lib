#include "MaterialManager.h"
#include "DXAppImplementation.h"
#include "GfxCommandQueue.h"
#include "ResourceDescriptor.h"
#include "DXHelper.h"

extern DXAppImplementation *gD3DApp;

MaterialManager::~MaterialManager() = default;

uint32_t MaterialManager::CreateMaterial(float metallic, float roughness) {
    for (uint32_t i = 0; i < m_materials.size(); i++){
        if (cmpf(metallic, m_materials[i].metallic) && cmpf(roughness, m_materials[i].roughness)){
            return i;
        }
    }

    Material m{ metallic, roughness };
    return m_materials.push_back(m);
}

void MaterialManager::LoadMaterials() {
    m_materials_res = std::make_unique<GpuResource>();
    uint32_t cb_size = (materials_num * sizeof(Material) + 255) & ~255;
    m_materials_res->CreateBuffer(HeapBuffer::BufferType::bt_default, cb_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, std::wstring(L"materials_buffer"));
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
    desc.SizeInBytes = cb_size;
    m_materials_res->Create_CBV(desc);
}

void MaterialManager::SyncGpuData(ComPtr<ID3D12GraphicsCommandList6>& command_list) {
    if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()){
        queue->ResourceBarrier(*m_materials_res.get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
    }
    if (std::shared_ptr<HeapBuffer> buff = m_materials_res->GetBuffer().lock()){
        uint32_t cb_size = (materials_num * sizeof(Material) + 255) & ~255;
        buff->Load(command_list, 1, cb_size, m_materials.data());
    }
    if (std::shared_ptr<GfxCommandQueue> queue = gD3DApp->GetGfxQueue().lock()){
        queue->ResourceBarrier(*m_materials_res.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    if (std::shared_ptr<ResourceDescriptor> srv = m_materials_res->GetCBV().lock()){
        command_list->SetGraphicsRootDescriptorTable(6, srv->GetGPUhandle());
    }
}