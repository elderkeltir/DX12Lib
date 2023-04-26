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
            assert(m_data.size() > root_id);
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

    const BindPointConverter& GetConverter() const {
        return m_converter;
    }
    BindPointConverter& GetConverter() {
        return m_converter;
    }
    void Init(const std::vector<VkDescriptorSetLayoutBinding> layout_bindings, VkDescriptorSetLayout desc_set_layout);
private:
    uint32_t m_id{uint32_t(-1)};
    std::vector<VkDescriptorSetLayoutBinding> m_layout_bindings;
    VkDescriptorSetLayout m_desc_set_layout;
    BindPointConverter m_converter;
};
