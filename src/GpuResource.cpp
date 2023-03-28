#include "GpuResource.h"
#include "ResourceDescriptor.h"
#include "CommandQueue.h"

GpuResource::~GpuResource(){

}

void GpuResource::CreateBuffer(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name){
    if (m_buffer){
        ResetViews();
    }
    m_buffer = std::make_shared<HeapBuffer>();
    m_buffer->Create(type, bufferSize, initial_state, dbg_name);
    m_current_state = initial_state;
}

void GpuResource::CreateTexture(HeapType type, const ResourceDesc &res_desc, ResourceState initial_state, const ClearColor *clear_val, std::optional<std::wstring> dbg_name){
    if (m_buffer){
        ResetViews();
    }
    m_buffer = std::make_shared<HeapBuffer>();
    m_buffer->CreateTexture(type, res_desc, initial_state, clear_val, dbg_name);
    m_current_state = initial_state;
}

void GpuResource::SetBuffer(ComPtr<ID3D12Resource> res){
    if (m_buffer){
        ResetViews();
    }
    m_buffer = std::make_shared<HeapBuffer>();
    m_buffer->Set(res);
}

void GpuResource::LoadBuffer(CommandList& command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData){
    m_buffer->Load(command_list, numElements, elementSize, bufferData);
}

void GpuResource::LoadBuffer(CommandList& command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData){
    m_buffer->Load(command_list, firstSubresource, numSubresources, subresourceData);
}

void GpuResource::CreateRTV(){
    m_rtv = std::make_shared<ResourceDescriptor>();
    m_rtv->Create_RTV(m_buffer);
}

void GpuResource::Create_DSV(const DSVdesc &desc){
    m_dsv = std::make_shared<ResourceDescriptor>();
    m_dsv->Create_DSV(m_buffer, desc);
}

void GpuResource::Create_SRV(const SRVdesc &desc){
    m_srv = std::make_shared<ResourceDescriptor>();
    m_srv->Create_SRV(m_buffer, desc);
}

void GpuResource::Create_UAV(const UAVdesc &desc){
    m_uav = std::make_shared<ResourceDescriptor>();
    m_uav->Create_UAV(m_buffer, desc);
}

void GpuResource::Create_CBV(const CBVdesc &desc) {
    m_cbv = std::make_shared<ResourceDescriptor>();
    m_cbv->Create_CBV(m_buffer, desc);
}

void GpuResource::Create_Index_View(ResourceFormat format, uint32_t SizeInBytes){
    m_index_view = std::make_shared<IndexVufferView>();
    m_index_view->buffer_location = m_buffer->GetResource()->GetGPUVirtualAddress();
    m_index_view->format = format;
    m_index_view->size_in_bytes = SizeInBytes;
}

void GpuResource::ResetViews(){
    m_rtv.reset();
    m_dsv.reset();
    m_srv.reset();
    m_index_view.reset();
}