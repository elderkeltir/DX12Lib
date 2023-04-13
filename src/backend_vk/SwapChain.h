#pragma once

#include <memory>
#include <array>
#include "defines.h"
#include <volk.h>

class IGpuResource;

class SwapChain {
public:
	void OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, uint32_t frame_count);
	uint32_t GetCurrentBackBufferIndex() const;
	IGpuResource& GetCurrentBackBuffer();
	IGpuResource* GetDepthBuffer();
	void Present();
	void OnResize() {}

	uint32_t GetHeight() { return m_height; }
	uint32_t GetWidth() { return m_width; }
	const WindowHandler& GetWindowHndl() const { return m_win_hndl; }
	~SwapChain();
private:
	void CreateSwapChain(VkFormat format);
	VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() const;
	VkFormat GetSwapchainFormat() const;

	VkSwapchainKHR m_swap_chain;
	VkSurfaceKHR m_surface;
	std::vector<VkFramebuffer> m_frame_buffers;

	std::array<std::unique_ptr<IGpuResource>, 2> m_renderTargets;
	std::unique_ptr<IGpuResource> m_depth_stencil;
	WindowHandler m_win_hndl;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_current_frame_id {0};
};