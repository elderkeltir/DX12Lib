#include "GpuDataManager.h"
#include "GfxCommandQueue.h"
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
    m_vertex_buffer_res->CreateBuffer(HeapBuffer::BufferType::bt_default, vertex_storage_size, HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, std::wstring(L"vertex_buffer"));
    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = DXGI_FORMAT_R8_UINT;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    desc.Buffer.NumElements = vertex_storage_size;
    desc.Buffer.StructureByteStride = 0;

    m_vertex_buffer_res->Create_SRV(desc);
}

void GpuDataManager::UploadToGpu(CommandList& command_list)
{
    if (m_dirty) {
        command_list.GetQueue()->ResourceBarrier(*m_vertex_buffer_res, D3D12_RESOURCE_STATE_COPY_DEST);
        m_vertex_buffer_res->LoadBuffer(command_list, vertex_storage_size, 1, (void*)m_vertex_storage.begin());
        command_list.GetQueue()->ResourceBarrier(*m_vertex_buffer_res, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        m_dirty = false;
    }
}

