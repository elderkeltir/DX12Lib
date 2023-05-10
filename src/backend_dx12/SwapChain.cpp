#include "SwapChain.h"
#include "defines.h"
#include "dx12_helper.h"
#include "DxBackend.h"
#include "GpuResource.h"
#include "CommandQueue.h"

#include <directx/d3dx12.h>
#include <dxgi1_6.h>

extern DxBackend* gBackend;

SwapChain::~SwapChain() = default;

void SwapChain::OnInit(IDXGIFactory4* pFactory, const WindowHandler& window_hndl, uint32_t width, uint32_t height, uint32_t frame_count)
{
	m_win_hndl = window_hndl;
	m_width = width;
	m_height = height;

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = frame_count;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	HWND hwnd = *(HWND*)window_hndl.ptr;

	CommandQueue* queue = (CommandQueue*)(gBackend->GetQueue(ICommandQueue::QueueType::qt_gfx).get());

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(pFactory->CreateSwapChainForHwnd(
		queue->GetNativeObject().Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	// Create frame resources.
	{
		// Create a RTV for each frame.
		for (uint32_t i = 0; i < frame_count; i++)
		{
			ComPtr<ID3D12Resource> renderTarget;
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget)));
			SetName(renderTarget, std::wstring(L"render_target_").append(std::to_wstring(i)).c_str());
			GpuResource* res = new GpuResource;
			res->SetBuffer(renderTarget);
			m_renderTargets[i].reset(res);
			m_renderTargets[i]->CreateRTV();
		}
	}

	// Create the depth stencil view.
	{
		m_depthStencil.reset(new GpuResource);
		ClearColor depthOptimizedClearValue = {};
		depthOptimizedClearValue.format = ResourceFormat::rf_d32_float;
		depthOptimizedClearValue.isDepth = true;
		depthOptimizedClearValue.depth_tencil.depth = 1.0f;
		depthOptimizedClearValue.depth_tencil.stencil = 0;
		ResourceDesc res_desc = ResourceDesc::tex_2d(ResourceFormat::rf_d32_float, width, height, 1, 0, 1, 0, ResourceDesc::ResourceFlags::rf_allow_depth_stencil);

		m_depthStencil->CreateTexture(HeapType::ht_default, res_desc, ResourceState::rs_resource_state_depth_write, &depthOptimizedClearValue, L"depth_stencil");

		DSVdesc depthStencilDesc = {};
		depthStencilDesc.format = ResourceFormat::rf_d32_float;
		depthStencilDesc.dimension = DSVdesc::DSVdimensionType::dsv_dt_texture2d;

		m_depthStencil->Create_DSV(depthStencilDesc);

		SRVdesc srv_desc = {};

		srv_desc.format = ResourceFormat::rf_r32_float;
		srv_desc.dimension = SRVdesc::SRVdimensionType::srv_dt_texture2d;
		srv_desc.texture2d.most_detailed_mip = 0;
		srv_desc.texture2d.mip_levels = 1;
		srv_desc.texture2d.res_min_lod_clamp = 0.0f;
		m_depthStencil->Create_SRV(srv_desc);
	}
}

uint32_t SwapChain::GetCurrentBackBufferIndex() const
{
	return m_swapChain->GetCurrentBackBufferIndex();
}

IGpuResource& SwapChain::GetCurrentBackBuffer()
{
	return *m_renderTargets[GetCurrentBackBufferIndex()];
}

IGpuResource* SwapChain::GetDepthBuffer()
{
	return m_depthStencil.get();
}

HRESULT SwapChain::Present()
{
	return m_swapChain->Present(1, 0);
}

void SwapChain::OnResize()
{
	// TODO: process window resize
}
