#include "GpuDataManager.h"

#include "GpuResource.h"

uint64_t GpuDataManager::AllocateVertexBuffer(uint32_t size) {
    return m_vertex_storage.allocate(size);
    m_dirty = true;
}

void GpuDataManager::DeallocateVertexBuffer(uint64_t start, uint32_t size) {
    m_vertex_storage.deallocate(start, size);
    m_dirty = true;
}

void GpuDataManager::Initialize(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
    m_vertex_buffer_res = std::make_unique<GpuResource>();
    m_vertex_buffer_res->CreateBuffer(HeapBuffer::BufferType::bt_default, vertex_storage_size, HeapBuffer::UseFlag::uf_srv, D3D12_RESOURCE_STATE_COPY_DEST, std::wstring(L"vertex_buffer"));
    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_UINT;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    desc.Buffer.NumElements = vertex_storage_size;

    m_vertex_buffer_res->Create_SRV(desc);
}

void GpuDataManager::UploadToGpu(ComPtr<ID3D12GraphicsCommandList6>& command_list)
{
    if (m_dirty) {
        m_vertex_buffer_res->LoadBuffer(command_list, vertex_storage_size, 1, (void*)m_vertex_storage.begin());
        

        m_dirty = false;
    }
}

