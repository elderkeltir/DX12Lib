#pragma once

#include <volk.h>
#include <string>
#include <cassert>
#include "defines.h"

#ifdef _DEBUG
inline void SetName(VkDevice device, uint64_t hndl, VkObjectType type, const std::wstring &name) {
	VkDebugUtilsObjectNameInfoEXT nameInfo = {};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = type;
	nameInfo.objectHandle = hndl; // this cast may vary by platform/compiler
    std::string s((const char*)&name[0], sizeof(wchar_t)/sizeof(char)*name.size());
    nameInfo.pObjectName = s.c_str();
	vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}
#endif // _DEBUG

#define VK_CHECK(call) \
	do { \
		VkResult result = call; \
		assert(result == VK_SUCCESS); \
	} while(0)

struct MemAllocation {
    uint64_t user_data{0};
};
struct ImageMemAllocation : public MemAllocation {
    VkImage image;
    VkFormat format;
    VkImageAspectFlags aspect;
};
struct BufferMemAllocation : public MemAllocation {
    VkBuffer buffer;
    VkDeviceSize size;
    VkDeviceSize offset;
};

// TODO: fill when needed
inline VkFormat ConvertResourceFormat(ResourceFormat format)
{
    switch (format)
    {
        case ResourceFormat::rf_r32g32b32a32_float: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default: return VK_FORMAT_UNDEFINED;
    }
}
