#pragma once

#include "IRootSignature.h"
#include <vector>
#include <volk.h>
#include "IResourceDescriptor.h"
#include "defines.h"

class RootSignature : public IRootSignature {
public:
    uint32_t Convert(IResourceDescriptor::ResourceDescriptorType type, uint32_t offset = 0) const {
        uint32_t bind_point = offset;
        switch(type) {
            case IResourceDescriptor::ResourceDescriptorType::rdt_cbv:
                bind_point += vk_hlsl_bindings::cbv_offset;
                break;
            case IResourceDescriptor::ResourceDescriptorType::rdt_srv:
                bind_point += vk_hlsl_bindings::srv_offset;
                break;
            case IResourceDescriptor::ResourceDescriptorType::rdt_uav:
                bind_point += vk_hlsl_bindings::uav_offset;
                break;
            default:
                break;
        }

        return bind_point;
    }
public:
    uint32_t GetRSId() const override {
        return m_id;
    }
	void SetRSId(uint32_t id) override {
        m_id = id;
    }

    const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const {
        return m_layout_bindings;
    }

    VkDescriptorSetLayout GetLayout() {
        return m_desc_set_layout;
    }
    void Init(const std::vector<VkDescriptorSetLayoutBinding> layout_bindings, VkDescriptorSetLayout desc_set_layout);
private:
    uint32_t m_id{uint32_t(-1)};
    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
    VkDescriptorSetLayout m_desc_set_layout;
};
