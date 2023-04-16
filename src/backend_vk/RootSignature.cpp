#include "RootSignature.h"
#include "vk_helper.h"
#include "VkBackend.h"
#include "VkDevice.h"

extern VkBackend* gBackend;

void RootSignature::Init(const std::vector<VkDescriptorSetLayoutBinding> layout_bindings, VkDescriptorSetLayout desc_set_layout) {
    m_layout_bindings = layout_bindings;
    m_desc_set_layout = desc_set_layout;
}
