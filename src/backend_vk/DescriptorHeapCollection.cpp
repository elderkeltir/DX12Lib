#include "DescriptorHeapCollection.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "ResourceDescriptor.h"
#include "vk_helper.h"
#include <vulkan/vulkan_core.h>

extern VkBackend* gBackend;

void DescriptorHeapCollection::Initialize(std::optional<std::wstring> dbg_name) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    std::vector<VkDescriptorPoolSize> poolSizes(5);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSizes[0].descriptorCount = srv_img_desc_count;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[1].descriptorCount = uav_img_desc_count;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	poolSizes[2].descriptorCount = srv_buf_desc_count;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[3].descriptorCount = cbv_buf_desc_count;
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	poolSizes[4].descriptorCount = uav_buf_desc_count;
	
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = uint32_t(srv_img_desc_count + uav_img_desc_count + srv_buf_desc_count + cbv_buf_desc_count + uav_buf_desc_count);

	for (uint32_t i = 0; i < frame_num * 2; i++){
		vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool_cpu[i]);

		//poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT; // TODO: most likely will not use this one?
		//vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool_gpu[i]);
	}
}

DescriptorHeapCollection::~DescriptorHeapCollection() {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();
	for (uint32_t i = 0; i < frame_num * 2; i++){
		vkDestroyDescriptorPool(device, m_pool_gpu[i], nullptr);
		vkDestroyDescriptorPool(device, m_pool_cpu[i], nullptr);
	}
}

void DescriptorHeapCollection::ReserveRTVhandle(CPUdescriptor& rtvHandle, uint64_t data, bool gpu_only) {
	const ImageMemAllocation* const img_data = (const ImageMemAllocation* const)data;

	VkDevice device = gBackend->GetDevice()->GetNativeObject();
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = img_data->image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = img_data->format;
	createInfo.subresourceRange.aspectMask = img_data->aspect;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView view = 0;
	VK_CHECK(vkCreateImageView(device, &createInfo, 0, &view));

	rtvHandle.ptr = (uint64_t)view;
}

void DescriptorHeapCollection::ReserveDSVhandle(CPUdescriptor& dsvHandle, uint64_t data, bool gpu_only) {
	const ImageMemAllocation* const img_data = (const ImageMemAllocation* const)data;

	VkDevice device = gBackend->GetDevice()->GetNativeObject();
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = img_data->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = img_data->format;
    viewInfo.subresourceRange.aspectMask = img_data->aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

	VkImageView view = 0;
    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &view));

	dsvHandle.ptr = (uint64_t)view;
}

void DescriptorHeapCollection::ReserveCBVhandle(CPUdescriptor& cbvHandle, uint64_t data, bool gpu_only) {
	return; // TODO: there is no need to bufferview here
}

void DescriptorHeapCollection::ReserveSRVhandle(CPUdescriptor& srvHandle, uint64_t data, bool gpu_only) {
	VkDevice device = gBackend->GetDevice()->GetNativeObject();
	const MemAllocation* const alloc_data = (const MemAllocation* const)data;
	if (alloc_data->is_image) {
		const ImageMemAllocation* const img_data = (const ImageMemAllocation* const)data;
		
		VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = img_data->image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = img_data->format;
		createInfo.subresourceRange.aspectMask = img_data->aspect;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView view = 0;
		VK_CHECK(vkCreateImageView(device, &createInfo, 0, &view));
		srvHandle.ptr = (uint64_t)view;
	}
	else {
		const BufferMemAllocation* const buf_data = (const BufferMemAllocation* const)data;

		VkBufferViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };
		createInfo.buffer = buf_data->buffer;
		createInfo.format = VK_FORMAT_R8_UINT; // the format of the data
		createInfo.offset = buf_data->offset; // the offset in bytes from the start of the buffer
		createInfo.range = buf_data->size; // the size of the buffer

		VkBufferView bufferView = VK_NULL_HANDLE;
		VkResult result = vkCreateBufferView(device, &createInfo, nullptr, &bufferView);
		srvHandle.ptr = (uint64_t)bufferView;
	}
}

void DescriptorHeapCollection::ReserveUAVhandle(CPUdescriptor& uavHandle, uint64_t data, bool gpu_only) {
	const ImageMemAllocation* const img_data = (const ImageMemAllocation* const)data;

	VkDevice device = gBackend->GetDevice()->GetNativeObject();
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = img_data->image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = img_data->format;
	createInfo.subresourceRange.aspectMask = img_data->aspect;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView view = 0;
	VK_CHECK(vkCreateImageView(device, &createInfo, 0, &view));

	uavHandle.ptr = (uint64_t)view;
}
