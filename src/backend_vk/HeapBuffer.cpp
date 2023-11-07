#include "HeapBuffer.h"
#include "VkMemoryHelper.h"
#include "VkBackend.h"
#include "CommandList.h"
#include <cstring> // memcpy?

extern VkBackend *gBackend;

VkImageUsageFlags CastHeapTypeImage(HeapType type){
    VkImageUsageFlags usage{0};
    if (type & HeapType::ht_image_sampled)
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (type & HeapType::ht_image_depth_stencil_attachment)
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (type & HeapType::ht_image_storage)
        usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (type & HeapType::ht_image_color_attach)
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return usage;
}

VkBufferUsageFlags CastHeapTypeBuffer(HeapType type){
    VkBufferUsageFlags usage{0};
    if (type & HeapType::ht_buff_transfer_src)
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (type & HeapType::ht_buff_transfer_dst)
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (type & HeapType::ht_buff_uniform_buffer)
        usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (type & HeapType::ht_buff_index_buffer)
        usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (type & HeapType::ht_buff_srv_buffer)
        usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

    return usage;
}

VkImageAspectFlags CatsAspectFlags(HeapType type) {
    VkImageAspectFlags aspect_bits(VK_IMAGE_ASPECT_NONE);
    if (type & HeapType::ht_aspect_color_bit)
        aspect_bits |= VK_IMAGE_ASPECT_COLOR_BIT;
    if (type & HeapType::ht_aspect_depth_bit)
        aspect_bits |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if (type & HeapType::ht_aspect_stencil_bit)
        aspect_bits |= VK_IMAGE_ASPECT_STENCIL_BIT;
        
    return aspect_bits;
}

void HeapBuffer::Create(HeapType type, uint32_t bufferSize, ResourceState initial_state, std::optional<std::wstring> dbg_name) {
    m_type = BufferResourceType::rt_buffer;
    VkBufferUsageFlags usage = CastHeapTypeBuffer(type);
    m_buffer_allocation = gBackend->GetMemoryHelper()->AllocateBuffer(bufferSize, usage, dbg_name);
}

void HeapBuffer::CreateTexture(HeapType type, const ResourceDesc& res_desc, ResourceState initial_state, const ClearColor* clear_val, std::optional<std::wstring> dbg_name) {
    m_type = BufferResourceType::rt_texture;
    VkImageUsageFlags usage = CastHeapTypeImage(type);
    VkFormat format = ConvertResourceFormat(res_desc.format);
    assert(format != VK_FORMAT_UNDEFINED);
    VkImageAspectFlags aspect = CatsAspectFlags(type);
    m_image_allocation = gBackend->GetMemoryHelper()->AllocateImage(res_desc.width, res_desc.height, format, VK_IMAGE_TILING_OPTIMAL, usage, aspect, dbg_name);
}

void HeapBuffer::Load(ICommandList* command_list, uint32_t numElements, uint32_t elementSize, const void* bufferData) {
    assert(m_type == BufferResourceType::rt_buffer);

    const uint32_t buff_size = numElements * elementSize;
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    BufferMemAllocation buffer_allocation = gBackend->GetMemoryHelper()->AllocateBuffer(buff_size, usage, L"src_buffer");
    void* data = gBackend->GetMemoryHelper()->Map(buffer_allocation);

    std::memcpy(data, bufferData, buff_size);
    gBackend->GetMemoryHelper()->Unmap(buffer_allocation);

    VkCommandBuffer cmd_list = ((CommandList*)command_list)->GetNativeObject();

    VkBufferCopy copyRegion{};
	copyRegion.srcOffset = buffer_allocation.offset;
	copyRegion.dstOffset = m_buffer_allocation.offset;
	copyRegion.size = buffer_allocation.size;
	vkCmdCopyBuffer(cmd_list, buffer_allocation.buffer, m_buffer_allocation.buffer, 1, &copyRegion);
}

void HeapBuffer::Load(ICommandList* command_list, uint32_t firstSubresource, uint32_t numSubresources, SubresourceData* subresourceData) {
    assert(m_type == BufferResourceType::rt_texture);
    assert(numSubresources == 1); // TODO: handle this later

    const uint32_t buff_size = subresourceData->slice_pitch;
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    BufferMemAllocation buffer_allocation = gBackend->GetMemoryHelper()->AllocateBuffer(buff_size, usage, L"src_buffer");
    void* data = gBackend->GetMemoryHelper()->Map(buffer_allocation);

    std::memcpy(data, subresourceData->data, buff_size);
    gBackend->GetMemoryHelper()->Unmap(buffer_allocation);

    VkCommandBuffer cmd_list = ((CommandList*)command_list)->GetNativeObject();

    VkBufferImageCopy region{};
	region.bufferOffset = buffer_allocation.offset;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

    SubresourceDataVk* subres_data_vk = (SubresourceDataVk*)subresourceData;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
		subres_data_vk->width,
		subres_data_vk->height,
		1
	};

	vkCmdCopyBufferToImage(
		cmd_list,
		buffer_allocation.buffer,
		m_image_allocation.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
}

void* HeapBuffer::Map() {
    if (m_type == BufferResourceType::rt_buffer)
        m_cpu_data = (uint8_t*)gBackend->GetMemoryHelper()->Map(m_buffer_allocation);
    else
        m_cpu_data =  (uint8_t*)gBackend->GetMemoryHelper()->Map(m_image_allocation);

    return m_cpu_data;
}

void HeapBuffer::Unmap() {
    if (m_type == BufferResourceType::rt_buffer)
        gBackend->GetMemoryHelper()->Unmap(m_buffer_allocation);
    else
        gBackend->GetMemoryHelper()->Unmap(m_image_allocation);
    
    m_cpu_data = nullptr;
}

void HeapBuffer::SetImage(VkImage image, VkFormat format, VkImageAspectFlags aspect) {
    m_image_allocation.image = image;
    m_image_allocation.user_data = 0;
    m_image_allocation.aspect = aspect;
    m_image_allocation.format = format;

    m_type = BufferResourceType::rt_texture;
}
