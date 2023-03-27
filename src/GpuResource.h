#pragma once

#include <memory>
#include "IGpuResource.h"
#include "HeapBuffer.h"
class ResourceDescriptor;
class CommandList;

class GpuResource : public IGpuResource {
public:
    ~GpuResource();
    void CreateBuffer(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void CreateTexture(HeapType type, const ResourceDesc &res_desc, ResourceState initial_state, const ClearColor *clear_val, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void SetBuffer(ComPtr<ID3D12Resource> res) override;
    void LoadBuffer(CommandList& command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) override;
    void LoadBuffer(CommandList& command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) override;
    
    void CreateRTV() override;
    void Create_DSV(const DSVdesc&desc) override;
    void Create_SRV(const SRVdesc &desc) override;
    void Create_UAV(const UAVdesc &desc) override;
    void Create_CBV(const CBVdesc &desc) override;
    void Create_Index_View(ResourceFormat format, uint32_t SizeInBytes) override;

    std::weak_ptr<HeapBuffer> GetBuffer() override { return m_buffer; }
    std::weak_ptr<ResourceDescriptor> GetRTV() override { return m_rtv; }
    std::weak_ptr<ResourceDescriptor> GetDSV() override { return m_dsv; }
    std::weak_ptr<ResourceDescriptor> GetSRV() override { return m_srv; }
    std::weak_ptr<ResourceDescriptor> GetUAV() override { return m_uav; }
    std::weak_ptr<ResourceDescriptor> GetCBV() override { return m_cbv; }
    std::weak_ptr<IndexVufferView> Get_Index_View() override { return m_index_view; }
private:
    friend class GfxCommandQueue;
    ResourceState GetState() const override { return m_current_state; }
    void UpdateState(ResourceState new_state) override {
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