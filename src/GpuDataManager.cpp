#include "GpuDataManager.h"
#include "CommandQueue.h"
#include "GpuResource.h"

uint64_t GpuDataManager::AllocateVertexBuffer(uint32_t size) {
    m_dirty = true;
    return m_vertex_storage.allocate(size);
}

void GpuDataManager::DeallocateVertexBuffer(uint64_t start, uint32_t size) {
    m_vertex_storage.deallocate(start, size);
    m_dirty = true;
}

void GpuDataManager::Initialize()
{
    m_vertex_buffer_res = std::make_unique<GpuResource>();
    m_vertex_buffer_res->CreateBuffer(HeapType::ht_default, vertex_storage_size, ResourceState::rs_resource_state_all_shader_resource, std::wstring(L"vertex_buffer"));
    SRVdesc desc;
    desc.format = ResourceFormat::rf_r8_uint;
    desc.dimension = SRVdesc::SRVdimensionType::srv_dt_buffer;
    desc.buffer.first_element = 0;
    desc.buffer.num_elements = vertex_storage_size;
    desc.buffer.structure_byte_stride = 0;

    m_vertex_buffer_res->Create_SRV(desc);
}

void GpuDataManager::UploadToGpu(CommandList& command_list)
{
    if (m_dirty) {
        command_list.GetQueue()->ResourceBarrier(*m_vertex_buffer_res, ResourceState::rs_resource_state_copy_dest);
        m_vertex_buffer_res->LoadBuffer(command_list, vertex_storage_size, 1, (void*)m_vertex_storage.begin());
        command_list.GetQueue()->ResourceBarrier(*m_vertex_buffer_res, ResourceState::rs_resource_state_all_shader_resource);

        m_dirty = false;
    }
}

