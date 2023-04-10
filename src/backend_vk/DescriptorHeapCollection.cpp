#include "DescriptorHeapCollection.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "ResourceDescriptor.h"
#include "vk_helper.h"

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

	vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool_cpu);

    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT; // TODO: most likely will not use this one?
    vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool_gpu);
}

DescriptorHeapCollection::~DescriptorHeapCollection() {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

    vkDestroyDescriptorPool(device, m_pool_gpu, nullptr);
    vkDestroyDescriptorPool(device, m_pool_cpu, nullptr);
}

void DescriptorHeapCollection::ReserveRTVhandle(CPUdescriptor& rtvHandle, uint64_t data = 0, bool gpu_only) {
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

void DescriptorHeapCollection::ReserveDSVhandle(CPUdescriptor& dsvHandle, uint64_t data = 0, bool gpu_only) {
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

void DescriptorHeapCollection::ReserveCBVhandle(CPUdescriptor& cbvHandle, uint64_t data = 0, bool gpu_only) {
	return; // TODO: there is no need to bufferview here
}

void DescriptorHeapCollection::ReserveSRVhandle(CPUdescriptor& srvHandle, uint64_t data = 0, bool gpu_only) {
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

	srvHandle.ptr = (uint64_t)view;
}

void DescriptorHeapCollection::ReserveUAVhandle(CPUdescriptor& uavHandle, uint64_t data = 0, bool gpu_only) {
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
