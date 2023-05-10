#pragma once

#include <wrl.h>
#include <memory>
#include <array>
#include "defines.h"
using Microsoft::WRL::ComPtr;
struct IDXGISwapChain4;
struct IDXGIFactory4;
class IGpuResource;

class SwapChain {
public:
	void OnInit(IDXGIFactory4* pFactory, const WindowHandler& window_hndl, uint32_t width, uint32_t height, uint32_t frame_count);
	uint32_t GetCurrentBackBufferIndex() const;
	IGpuResource& GetCurrentBackBuffer();
	IGpuResource* GetDepthBuffer();
	HRESULT Present();
	void OnResize();

	uint32_t GetHeight() { return m_height; }
	uint32_t GetWidth() { return m_width; }
	const WindowHandler& GetWindowHndl() const { return m_win_hndl; }
	~SwapChain();
private:
	ComPtr<IDXGISwapChain4> m_swapChain;
	std::array<std::unique_ptr<IGpuResource>, 2> m_renderTargets;
	std::unique_ptr<IGpuResource> m_depthStencil;
	WindowHandler m_win_hndl;
	uint32_t m_width;
	uint32_t m_height;
};