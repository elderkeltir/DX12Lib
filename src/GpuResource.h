#pragma once

#include <memory>
#include "HeapBuffer.h"

class ResourceDescriptor; // TODO: swap to interfaces later
class CommandList;
class HeapBuffer;

class GpuResource {
public:
    ~GpuResource();
    void CreateBuffer(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt);
    void CreateTexture(HeapType type, const ResourceDesc &res_desc, ResourceState initial_state, const ClearColor *clear_val, std::optional<std::wstring> dbg_name = std::nullopt);
    void SetBuffer(ComPtr<ID3D12Resource> res);
    void LoadBuffer(CommandList& command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData);
    void LoadBuffer(CommandList& command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData);
    
    void CreateRTV();
    void Create_DSV(const DSVdesc&desc);
    void Create_SRV(const SRVdesc &desc);
    void Create_UAV(const UAVdesc &desc);
    void Create_CBV(const CBVdesc &desc);
    void Create_Index_View(ResourceFormat format, uint32_t SizeInBytes);

    std::weak_ptr<HeapBuffer> GetBuffer() { return m_buffer; }
    std::weak_ptr<ResourceDescriptor> GetRTV() { return m_rtv; }
    std::weak_ptr<ResourceDescriptor> GetDSV() { return m_dsv; }
    std::weak_ptr<ResourceDescriptor> GetSRV() { return m_srv; }
    std::weak_ptr<ResourceDescriptor> GetUAV() { return m_uav; }
    std::weak_ptr<ResourceDescriptor> GetCBV() { return m_cbv; }
    std::weak_ptr<IndexVufferView> Get_Index_View() { return m_index_view; }
private:
    friend class CommandQueue;
    ResourceState GetState() const { return m_current_state; }
    void UpdateState(ResourceState new_state) {
        m_current_state = new_state;
    }
    void ResetViews();
    std::shared_ptr<HeapBuffer> m_buffer;
    std::shared_ptr<ResourceDescriptor> m_rtv;
    std::shared_ptr<ResourceDescriptor> m_dsv;
    std::shared_ptr<ResourceDescriptor> m_srv;
    std::shared_ptr<ResourceDescriptor> m_uav;
    std::shared_ptr<ResourceDescriptor> m_cbv;
    std::shared_ptr<IndexVufferView> m_index_view;

    ResourceState m_current_state{ ResourceState::rs_resource_state_common };
};