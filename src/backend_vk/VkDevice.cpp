#include "VkDevice.h"
#include "vk_helper.h"
#include "VkBackend.h"
#include <vulkan/vulkan_core.h>

extern VkBackend* gBackend;

void VkDeviceW::Init(VkInstance instance) {
    // pick Physical device
	uint32_t physicalDeviceCount;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));
	m_phys_device = PickPhysicalDevice(physicalDevices);
	assert(m_phys_device);

    // create Device
	CreateDevice();
	assert(m_device);
	volkLoadDevice(m_device);
}

VkPhysicalDevice VkDeviceW::PickPhysicalDevice(const std::vector<VkPhysicalDevice> &physicalDevices) {
	VkPhysicalDevice discrete = 0;
	VkPhysicalDevice fallback = 0;

	for (uint32_t i = 0; i < physicalDevices.size(); ++i) {
		bool graphicsSupported = false;
		bool computeSupported = false;
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

		// TODO: save somewhere
		VkDeviceSize alignment = props.limits.minUniformBufferOffsetAlignment;

		printf("GPU%d: %s\n", i, props.deviceName); // TODO: some ifdef for linux and perf build to select proprietary amdgpu driver to actually use amd perf tool
		{
			uint32_t familyIndex = TestFamilQueueyIndex(physicalDevices[i], VK_QUEUE_GRAPHICS_BIT);
			if (familyIndex != VK_QUEUE_FAMILY_IGNORED)
			{
				graphicsSupported = true;
                m_family_idx_gfx = familyIndex;
			}
		}
		{
			uint32_t familyIndex = TestFamilQueueyIndex(physicalDevices[i], VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
			if (familyIndex != VK_QUEUE_FAMILY_IGNORED && familyIndex != m_family_idx_gfx)
			{
				computeSupported = true;
                m_family_idx_compute = familyIndex;
			}
		}

		if (!discrete && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && graphicsSupported && computeSupported) {
			discrete = physicalDevices[i];
		}

		if (!fallback && graphicsSupported) {
			fallback = physicalDevices[i];
		}
	}

	VkPhysicalDevice result = discrete ? discrete : fallback;

	if (result) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(result, &props);

		printf("Selected GPU %s\n", props.deviceName);
	}
	else {
		printf("ERROR: No GPUs found\n");
	}

	return result;
}

void VkDeviceW::CreateDevice(){
	float queuePriorities[] = { 1.0f };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(2);

	queueCreateInfos[0] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfos[0].queueFamilyIndex = m_family_idx_gfx;
	queueCreateInfos[0].queueCount = 1;
	queueCreateInfos[0].pQueuePriorities = queuePriorities;

	queueCreateInfos[1] = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfos[1].queueFamilyIndex = m_family_idx_compute;
	queueCreateInfos[1].queueCount = 1;
	queueCreateInfos[1].pQueuePriorities = queuePriorities;

	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
		VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME
	#ifdef VK_USE_PLATFORM_METAL_EXT
		VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
	#endif
		//VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, 1:35
	};

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT ext_struct {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT };
	ext_struct.vertexInputDynamicState = VK_TRUE;
	createInfo.pNext = &ext_struct;

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	// TODO: shortcut. we need to check is it supported vkGetPhysicalDeviceFeatures
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	createInfo.pEnabledFeatures = &deviceFeatures;

	VK_CHECK(vkCreateDevice(m_phys_device, &createInfo, 0, &m_device));
}

uint32_t VkDeviceW::TestFamilQueueyIndex(VkPhysicalDevice phys_device, uint8_t queueFlags, uint8_t queueNotFlags /*VkQueueFlagBits*/) const {
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queueCount, 0);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queueCount, queues.data());
	uint32_t fallback_queue_family = VK_QUEUE_FAMILY_IGNORED;

	for (uint32_t i = 0; i < queueCount; ++i){
        if ((queues[i].queueFlags & queueFlags) && !(queues[i].queueFlags & queueNotFlags))
			return i;

		if (queues[i].queueFlags & queueFlags){
			fallback_queue_family = i;
		}
	}

	return fallback_queue_family;
}
