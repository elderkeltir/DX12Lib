#include "RenderObject.h"
#include "GpuResource.h"
#include "DXAppImplementation.h"
#include "GpuDataManager.h"

extern DXAppImplementation *gD3DApp;

RenderObject::~RenderObject() {
    DeallocateVertexBuffer();
}

void RenderObject::LoadVertexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList, const void* data, uint32_t size_of_vertex, uint32_t vertex_count){
    if (m_dirty & db_vertex){
        m_VertexBuffer = std::make_unique<GpuResource>();
        m_VertexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_default, (vertex_count * size_of_vertex), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST, std::wstring(L"vertex_buffer").append(m_name));
        m_VertexBuffer->LoadBuffer(commandList, vertex_count, size_of_vertex, data);
        if (std::shared_ptr<HeapBuffer> buff = m_VertexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        }
        m_VertexBuffer->Create_Vertex_View((vertex_count * size_of_vertex), size_of_vertex);
        m_dirty &= (~db_vertex);
    }
}

void RenderObject::LoadIndexDataOnGpu(ComPtr<ID3D12GraphicsCommandList6> &commandList){
    if (m_dirty & db_index){
        m_IndexBuffer = std::make_unique<GpuResource>();
        m_IndexBuffer->CreateBuffer(HeapBuffer::BufferType::bt_default, ((uint32_t)m_indices.size() * sizeof(uint16_t)), HeapBuffer::UseFlag::uf_none, D3D12_RESOURCE_STATE_COPY_DEST, std::wstring(L"index_buffer").append(m_name));
        m_IndexBuffer->LoadBuffer(commandList, (uint32_t)m_indices.size(), sizeof(uint16_t), m_indices.data());
        if (std::shared_ptr<HeapBuffer> buff = m_IndexBuffer->GetBuffer().lock()){
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        }
        m_IndexBuffer->Create_Index_View(DXGI_FORMAT_R16_UINT, ((uint32_t)m_indices.size() * sizeof(uint16_t)));
        m_dirty &= (~db_index);
    }
}

void RenderObject::Loadtexture(ComPtr<ID3D12GraphicsCommandList6> & commandList, GpuResource* res, TextureData* tex_data, const CD3DX12_RESOURCE_DESC &tex_desc, const D3D12_SRV_DIMENSION &srv_dim, uint32_t idx) const {
    D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
    res->CreateTexture(HeapBuffer::BufferType::bt_default, tex_desc, initial_state, nullptr, std::wstring(m_name).append(L"model_srv_").append(std::to_wstring(idx)).c_str());

    const uint32_t image_count = (uint32_t)tex_data->scratch_image.GetImageCount();
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(image_count);
    const DirectX::Image* pImages = tex_data->scratch_image.GetImages();
    for (uint32_t i = 0; i < (uint32_t)image_count; ++i) {
        auto& subresource = subresources[i];
        subresource.RowPitch = pImages[i].rowPitch;
        subresource.SlicePitch = pImages[i].slicePitch;
        subresource.pData = pImages[i].pixels;
    }
    res->LoadBuffer(commandList, 0, (uint32_t)subresources.size(), subresources.data());
    if (std::shared_ptr<HeapBuffer> buff = res->GetBuffer().lock()){
        commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buff->GetResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Format = tex_data->meta_data.format;
    srv_desc.ViewDimension = srv_dim;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = (UINT)tex_data->meta_data.mipLevels;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    res->Create_SRV(srv_desc, true);
}

void RenderObject::AllocateVertexBuffer(uint32_t size) {
    if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gD3DApp->GetGpuDatamanager().lock()){
        m_vertex_buffer_start = gpu_res_mgr->AllocateVertexBuffer(size);
        m_vertex_buffer_size = size;
    }
}

void RenderObject::DeallocateVertexBuffer() {
    if (!gD3DApp) {
        return;
    }
    
    if (std::shared_ptr<GpuDataManager> gpu_res_mgr = gD3DApp->GetGpuDatamanager().lock()){
        gpu_res_mgr->DeallocateVertexBuffer(m_vertex_buffer_start, m_vertex_buffer_size);
        m_vertex_buffer_start = 0ull;
        m_vertex_buffer_size = 0ul;
    }
}