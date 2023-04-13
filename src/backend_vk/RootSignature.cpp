#include "RootSignature.h"
#include "vk_helper.h"
#include "VkBackend.h"
#include "VkDevice.h"

extern VkBackend* gBackend;

void RootSignature::StageLayout(VkDescriptorSetLayoutCreateInfo layout) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    m_layout_info.flags = layout.flags;
    m_layout_info.sType = layout.sType;
    m_layout_info.bindingCount = layout.bindingCount;

    m_layout_info.pBindings = new VkDescriptorSetLayoutBinding[m_layout_info.bindingCount];
    memcpy((VkDescriptorSetLayoutBinding*)m_layout_info.pBindings, layout.pBindings, sizeof(VkDescriptorSetLayoutBinding) * m_layout_info.bindingCount);

	VK_CHECK(vkCreateDescriptorSetLayout(device, &layout, nullptr, &m_desc_set_layout));
}