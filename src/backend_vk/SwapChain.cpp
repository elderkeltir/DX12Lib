#include "SwapChain.h"
#include <vulkan/vulkan_win32.h>
#include "VkBackend.h"
#include "CommandQueue.h"
#include "vk_helper.h"
#include "HeapBuffer.h"
#include "VkDevice.h"
#include "VkGpuResource.h"

#include <Windows.h>


extern VkBackend* gBackend;
class GLFWwindow;

void SwapChain::OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, uint32_t frame_count) {
	m_width = width;
	m_height = height;
    VkDevice device = gBackend->GetDevice()->GetNativeObject();
    VkPhysicalDevice physical_device = gBackend->GetDevice()->GetPhysicalDevice();

	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = *(HINSTANCE*)window_hndl.instance; // Your Windows HINSTANCE
	createInfo.hwnd = *(HWND*)window_hndl.ptr; // Your Windows HWND

	VK_CHECK(vkCreateWin32SurfaceKHR(gBackend->GetInstance(), &createInfo, nullptr, &m_surface));

    VkFormat format = GetSwapchainFormat();
    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageAspectFlags depth_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

    CreateSwapChain(format);
	assert(m_swap_chain);

    uint32_t imageCount{0};
	VK_CHECK(vkGetSwapchainImagesKHR(device, m_swap_chain, &imageCount, 0));
    assert(imageCount == 2);

    std::vector<VkImage> images(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(device, m_swap_chain, &imageCount, images.data()));

	for (uint32_t i = 0; i < imageCount; ++i)
	{
        std::unique_ptr<IGpuResource> rt(new VkGpuResource);
		((VkGpuResource*)rt.get())->InitBuffer();
        if (std::shared_ptr<IHeapBuffer> buf = rt->GetBuffer().lock()){
            HeapBuffer* buffer = (HeapBuffer*) buf.get();
            buffer->SetImage(images[i], format, aspect);
        }
        rt->CreateRTV();

        m_renderTargets[i].swap(rt);
	}

	// depth buffer
    m_depth_stencil.reset(new VkGpuResource);
    HeapType h_type = (HeapType)(HeapType::ht_default | HeapType::ht_image_depth_stencil_attachment | ht_aspect_depth_bit); // TODO: fill in
    ResourceDesc res_desc;
    res_desc.format = ResourceFormat::rf_d32_float;
    res_desc.width = width;
    res_desc.height = height;
    m_depth_stencil->CreateTexture(h_type, res_desc, ResourceState::rs_resource_state_depth_write, nullptr, L"depth_buffer"); // TODO: state?
    DSVdesc depth_desc; // TODO: seems like shit?
    m_depth_stencil->Create_DSV(depth_desc);
    SRVdesc srv_desc;
    m_depth_stencil->Create_SRV(srv_desc);

	m_frame_buffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		std::vector<IGpuResource*> rts{ m_renderTargets[i].get() };
        gBackend->CreateFrameBuffer(rts, nullptr, ITechniques::TecnhinueType::tt_post_processing);
	}
}

uint32_t SwapChain::GetCurrentBackBufferIndex() const {
    return m_current_frame_id;
}

IGpuResource& SwapChain::GetCurrentBackBuffer() {
    return *m_renderTargets[m_current_frame_id];
}

IGpuResource* SwapChain::GetDepthBuffer() {
    return m_depth_stencil.get();
}

void SwapChain::Present() {
	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	//presentInfo.pWaitSemaphores = &m_releaseSemaphore; // TODO: ???
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swap_chain;
	presentInfo.pImageIndices = &m_current_frame_id;

    std::shared_ptr<ICommandQueue>& gfx_queue = gBackend->GetQueue(ICommandQueue::QueueType::qt_gfx);
    CommandQueue* queue = (CommandQueue*) gfx_queue.get();
	VK_CHECK(vkQueuePresentKHR(queue->GetNativeObject(), &presentInfo));
}

void SwapChain::CreateSwapChain(VkFormat format) {
    VkDevice device = gBackend->GetDevice()->GetNativeObject();

	VkSurfaceCapabilitiesKHR surfaceCaps = GetSurfaceCapabilities();
	VkCompositeAlphaFlagBitsKHR surfaceComposite =
		(surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
		: (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
		? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
		: (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
		? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
		: VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

    assert(surfaceCaps.minImageCount >= 2); // TODO: magic number

    uint32_t family_idx = gBackend->GetDevice()->GetFamilyIndex(true);
	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = m_surface;
	createInfo.minImageCount = 2; // TODO: magic number
	createInfo.imageFormat = format;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = m_width;
	createInfo.imageExtent.height = m_height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.queueFamilyIndexCount = 1;
	createInfo.pQueueFamilyIndices = &family_idx;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = surfaceComposite;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &m_swap_chain));
}

VkSurfaceCapabilitiesKHR SwapChain::GetSurfaceCapabilities() const {
	VkSurfaceCapabilitiesKHR surfaceCaps;
    VkPhysicalDevice physical_device = gBackend->GetDevice()->GetPhysicalDevice();
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_surface, &surfaceCaps));

	return surfaceCaps;
}

VkFormat SwapChain::GetSwapchainFormat() const{
    VkPhysicalDevice physical_device = gBackend->GetDevice()->GetPhysicalDevice();

	uint32_t formatCount = 0;
	assert(vkGetPhysicalDeviceSurfaceFormatsKHR);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &formatCount, 0));
	assert(formatCount > 0);

	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &formatCount, formats.data()));

	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		return VK_FORMAT_R8G8B8A8_UNORM;

	for (uint32_t i = 0; i < formatCount; ++i)
		if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM || formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
			return formats[i].format;

	return formats[0].format;
}

SwapChain::~SwapChain() {

}
