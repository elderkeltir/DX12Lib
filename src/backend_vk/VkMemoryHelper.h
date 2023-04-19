#pragma once

#include <volk.h>
#include <optional>
// forward declaration for VMA
VK_DEFINE_HANDLE(VmaAllocator)

#include "vk_helper.h"

class VkMemoryHelper
{
public:

    void Init();
    BufferMemAllocation AllocateBuffer(uint32_t size, VkBufferUsageFlags usage, std::optional<std::wstring> dbg_name = std::nullopt);
    ImageMemAllocation AllocateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspect, std::optional<std::wstring> dbg_name = std::nullopt);
    void Deallocate(BufferMemAllocation &alloc);
    void Deallocate(ImageMemAllocation &alloc);
    void* Map(const MemAllocation& allocation);
    void Unmap(const MemAllocation& allocation);
    ~VkMemoryHelper();
private:
    VmaAllocator m_allocator;
};
