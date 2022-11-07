#include "GpuResource.h"
#include "ResourceDescriptor.h"

GpuResource::~GpuResource(){

}

void GpuResource::CreateBuffer(HeapBuffer::BufferType type, uint32_t bufferSize, HeapBuffer::UseFlag flag, D3D12_RESOURCE_STATES initial_state, std::optional<std::wstring> dbg_name){
    if (m_buffer){
        ResetViews();
    }
    m_buffer = std::make_shared<HeapBuffer>();
    m_buffer->Create(type, bufferSize, flag, initial_state, dbg_name);
}

void GpuResource::CreateTexture(HeapBuffer::BufferType type, const CD3DX12_RESOURCE_DESC &res_desc, D3D12_RESOURCE_STATES initial_state, const D3D12_CLEAR_VALUE *clear_val, std::optional<std::wstring> dbg_name){
    if (m_buffer){
        ResetViews();
    }
    m_buffer = std::make_shared<HeapBuffer>();
    m_buffer->CreateTexture(type, res_desc, initial_state, clear_val, dbg_name);
}

void GpuResource::SetBuffer(ComPtr<ID3D12Resource> res){
    if (m_buffer){
        ResetViews();
    }
    m_buffer = std::make_shared<HeapBuffer>();
    m_buffer->Set(res);
}

void GpuResource::LoadBuffer(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t numElements, uint32_t elementSize, const void* bufferData){
    m_buffer->Load(commandList, numElements, elementSize, bufferData);
}

void GpuResource::LoadBuffer(ComPtr<ID3D12GraphicsCommandList6> &commandList, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData){
    m_buffer->Load(commandList, firstSubresource, numSubresources, subresourceData);
}

void GpuResource::CreateRTV(){
    m_rtv = std::make_shared<ResourceDescriptor>();
    m_rtv->Create_RTV(m_buffer);
}

void GpuResource::Create_DSV(const D3D12_DEPTH_STENCIL_VIEW_DESC &desc){
    m_dsv = std::make_shared<ResourceDescriptor>();
    m_dsv->Create_DSV(m_buffer, desc);
}

void GpuResource::Create_SRV(const D3D12_SHADER_RESOURCE_VIEW_DESC &desc, bool gpu_visible){
    m_srv = std::make_shared<ResourceDescriptor>();
    m_srv->Create_SRV(m_buffer, desc, gpu_visible);
}

void GpuResource::Create_UAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC &desc, bool gpu_visible){
    m_uav = std::make_shared<ResourceDescriptor>();
    m_uav->Create_UAV(m_buffer, desc, gpu_visible);
}

void GpuResource::Create_Vertex_View(uint32_t sizSizeInBytese, uint32_t StrideInBytes){
    m_vertex_view = std::make_shared<D3D12_VERTEX_BUFFER_VIEW>();
    m_vertex_view->BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
    m_vertex_view->SizeInBytes = sizSizeInBytese;
    m_vertex_view->StrideInBytes = StrideInBytes;
}

void GpuResource::Create_Index_View(DXGI_FORMAT format, uint32_t SizeInBytes){
    m_index_view = std::make_shared<D3D12_INDEX_BUFFER_VIEW>();
    m_index_view->BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
    m_index_view->Format = format;
    m_index_view->SizeInBytes = SizeInBytes;
}

void GpuResource::ResetViews(){
    m_rtv.reset();
    m_dsv.reset();
    m_srv.reset();
    m_vertex_view.reset();
    m_index_view.reset();
}