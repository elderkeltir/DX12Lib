#pragma once

#include "IRootSignature.h"
#include <vector>
#include <volk.h>
#include "vk_helper.h"

class RootSignature : public IRootSignature {
public:
class BindPointConverter {
public:
    uint32_t Convert(uint32_t root_id, uint32_t offset = 0) const {
        const std::vector<uint32_t>& table = m_data[root_id];
        assert(table.size() > offset);

        return table[offset];
    }
    std::vector<std::vector<uint32_t>> & GetData() {
        return m_data;
    }
private:
    std::vector<std::vector<uint32_t>> m_data;
};
    uint32_t GetRSId() const override {
        return m_id;
    }
	void SetRSId(uint32_t id) override {
        m_id = id;
    }

    void StageLayout(VkDescriptorSetLayoutCreateInfo layout);
    const VkDescriptorSetLayoutCreateInfo * GetLayoutInfo() const {
        return &m_layout_info;
    }

    VkDescriptorSetLayout GetLayout() {
        return m_desc_set_layout;
    }

    const BindPointConverter& GetConverter() const {
        return m_converter;
    }
    BindPointConverter& GetConverter() {
        return m_converter;
    }
private:
struct VkDescriptorSetLayoutCreateInfoWrapper : public VkDescriptorSetLayoutCreateInfo {
    ~VkDescriptorSetLayoutCreateInfoWrapper() {
        delete[] pBindings;
    }
};
    uint32_t m_id{-1};
    VkDescriptorSetLayoutCreateInfo m_layout_info;
    VkDescriptorSetLayout m_desc_set_layout;
    BindPointConverter m_converter;
};