#include "RenderObject.h"
#include "IGpuResource.h"
#include "Frontend.h"
#include "GpuDataManager.h"
#include "ICommandList.h"

extern Frontend* gFrontend;

RenderObject::~RenderObject() {
    DeallocateVertexBuffer();
}

void RenderObject::LoadIndexDataOnGpu(ICommandList* command_list){
    if (m_dirty & db_index && m_mesh->GetIndicesNum()){
        m_IndexBuffer.reset(CreateGpuResource());
        m_IndexBuffer->CreateBuffer(HeapType::ht_default, (m_mesh->GetIndicesNum() * sizeof(uint16_t)), ResourceState::rs_resource_state_copy_dest, std::wstring(L"index_buffer").append(m_name));
        m_IndexBuffer->LoadBuffer(command_list, m_mesh->GetIndicesNum(), sizeof(uint16_t), m_mesh->GetIndicesData());
        command_list->ResourceBarrier(*m_IndexBuffer, ResourceState::rs_resource_state_index_buffer);
        m_IndexBuffer->Create_Index_View(ResourceFormat::rf_r16_uint, (m_mesh->GetIndicesNum() * sizeof(uint16_t)));
        m_dirty &= (~db_index);
    }
}

void RenderObject::AllocateVertexBuffer(uint32_t size) {
    if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gFrontend->GetGpuDataManager().lock()){
        m_vertex_buffer_start = gpu_res_mgr->AllocateVertexBuffer(size);
        m_vertex_buffer_size = size;
    }
}

void RenderObject::DeallocateVertexBuffer() {
    if (!gFrontend) {
        return;
    }
    
    if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gFrontend->GetGpuDataManager().lock()){
        gpu_res_mgr->DeallocateVertexBuffer(m_vertex_buffer_start, m_vertex_buffer_size);
        m_vertex_buffer_start = 0ull;
        m_vertex_buffer_size = 0ul;
    }
}