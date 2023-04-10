#include "Fence.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "vk_helper.h"

extern VkBackend* gBackend;

void Fence::Initialize(uint32_t val) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &m_fence));

    VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &m_semaphore));
}