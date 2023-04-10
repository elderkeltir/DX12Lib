#include "LinApplication.h"

#ifndef WIN32

// hack just to connect Vulkan and test it works, temporary!
#define VULKAN_HACK_TEST
#ifdef VULKAN_HACK_TEST
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>

#define VK_CHECK(call) \
	do { \
		VkResult result = call; \
		assert(result == VK_SUCCESS); \
	} while(0)

VkInstance createInstance() {
	VkApplicationInfo appinfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appinfo.apiVersion = VK_API_VERSION_1_1; // TODO: check supported versions
	VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appinfo;

#ifdef _DEBUG
	const char* debugLayers[] =
	{
		"VK_LAYER_KHRONOS_validation",
	};

	createInfo.ppEnabledLayerNames = debugLayers;
	createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif

    uint32_t glfwExtensionsNum = 0;
	const char ** glfwxtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsNum);
    std::vector<const char*> extensions(glfwExtensionsNum + 1);
    for (size_t i = 0u; i < glfwExtensionsNum; i++){
        extensions[i] = glfwxtensions[i];
    }
	extensions[glfwExtensionsNum] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = extensions.size();

	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

	return instance;
}

VkPhysicalDevice pickPhysDevice(VkPhysicalDevice * devices, uint32_t num) {

	VkPhysicalDeviceProperties physicalDevProps;
	for (uint32_t i = 0; i < num; i++) {
		vkGetPhysicalDeviceProperties(devices[i], &physicalDevProps);

		if (physicalDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return devices[i];
		}
	}
	assert(false);
	return devices[0];
}

VkDevice createDevice(VkPhysicalDevice physDevice, uint32_t * familyIdx) {
	*familyIdx = 0; // TODO comupe from queue properties
	float queueProperties[] = { 0.f };
	VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueCreateInfo.queueFamilyIndex = *familyIdx;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = queueProperties;

	VkDevice device = 0;
	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	deviceCreateInfo.ppEnabledExtensionNames = extensions;
	deviceCreateInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VK_CHECK(vkCreateDevice(physDevice, &deviceCreateInfo, 0, &device));

	return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {

	VkSurfaceKHR khr_surface = 0;
    VK_CHECK(glfwCreateWindowSurface(instance, window, NULL, &khr_surface));

    return khr_surface;
}

VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, uint32_t width, uint32_t height)
{
	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = 1280;
	createInfo.imageExtent.height = 720;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.queueFamilyIndexCount = 1;
	createInfo.pQueueFamilyIndices = &familyIndex;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkSwapchainKHR swapchain = 0;

	VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));

	return swapchain;
}

VkSemaphore createSemaphore(VkDevice device)
{
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	VkSemaphore semaphore = 0;
	VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));

	return semaphore;
}

VkCommandPool createCommandPool(VkDevice &device, uint32_t familyIdx)
{
	VkCommandPool cmdPool = 0;

	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	createInfo.queueFamilyIndex = familyIdx;

	VK_CHECK(vkCreateCommandPool(device, &createInfo, NULL, &cmdPool));

	return cmdPool;
}   

#endif // VULKAN_HACK_TEST

int LinApplication::Run() {
    // Window


	assert(glfwInit());

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "DX12LibApp", NULL, NULL);

	// Vulkan
	VkInstance instance = createInstance();
	assert(instance);
	
	VkPhysicalDevice physicalDevices[16];
	uint32_t physDevNum = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physDevNum, physicalDevices));

	VkPhysicalDevice physDevice = pickPhysDevice(physicalDevices, physDevNum);

	uint32_t familyIdx = 0;
	VkDevice device = createDevice(physDevice, &familyIdx);

	VkSurfaceKHR surface = createSurface(instance, window);
	VkSwapchainKHR swapchain = createSwapchain(device, surface, familyIdx, 1280, 720);
	
	VkSemaphore acquireSemaphore = createSemaphore(device);
	
	VkSemaphore releaseSemaphore = createSemaphore(device);
	
	VkQueue queue = 0;
	vkGetDeviceQueue(device, familyIdx, 0, &queue);

	VkImage swapchainImages[16]; // SHORTCUT: seriously?
	uint32_t swapchainImageCount = sizeof(swapchainImages) / sizeof(swapchainImages[0]);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages));

	VkCommandPool cmdPool = createCommandPool(device, familyIdx);


	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = cmdPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(device, &allocateInfo, &cmdBuffer);

	// GLFW

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		
		uint32_t imageIndex = 0;
		VK_CHECK(vkAcquireNextImageKHR(device, swapchain, ~0ull, acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

		VK_CHECK(vkResetCommandPool(device, cmdPool, 0));

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
		
		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.layerCount = 1;

		VkClearColorValue color = { 1, 1, 0, 1 };

		vkCmdClearColorImage(cmdBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);

		VK_CHECK(vkEndCommandBuffer(cmdBuffer));

		VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &acquireSemaphore;
		submitInfo.pWaitDstStageMask = &submitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &releaseSemaphore;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &releaseSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;

		VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));
		vkDeviceWaitIdle(device);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	vkDestroySurfaceKHR(instance, surface, 0);
	vkDestroyDevice(device, 0);
	vkDestroyInstance(instance, 0);
	

	return 0;
}

LinApplication::LinApplication(uint32_t width, uint32_t height, const std::wstring& window_name) :
    m_width(width),
    m_height(height),
    m_window_name(window_name)
{

}

#endif // WIN32