#include "CommandQueue.h"
#include "CommandList.h"
#include "Fence.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "DynamicGpuHeap.h"

extern VkBackend* gBackend;

#include "vk_helper.h"

extern VkBackend* gBackend;

void CommandQueue::OnInit(ICommandQueue::QueueType type, uint32_t command_list_num, std::optional<std::wstring> dbg_name)
{
	m_type = type;
	m_command_list_num = command_list_num;

	VkDevice device = gBackend->GetDevice()->GetNativeObject();
	// queue
	uint8_t queue_bit = (type == ICommandQueue::QueueType::qt_gfx ? VK_QUEUE_GRAPHICS_BIT : VK_QUEUE_COMPUTE_BIT);
    uint32_t family_idx = TestFamilQueueyIndex(queue_bit);
	assert(family_idx != VK_QUEUE_FAMILY_IGNORED);

	vkGetDeviceQueue(device, family_idx, 0, &m_queue);
	assert(m_queue);

	// cmd pools
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = family_idx;

	VK_CHECK(vkCreateCommandPool(device, &createInfo, 0, &m_command_pool));

	// cmd buffers
	cmd_lists.resize(command_list_num);
	for (uint32_t i = 0; i < command_list_num; i++) {
		VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = m_command_pool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		VkCommandBuffer cmd_list_native;
		VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &cmd_list_native));
		
		CommandList * cmd_list = new CommandList; // TODO: when to delete?
		cmd_list->m_command_list = cmd_list_native;
		cmd_list->m_queue = this;
		cmd_lists[i]= cmd_list;
	}

	{
		VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = m_command_pool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &m_cmd_list_cpu_sync));
		VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &m_cmd_list_gpu_sync));
	}

	m_fence.reset(new Fence);
	m_fence->Initialize(0);

	for (uint32_t i = 0; i < 2; i++){
		m_dynamic_gpu_heaps[i].reset(new DynamicGpuHeap);
		m_dynamic_gpu_heaps[i]->Initialize((uint32_t)m_type);
	}
}

void CommandQueue::OnDestroy() {
	VkDevice device = gBackend->GetDevice()->GetNativeObject();
	Flush();
	
	vkDestroyCommandPool(device, m_command_pool, 0);
}

void CommandQueue::Signal(IFence* fence, bool on_cpu) {
	Fence* fence_vk = (Fence*)fence;
	if (on_cpu){
		// Submit a command buffer to the GPU
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_cmd_list_cpu_sync;
		vkQueueSubmit(m_queue, 1, &submitInfo, fence_vk->GetFence());
	}
	else {
		// Submit command buffer to Queue1 and signal S1 when it's done
		VkSubmitInfo submitInfo1 = {};
		submitInfo1.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo1.commandBufferCount = 1;
		submitInfo1.pCommandBuffers = &m_cmd_list_gpu_sync;
		submitInfo1.signalSemaphoreCount = 1;
		submitInfo1.pSignalSemaphores = &(fence_vk->GetSemaphore());
		vkQueueSubmit(m_queue, 1, &submitInfo1, VK_NULL_HANDLE);
	}
}

void CommandQueue::WaitOnCPU(IFence* fence) {
	Fence* fence_vk = (Fence*)fence;
	VkDevice device = gBackend->GetDevice()->GetNativeObject();
	vkWaitForFences(device, 1, &fence_vk->GetFence(), VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &fence_vk->GetFence());
}

void CommandQueue::WaitOnGPU(IFence* fence) {
	Fence* fence_vk = (Fence*)fence;
	// Submit command buffer to Queue2 and wait on S2 before starting
	VkSubmitInfo submitInfo2 = {};
	submitInfo2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo2.commandBufferCount = 1;
	submitInfo2.pCommandBuffers = &m_cmd_list_gpu_sync;
	submitInfo2.waitSemaphoreCount = 1;
	submitInfo2.pWaitSemaphores = &fence_vk->GetSemaphore();
	submitInfo2.pWaitDstStageMask = nullptr; // TODO: ????
	vkQueueSubmit(m_queue, 1, &submitInfo2, VK_NULL_HANDLE);
}

uint32_t CommandQueue::Signal() {
	Signal(m_fence.get(), true);
}

void CommandQueue::WaitOnCPU(uint32_t fence_value) {
	WaitOnCPU(m_fence.get());
}

void CommandQueue::Flush()  {
	Signal();
	WaitOnCPU(uint32_t(0));
}

ICommandList* CommandQueue::ResetActiveCL()  {
	m_active_cl = ++m_active_cl % m_command_list_num;

	CommandList* cmd_list = (CommandList*)cmd_lists[m_active_cl];

	vkResetCommandBuffer(cmd_list->GetNativeObject(), 0);

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmd_list->GetNativeObject(), &beginInfo);

	cmd_list->Reset();

	return cmd_lists[m_active_cl];
}

ICommandList* CommandQueue::GetActiveCL()  {
	return cmd_lists[m_active_cl];
}

void CommandQueue::ExecuteActiveCL()  {
	VK_CHECK(vkEndCommandBuffer(((CommandList*)GetActiveCL())->GetNativeObject()));
}

IDynamicGpuHeap& CommandQueue::GetGpuHeap()  {
	return *m_dynamic_gpu_heaps[gBackend->GetCurrentBackBufferIndex()];
}

uint32_t CommandQueue::TestFamilQueueyIndex(uint8_t queueFlags) {
	uint32_t queueCount = 0;
	VkPhysicalDevice phys_dev = gBackend->GetDevice()->GetPhysicalDevice();
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queueCount, 0);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &queueCount, queues.data());
	uint32_t fallback_queue_family = VK_QUEUE_FAMILY_IGNORED;

	for (uint32_t i = 0; i < queueCount; ++i){
        if ((queues[i].queueFlags & queueFlags))
			return i;

		if (queues[i].queueFlags & queueFlags){
			fallback_queue_family = i;
		}
	}

	return fallback_queue_family;
}
