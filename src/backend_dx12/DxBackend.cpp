#include "DxBackend.h"
#include "defines.h"
#include "dx12_helper.h"
#include "DxDevice.h"
#include "SwapChain.h"
#include "Logger.h"
#include "Fence.h"
#include "DescriptorHeapCollection.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "Techniques.h"
#include "ImguiHelper.h"
#include "ShaderManager.h"

#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include "WinPixEventRuntime/pix3.h"
#include <cassert>

#if defined(USE_PIX) && defined(USE_PIX_DEBUG)
#include "PIXapi.h"
#endif // defined(USE_PIX) && defined(USE_PIX_DEBUG)

DxBackend* gBackend = nullptr;

IBackend* CreateBackend() {
	assert(!gBackend);

	gBackend = new DxBackend;
	return (IBackend *)gBackend;
}

void DestroyBackend() {
	assert(gBackend);
	delete gBackend;
	gBackend = nullptr;
}

void DxBackend::OnInit(const WindowHandler& window_hndl, uint32_t width, uint32_t height, const std::filesystem::path& path)
{
	// Factory
#if defined(USE_PIX) && defined(USE_PIX_DEBUG)
	{
		// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
		// This may happen if the application is launched through the PIX UI. 
		if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
		{
			LoadLibrary(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
		}
	}
#endif // defined(USE_PIX) && defined(USE_PIX_DEBUG)
	uint32_t dxgiFactoryFlags = 0;

#if defined(_DEBUG) && !defined(USE_PIX_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));

	// Device
	m_device.swap(std::make_unique<DxDevice>());
	m_device->OnInit(m_factory.Get());

	// Queues
	m_commandQueueGfx.reset(new CommandQueue);
	m_commandQueueCompute.reset(new CommandQueue);
	m_commandQueueGfx->OnInit(ICommandQueue::QueueType::qt_gfx, GfxQueueCmdList_num, L"Gfx");
	m_commandQueueCompute->OnInit(ICommandQueue::QueueType::qt_compute, ComputeQueueCmdList_num, L"Compute");

	m_descriptor_heap_collection.swap(std::make_shared<DescriptorHeapCollection>());
	m_descriptor_heap_collection->Initialize();

	// SwapChain
	m_swap_chain.swap(std::make_unique<SwapChain>());
	m_swap_chain->OnInit(m_factory.Get(), window_hndl, width, height, FramesCount);

	// Misc
	m_viewport = ViewPort(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	m_scissorRect = RectScissors(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

	m_logger.swap(std::make_unique<logger>("app.log", logger::log_level::ll_INFO));
	
	m_frameIndex = m_swap_chain->GetCurrentBackBufferIndex();
	m_root_dir = path;

	m_shader_mgr.swap(std::make_unique<ShaderManager>());
	m_techniques.reset(new Techniques);
	m_techniques->OnInit();

	m_gui.reset(new ImguiHelper);
	m_gui->Initialize(FramesCount);
	
	m_fence_inter_queue.reset(new Fence);
	m_fence_inter_queue->Initialize(m_fence_inter_queue_val);
}

uint32_t DxBackend::GetCurrentBackBufferIndex() const
{
	return m_swap_chain->GetCurrentBackBufferIndex();
}

IGpuResource& DxBackend::GetCurrentBackBuffer()
{
	return m_swap_chain->GetCurrentBackBuffer();
}

IGpuResource* DxBackend::GetDepthBuffer()
{
	return m_swap_chain->GetDepthBuffer();
}

void DxBackend::Present()
{
	m_swap_chain->Present();

	// Signal for this frame
	m_fenceValues[m_frameIndex] = m_commandQueueGfx->Signal();

	// get next frame
	m_frameIndex = m_swap_chain->GetCurrentBackBufferIndex();
}

void DxBackend::OnResizeWindow()
{
	m_swap_chain->OnResize();
}

void DxBackend::SyncWithCPU()
{
	m_commandQueueGfx->WaitOnCPU(m_fenceValues[m_frameIndex]);
}

void DxBackend::SyncWithGpu(ICommandQueue::QueueType from, ICommandQueue::QueueType to)
{
	auto get_queue = [this](ICommandQueue::QueueType type) {
		std::shared_ptr<ICommandQueue>& from_queue = (type == ICommandQueue::QueueType::qt_gfx ? m_commandQueueGfx : m_commandQueueCompute);
		return from_queue;
	};
	get_queue(from)->Signal(m_fence_inter_queue, ++m_fence_inter_queue_val);
	get_queue(to)->WaitOnGPU(m_fence_inter_queue, m_fence_inter_queue_val);
}

ICommandList* DxBackend::InitCmdList()
{
	ICommandList* command_list_gfx = m_commandQueueGfx->ResetActiveCL();
	command_list_gfx->RSSetViewports(1, &m_viewport);
	command_list_gfx->RSSetScissorRects(1, &m_scissorRect);

	return command_list_gfx;
}

void DxBackend::ChechUpdatedShader()
{
	if (m_rebuild_shaders) {
		m_commandQueueGfx->Flush();
		m_techniques->RebuildShaders(L"Rebuild techniques");
		m_rebuild_shaders = false;
	}
}

void DxBackend::RenderUI()
{
	m_gui->Render(m_frameIndex);
}

void DxBackend::DebugSectionBegin(ICommandList* cmd_list, const std::string& name)
{
	CommandList* cl = (CommandList*)cmd_list;
	PIXBeginEvent(cl->GetRawCommandList().Get(), PIX_COLOR(55, 120, 55), name.c_str());
}

void DxBackend::DebugSectionEnd(ICommandList* cmd_list)
{
	CommandList* cl = (CommandList*)cmd_list;
	PIXEndEvent(cl->GetRawCommandList().Get());
}

const ITechniques::Technique* DxBackend::GetTechniqueById(uint32_t id) const
{
	return m_techniques->GetTechniqueById(id);
}

const IRootSignature* DxBackend::GetRootSignById(uint32_t id)
{
	return m_techniques->GetRootSignById(id);
}

void DxBackend::RebuildShaders(std::optional<std::wstring> dbg_name)
{
	m_rebuild_shaders = true;
}

bool DxBackend::PassImguiWndProc(const ImguiWindowData& data)
{
	if(m_gui)
		return ((ImguiHelper*)(m_gui.get()))->PassImguiWndProc(data);

	return false;
}

DxBackend::~DxBackend()
{
	m_gui->Destroy();
	m_commandQueueGfx->OnDestroy();
}

