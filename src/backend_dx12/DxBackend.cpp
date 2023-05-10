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

#if defined(USE_NSIGHT_AFTERMATH)
#include "NsightAftermathHelpers.h"
#endif

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

#if defined(_DEBUG) && !defined(USE_PIX_DEBUG) && !defined(USE_NSIGHT_AFTERMATH)
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
#if defined(USE_NSIGHT_AFTERMATH)
	m_gpuCrashTracker.reset(new GpuCrashTracker(m_markerMap, path.string()));
	// Enable Nsight Aftermath GPU crash dump creation.
	// This needs to be done before the D3D device is created.
	m_gpuCrashTracker->Initialize();
#endif
	m_device->OnInit(m_factory.Get());
#if defined(USE_NSIGHT_AFTERMATH)
	// Initialize Nsight Aftermath for this device.
	//
	// * EnableMarkers - this will include information about the Aftermath
	//   event marker nearest to the crash.
	//
	//   Using event markers should be considered carefully as they can
	//   cause considerable CPU overhead when used in high frequency code
	//   paths. Therefore, the event marker features is only available if
	//   the Nsight Aftermath GPU Crash Dump Monitor is running on the
	//   system. No Aftermath configuration needs to be made in the
	//   Monitor. It serves only as a dongle to ensure Aftermath event
	//   markers do not impact application performance on end user systems.
	//
	// * EnableResourceTracking - this will include additional information about the
	//   resource related to a GPU virtual address seen in case of a crash due to a GPU
	//   page fault. This includes, for example, information about the size of the
	//   resource, its format, and an indication if the resource has been deleted.
	//
	// * CallStackCapturing - this will include call stack and module information for
	//   the draw call, compute dispatch, or resource copy nearest to the crash.
	//
	//   Using this option should be considered carefully. Enabling call stack
	//   capturing will cause very high CPU overhead.
	//
	// * GenerateShaderDebugInfo - this instructs the shader compiler to
	//   generate debug information (line tables) for all shaders. Using this option
	//   should be considered carefully. It may cause considerable shader compilation
	//   overhead and additional overhead for handling the corresponding shader debug
	//   information callbacks.
	//
	const uint32_t aftermathFlags =
		GFSDK_Aftermath_FeatureFlags_EnableMarkers |             // Enable event marker tracking. Only effective in combination with the Nsight Aftermath Crash Dump Monitor.
		GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |    // Enable tracking of resources.
		GFSDK_Aftermath_FeatureFlags_CallStackCapturing |        // Capture call stacks for all draw calls, compute dispatches, and resource copies.
		GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo;    // Generate debug information for shaders.

	AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_Initialize(
		GFSDK_Aftermath_Version_API,
		aftermathFlags,
		m_device->GetNativeObject().Get()));
#endif

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
	// Present the frame.
#if defined(USE_NSIGHT_AFTERMATH)
	HRESULT hr = m_swap_chain->Present();
	if (FAILED(hr))
	{
		// DXGI_ERROR error notification is asynchronous to the NVIDIA display
		// driver's GPU crash handling. Give the Nsight Aftermath GPU crash dump
		// thread some time to do its work before terminating the process.
		auto tdrTerminationTimeout = std::chrono::seconds(3);
		auto tStart = std::chrono::steady_clock::now();
		auto tElapsed = std::chrono::milliseconds::zero();

		GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

		while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
			status != GFSDK_Aftermath_CrashDump_Status_Finished &&
			tElapsed < tdrTerminationTimeout)
		{
			// Sleep 50ms and poll the status again until timeout or Aftermath finished processing the crash dump.
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

			auto tEnd = std::chrono::steady_clock::now();
			tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
		}

		if (status != GFSDK_Aftermath_CrashDump_Status_Finished)
		{
			std::stringstream err_msg;
			err_msg << "Unexpected crash dump status: " << status;
			MessageBoxA(NULL, err_msg.str().c_str(), "Aftermath Error", MB_OK);
		}

		// Terminate on failure
		exit(-1);
	}
	m_frameCounter++;

	// clear the marker map for the current frame before writing any markers
	m_markerMap[m_frameCounter % GpuCrashTracker::c_markerFrameHistory].clear();
#else
	m_swap_chain->Present();
#endif

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

#if defined(USE_NSIGHT_AFTERMATH)
	// A helper that prepends the frame number to a string
	auto createMarkerStringForFrame = [this](const char* markerString) {
		std::stringstream ss;
		ss << "Frame " << m_frameCounter << ": " << markerString;
		return ss.str();
	};

	
	// Inject a marker in the command list before the draw call.
	// Second argument appManagedMarker=true means that Aftermath will not copy marker data and depend on the app to resolve the marker later
	const std::string markerData = createMarkerStringForFrame(name.c_str());
	bool appManagedMarker = true;

	// A helper for setting Aftermath event markers.
	// For maximum CPU performance, use GFSDK_Aftermath_SetEventMarker() with dataSize=0.
	// This instructs Aftermath not to allocate and copy off memory internally, relying on
	// the application to manage marker pointers itself.
	if (appManagedMarker)
	{
		// App is responsible for handling marker memory, and for resolving the memory at crash dump generation time.
		// The actual "const void* markerData" passed to Aftermath in this case can be any uniquely identifying value that the app can resolve to the marker data later.
		// For this sample, we will use this approach to generating a unique marker value:
		// We keep a ringbuffer with a marker history of the last c_markerFrameHistory frames (currently 4).
		UINT markerMapIndex = m_frameCounter % GpuCrashTracker::c_markerFrameHistory;
		auto& currentFrameMarkerMap = m_markerMap[markerMapIndex];
		// Take the index into the ringbuffer, multiply by 10000, and add the total number of markers logged so far in the current frame, +1 to avoid a value of zero.
		size_t markerID = markerMapIndex * 10000 + currentFrameMarkerMap.size() + 1;
		// This value is the unique identifier we will pass to Aftermath and internally associate with the marker data in the map.
		currentFrameMarkerMap[markerID] = markerData;
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(cl->GetNvAfterMathHndl(), (void*)markerID, 0));
		// For example, if we are on frame 625, markerMapIndex = 625 % 4 = 1...
		// The first marker for the frame will have markerID = 1 * 10000 + 0 + 1 = 10001.
		// The 15th marker for the frame will have markerID = 1 * 10000 + 14 + 1 = 10015.
		// On the next frame, 626, markerMapIndex = 626 % 4 = 2.
		// The first marker for this frame will have markerID = 2 * 10000 + 0 + 1 = 20001.
		// The 15th marker for the frame will have markerID = 2 * 10000 + 14 + 1 = 20015.
		// So with this scheme, we can safely have up to 10000 markers per frame, and can guarantee a unique markerID for each one.
		// There are many ways to generate and track markers and unique marker identifiers!
	}
	else
	{
		AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_SetEventMarker(cl->GetNvAfterMathHndl(), (void*)markerData.c_str(), (unsigned int)markerData.size() + 1));
	}
#endif
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

