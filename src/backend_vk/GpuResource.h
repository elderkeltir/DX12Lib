#pragma once

#include "IGpuResource.h"

class GpuResource : public IGpuResource {
public:
    ~GpuResource();
    void CreateBuffer(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name = std::nullopt) override;
    void CreateTexture(HeapType type, const ResourceDesc &res_desc, ResourceState initial_state, const ClearColor *clear_val, std::optional<std::wstring> dbg_name = std::nullopt) override;
    
    void LoadBuffer(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) override;
    void LoadBuffer(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) override;
    
    void CreateRTV() override;
    void Create_DSV(const DSVdesc&desc) override;
    void Create_SRV(const SRVdesc &desc) override;
    void Create_UAV(const UAVdesc &desc) override;
    void Create_CBV(const CBVdesc &desc) override;
    void Create_Index_View(ResourceFormat format, uint32_t SizeInBytes) override;

    std::weak_ptr<IHeapBuffer> GetBuffer() override { return m_buffer; }
    std::weak_ptr<IResourceDescriptor> GetRTV() override { return m_rtv; }
    std::weak_ptr<IResourceDescriptor> GetDSV() override { return m_dsv; }
    std::weak_ptr<IResourceDescriptor> GetSRV() override { return m_srv; }
    std::weak_ptr<IResourceDescriptor> GetUAV() override { return m_uav; }
    std::weak_ptr<IResourceDescriptor> GetCBV() override { return m_cbv; }
    std::weak_ptr<IndexVufferView> Get_Index_View() override { return m_index_view; }

    ResourceState GetState() const override { return m_current_state; }
    void UpdateState(ResourceState new_state) override {
        m_current_state = new_state;
    }

    void SetFrameBuffer(VkFramebuffer fb) {
        m_fb = fb;
    }
    VkFramebuffer GetFrameBuffer() const {
        return m_fb;
    }

    void SetRenderPass(VkRenderPass rp) {
        m_rp = rp;
    }
    VkRenderPass GetRenderPass() const {
        return m_rp;
    }

    const ResourceDesc& GetResourceDesc() const {
        return m_res_desc;
    }

private:
    void ResetViews();
    std::shared_ptr<IHeapBuffer> m_buffer;
    std::shared_ptr<IResourceDescriptor> m_rtv;
    std::shared_ptr<IResourceDescriptor> m_dsv;
    std::shared_ptr<IResourceDescriptor> m_srv;
    std::shared_ptr<IResourceDescriptor> m_uav;
    std::shared_ptr<IResourceDescriptor> m_cbv;
    std::shared_ptr<IndexVufferView> m_index_view;
    VkFramebuffer m_fb;
    VkRenderPass m_rp;
    ResourceDesc m_res_desc;

    ResourceState m_current_state{ ResourceState::rs_resource_state_common };
};