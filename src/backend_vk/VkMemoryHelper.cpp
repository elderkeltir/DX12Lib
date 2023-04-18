#include "VkMemoryHelper.h"
#include "VkBackend.h"
#include "VkDevice.h"
#include "vk_helper.h"

#include <volk.h>
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.3
#include "vk_mem_alloc.h"

extern VkBackend *gBackend;

void VkMemoryHelper::Init()
{
    VkDevice device = gBackend->GetDevice()->GetNativeObject();
    VkPhysicalDevice physical_device = gBackend->GetDevice()->GetPhysicalDevice();
    VkInstance instance = gBackend->GetInstance();

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vulkanFunctions.vkCreateImage = vkCreateImage;
    vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vulkanFunctions.vkDestroyImage = vkDestroyImage;
    vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkFreeMemory = vkFreeMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkMapMemory = vkMapMemory;
    vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
    vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
    vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
    vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = physical_device;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.preferredLargeHeapBlockSize = 1024;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
}

BufferMemAllocation VkMemoryHelper::AllocateBuffer(uint32_t size, VkBufferUsageFlags usage, std::optional<std::wstring> dbg_name)
{
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage; // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr);

    VmaAllocationInfo alloc_info;
    vmaGetAllocationInfo(m_allocator, allocation, &alloc_info);

    {
        VkDevice device = gBackend->GetDevice()->GetNativeObject();
        SetName(device, (uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, dbg_name.value());
    }

    BufferMemAllocation buffer_mem_alloc;
    buffer_mem_alloc.user_data = (uint64_t)allocation;
    buffer_mem_alloc.buffer = buffer;
    buffer_mem_alloc.size = alloc_info.size;
    buffer_mem_alloc.offset = alloc_info.offset;
    buffer_mem_alloc.is_image = false;

    return buffer_mem_alloc;
}

ImageMemAllocation VkMemoryHelper::AllocateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspect, std::optional<std::wstring> dbg_name)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkImage image;
    VmaAllocation allocation;
    vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr);

    {
        VkDevice device = gBackend->GetDevice()->GetNativeObject();
        SetName(device, (uint64_t)image, VK_OBJECT_TYPE_IMAGE, dbg_name.value());
    }
    
    ImageMemAllocation img_mem_alloc;
    img_mem_alloc.user_data = (uint64_t)allocation;
    img_mem_alloc.image = image;
    img_mem_alloc.format = format;
    img_mem_alloc.aspect = aspect;
    img_mem_alloc.is_image = true;

    return img_mem_alloc;
}

void VkMemoryHelper::Deallocate(BufferMemAllocation &alloc) {
    vmaDestroyBuffer(m_allocator, alloc.buffer, (VmaAllocation)alloc.user_data);
}

void VkMemoryHelper::Deallocate(ImageMemAllocation &alloc) {
    vmaDestroyImage(m_allocator, alloc.image, (VmaAllocation)alloc.user_data);
}

void* VkMemoryHelper::Map(const MemAllocation& allocation) {
    VmaAllocation allocation_native = (VmaAllocation)allocation.user_data;
    void* mappedData{nullptr};
    vmaMapMemory(m_allocator, allocation_native, &mappedData);
    assert(mappedData);

    return mappedData;
}

void VkMemoryHelper::Unmap(const MemAllocation& allocation) {
    VmaAllocation allocation_native = (VmaAllocation)allocation.user_data;
    vmaUnmapMemory(m_allocator, allocation_native);
}

VkMemoryHelper::~VkMemoryHelper() {
    vmaDestroyAllocator(m_allocator);
}